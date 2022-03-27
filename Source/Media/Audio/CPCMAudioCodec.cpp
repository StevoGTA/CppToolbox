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
TArray<SAudioProcessingSetup> CPCMAudioCodec::getDecodeAudioProcessingSetups(
		const SAudioStorageFormat& audioStorageFormat, const I<CCodec::DecodeInfo>& decodeInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	DecodeInfo&	pcmAudioCodecDecodeInfo = (DecodeInfo&) *decodeInfo;

	return TNArray<SAudioProcessingSetup>(
				SAudioProcessingSetup(*audioStorageFormat.getBits(), audioStorageFormat.getSampleRate(),
						audioStorageFormat.getChannelMap(),
						(audioStorageFormat.getCodecID() == CPCMAudioCodec::mFloatID) ?
								SAudioProcessingSetup::kSampleTypeFloat :
								SAudioProcessingSetup::kSampleTypeSignedInteger,
						(pcmAudioCodecDecodeInfo.getFormat() == DecodeInfo::kFormatBigEndian) ?
								SAudioProcessingSetup::kEndianBig : SAudioProcessingSetup::kEndianLittle));
}

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
void CPCMAudioCodec::seek(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	DecodeInfo&	decodeInfo = (DecodeInfo&) (**mInternals->mDecodeInfo);

	// Seek
	decodeInfo.seek((UInt64) (timeInterval * mInternals->mAudioProcessingFormat->getSampleRate()));
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CPCMAudioCodec::decodeInto(CAudioFrames& audioFrames)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	DecodeInfo&	decodeInfo = (DecodeInfo&) (**mInternals->mDecodeInfo);

	// Read
	TVResult<UInt32>	frameCount = decodeInfo.readInto(audioFrames);
	ReturnErrorIfResultError(frameCount);

	// Check if need to convert unsigned 8 bit samples to signed 8 bit samples
	if (decodeInfo.getFormat() == DecodeInfo::kFormat8BitUnsigned)
		// Toggle
		audioFrames.toggle8BitSignedUnsigned();

	return OI<SError>();
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
OI<SAudioStorageFormat> CPCMAudioCodec::composeAudioStorageFormat(bool isFloat, UInt8 bits, Float32 sampleRate,
		EAudioChannelMap channelMap)
//----------------------------------------------------------------------------------------------------------------------
{
	// Return audio storage format
	return OI<SAudioStorageFormat>(
			new SAudioStorageFormat(isFloat ? mFloatID : mIntegerID, bits, sampleRate, channelMap));
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
		DecodeInfo::Format format)
//----------------------------------------------------------------------------------------------------------------------
{
	// Return decode info
	return I<CCodec::DecodeInfo>(
			new DecodeInfo(seekableDataSource, startByteOffset, byteCount,
					*audioStorageFormat.getBits() / 8 * audioStorageFormat.getChannels(), format));
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local procs

static	I<CAudioCodec>	sInstantiate(OSType id)
								{ return I<CAudioCodec>(new CPCMAudioCodec()); }

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Declare audio codecs

REGISTER_CODEC(pcmFloat,
		CAudioCodec::Info(CPCMAudioCodec::mFloatID, CString(OSSTR("None (Floating Point)")), sInstantiate));
REGISTER_CODEC(pcmInteger,
		CAudioCodec::Info(CPCMAudioCodec::mIntegerID, CString(OSSTR("None (Integer)")), sInstantiate));
