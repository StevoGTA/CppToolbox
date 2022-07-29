//----------------------------------------------------------------------------------------------------------------------
//	CPCMAudioCodec.h			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CPCMAudioCodec.h"

#include "CCodecRegistry.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CPCMDecodeAudioCodec

class CPCMDecodeAudioCodec : public CDecodeAudioCodec {
	public:
										// Lifecycle methods
										CPCMDecodeAudioCodec(const I<CRandomAccessDataSource>& randomAccessDataSource,
												UInt64 startByteOffset, UInt64 byteCount, UInt8 frameByteCount,
												CPCMAudioCodec::Format format) :
											mFrameSourceDecodeInfo(randomAccessDataSource, startByteOffset, byteCount,
															frameByteCount),
													mFormat(format)
											{}

										// CAudioCodec methods - Decoding
		TArray<SAudioProcessingSetup>	getAudioProcessingSetups(const SAudioStorageFormat& audioStorageFormat);
		OI<SError>						setup(const SAudioProcessingFormat& audioProcessingFormat);
		void							seek(UniversalTimeInterval timeInterval);
		OI<SError>						decodeInto(CAudioFrames& audioFrames);

	private:
		FrameSourceDecodeInfo		mFrameSourceDecodeInfo;
		CPCMAudioCodec::Format		mFormat;

		OI<SAudioProcessingFormat>	mAudioProcessingFormat;
};

// MARK: CAudioCodec methods - Decoding

//----------------------------------------------------------------------------------------------------------------------
TArray<SAudioProcessingSetup> CPCMDecodeAudioCodec::getAudioProcessingSetups(
		const SAudioStorageFormat& audioStorageFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	return TNArray<SAudioProcessingSetup>(
				SAudioProcessingSetup(*audioStorageFormat.getBits(), audioStorageFormat.getSampleRate(),
						audioStorageFormat.getChannelMap(),
						(audioStorageFormat.getCodecID() == CPCMAudioCodec::mFloatID) ?
								SAudioProcessingSetup::kSampleTypeFloat :
								SAudioProcessingSetup::kSampleTypeSignedInteger,
						(mFormat == CPCMAudioCodec::kFormatBigEndian) ?
								SAudioProcessingSetup::kEndianBig : SAudioProcessingSetup::kEndianLittle));
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CPCMDecodeAudioCodec::setup(const SAudioProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mAudioProcessingFormat = OI<SAudioProcessingFormat>(audioProcessingFormat);

	return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
void CPCMDecodeAudioCodec::seek(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Seek
	mFrameSourceDecodeInfo.seek((UInt64) (timeInterval * mAudioProcessingFormat->getSampleRate()));
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CPCMDecodeAudioCodec::decodeInto(CAudioFrames& audioFrames)
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	TVResult<UInt32>	frameCount = mFrameSourceDecodeInfo.readInto(audioFrames);
	ReturnErrorIfResultError(frameCount);

	// Check if need to convert unsigned 8 bit samples to signed 8 bit samples
	if (mFormat == CPCMAudioCodec::kFormat8BitUnsigned)
		// Toggle
		audioFrames.toggle8BitSignedUnsigned();

	return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CPCMAudioCodec

// MARK: Properties

const	OSType	CPCMAudioCodec::mFloatID = MAKE_OSTYPE('f', 'P', 'C', 'M');
const	OSType	CPCMAudioCodec::mIntegerID = MAKE_OSTYPE('N', 'O', 'N', 'E');

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
OI<I<CDecodeAudioCodec> > CPCMAudioCodec::create(const SAudioStorageFormat& audioStorageFormat,
		const I<CRandomAccessDataSource>& randomAccessDataSource, UInt64 startByteOffset, UInt64 byteCount,
		Format format)
//----------------------------------------------------------------------------------------------------------------------
{
	return OI<I<CDecodeAudioCodec> >(
			I<CDecodeAudioCodec>(
					new CPCMDecodeAudioCodec(randomAccessDataSource, startByteOffset, byteCount,
							*audioStorageFormat.getBits() / 8 * audioStorageFormat.getChannels(), format)));
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Declare audio codecs

REGISTER_CODEC(pcmFloat, CAudioCodec::Info(CPCMAudioCodec::mFloatID, CString(OSSTR("None (Floating Point)"))));
REGISTER_CODEC(pcmInteger, CAudioCodec::Info(CPCMAudioCodec::mIntegerID, CString(OSSTR("None (Integer)"))));
