//----------------------------------------------------------------------------------------------------------------------
//	CPCMAudioCodec.h			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CPCMAudioCodec.h"

#include "CCodecRegistry.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CPCMAudioCodecInternals
class CPCMAudioCodecInternals {
	public:
		CPCMAudioCodecInternals() {}

		OI<SAudioProcessingFormat>	mAudioProcessingFormat;
		OI<I<CCodec::DecodeInfo> >	mDecodeInfo;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CPCMAudioCodec

// MARK: Properties

OSType	CPCMAudioCodec::mFloatID = MAKE_OSTYPE('f', 'P', 'C', 'M');
OSType	CPCMAudioCodec::mIntegerID = MAKE_OSTYPE('N', 'O', 'N', 'E');

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CPCMAudioCodec::CPCMAudioCodec() : CDecodeOnlyAudioCodec()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CPCMAudioCodecInternals();
}

//----------------------------------------------------------------------------------------------------------------------
CPCMAudioCodec::~CPCMAudioCodec()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CAudioCodec methods - Decoding

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CPCMAudioCodec::setupForDecode(const SAudioProcessingFormat& audioProcessingFormat,
		const I<CCodec::DecodeInfo>& decodeInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mAudioProcessingFormat = OI<SAudioProcessingFormat>(audioProcessingFormat);
	mInternals->mDecodeInfo = OI<I<CCodec::DecodeInfo> >(decodeInfo);

	return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
CAudioFrames::Requirements CPCMAudioCodec::getRequirements() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Return requirements
	return CAudioFrames::Requirements(1, 1);
}

//----------------------------------------------------------------------------------------------------------------------
void CPCMAudioCodec::seek(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	DecodeInfo&	decodeInfo = (DecodeInfo&) (**mInternals->mDecodeInfo);

	// Seek
	decodeInfo.seek(timeInterval * mInternals->mAudioProcessingFormat->getSampleRate());
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CPCMAudioCodec::decode(CAudioFrames& audioFrames)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	DecodeInfo&	decodeInfo = (DecodeInfo&) (**mInternals->mDecodeInfo);

	// Read
	CAudioFrames::Info	writeInfo = audioFrames.getWriteInfo();
	UInt8*				buffer = (UInt8*) writeInfo.getSegments()[0];
	TVResult<UInt32>	frameCount = decodeInfo.read(buffer, writeInfo.getFrameCount());
	ReturnErrorIfResultError(frameCount);

	// Check if need to convert unsigned 8 bit samples to signed 8 bit samples
	if ((mInternals->mAudioProcessingFormat->getBits() == 8) && decodeInfo.get8BitIsUnsigned()) {
		// Convert
		UInt32	byteCount = frameCount.getValue() * decodeInfo.getFrameByteCount();

		// Do 8 byte chunks first
		while (byteCount >= 8) {
			// Do these 8 bytes
			*((UInt64*) buffer) ^= 0x8080808080808080LL;
			buffer += 8;
			byteCount -= 8;
		}

		// Finish the last 1-7 bytes
		while (byteCount-- > 0)
			// Do this byte
			*buffer++ ^= 0x80;
	}

	// Complete write
	audioFrames.completeWrite(frameCount.getValue());

	return OI<SError>();
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
OI<SAudioStorageFormat> CPCMAudioCodec::composeAudioStorageFormat(bool isFloat, UInt8 bits, Float32 sampleRate,
		EAudioChannelMap channelMap)
//----------------------------------------------------------------------------------------------------------------------
{
	// Return audio storage format
	return OI<SAudioStorageFormat>(new SAudioStorageFormat(isFloat ? mFloatID : mIntegerID, bits, sampleRate,
			channelMap));
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CPCMAudioCodec::composeFrameCount(const SAudioStorageFormat& audioStorageFormat, UInt64 byteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Return frame count
	return byteCount / (*audioStorageFormat.getBits() / 8 * audioStorageFormat.getChannels());
}

//----------------------------------------------------------------------------------------------------------------------
I<CCodec::DecodeInfo> CPCMAudioCodec::composeDecodeInfo(const SAudioStorageFormat& audioStorageFormat,
		const I<CSeekableDataSource>& seekableDataSource, UInt64 startByteOffset, UInt64 byteCount,
		bool _8BitIsUnsigned)
//----------------------------------------------------------------------------------------------------------------------
{
	// Return decode info
	return I<CCodec::DecodeInfo>(new DecodeInfo(seekableDataSource, startByteOffset, byteCount,
			*audioStorageFormat.getBits() / 8 * audioStorageFormat.getChannels(), _8BitIsUnsigned));
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local procs

static	TArray<SAudioProcessingSetup>	sGetAudioProcessingSetups(OSType id,
												const SAudioStorageFormat& audioStorageFormat)
											{
												return TNArray<SAudioProcessingSetup>(
															SAudioProcessingSetup(*audioStorageFormat.getBits(),
																	audioStorageFormat.getSampleRate(),
																	audioStorageFormat.getChannelMap(),
																	(audioStorageFormat.getCodecID() ==
																					CPCMAudioCodec::mFloatID) ?
																			SAudioProcessingSetup::kSampleTypeFloat :
																			SAudioProcessingSetup::
																					kSampleTypeSignedInteger));
											}
static	I<CAudioCodec>					sInstantiate(OSType id)
											{ return I<CAudioCodec>(new CPCMAudioCodec()); }

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Declare audio codecs

REGISTER_CODEC(PCMFloat,
		CAudioCodec::Info(CPCMAudioCodec::mFloatID, CString("None (Floating Point)"),
				sGetAudioProcessingSetups, sInstantiate));
REGISTER_CODEC(PCMInteger,
		CAudioCodec::Info(CPCMAudioCodec::mIntegerID, CString("None (Integer)"),
				sGetAudioProcessingSetups, sInstantiate));
