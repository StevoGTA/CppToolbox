//----------------------------------------------------------------------------------------------------------------------
//	CPCMAudioCodec.cpp			©2022 Stevo Brock	All rights reserved.
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
		TArray<SAudio::ProcessingSetup>	getAudioProcessingSetups(const SAudio::Format& audioFormat);
		OV<SError>						setup(const SAudio::ProcessingFormat& audioProcessingFormat);
		void							seek(UniversalTimeInterval timeInterval);
		OV<SError>						decodeInto(CAudioFrames& audioFrames);

	private:
		FrameSourceDecodeInfo			mFrameSourceDecodeInfo;
		CPCMAudioCodec::Format			mFormat;

		OV<SAudio::ProcessingFormat>	mAudioProcessingFormat;
};

// MARK: CAudioCodec methods - Decoding

//----------------------------------------------------------------------------------------------------------------------
TArray<SAudio::ProcessingSetup> CPCMDecodeAudioCodec::getAudioProcessingSetups(const SAudio::Format& audioFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	return TNArray<SAudio::ProcessingSetup>(
				SAudio::ProcessingSetup(*audioFormat.getBits(), audioFormat.getSampleRate(),
						audioFormat.getChannelMap(),
						(audioFormat.getCodecID() == CPCMAudioCodec::mFloatID) ?
								SAudio::ProcessingSetup::kSampleTypeFloat :
								SAudio::ProcessingSetup::kSampleTypeSignedInteger,
						(mFormat == CPCMAudioCodec::kFormatBigEndian) ?
								SAudio::ProcessingSetup::kEndianBig : SAudio::ProcessingSetup::kEndianLittle));
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CPCMDecodeAudioCodec::setup(const SAudio::ProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mAudioProcessingFormat.setValue(audioProcessingFormat);

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
void CPCMDecodeAudioCodec::seek(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Seek
	mFrameSourceDecodeInfo.seek((UInt64) (timeInterval * mAudioProcessingFormat->getSampleRate()));
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CPCMDecodeAudioCodec::decodeInto(CAudioFrames& audioFrames)
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	TVResult<UInt32>	frameCount = mFrameSourceDecodeInfo.readInto(audioFrames);
	ReturnErrorIfResultError(frameCount);

	// Check if need to convert unsigned 8 bit samples to signed 8 bit samples
	if (mFormat == CPCMAudioCodec::kFormat8BitUnsigned)
		// Toggle
		audioFrames.toggle8BitSignedUnsigned();

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CPCMAudioCodec

// MARK: Properties

const	OSType	CPCMAudioCodec::mFloatID = MAKE_OSTYPE('f', 'P', 'C', 'M');
const	OSType	CPCMAudioCodec::mIntegerID = MAKE_OSTYPE('N', 'O', 'N', 'E');

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
SAudio::Format CPCMAudioCodec::composeAudioFormat(bool isFloat, UInt8 bits, Float32 sampleRate,
		const SAudio::ChannelMap& audioChannelMap)
//----------------------------------------------------------------------------------------------------------------------
{
	return SAudio::Format(isFloat ? mFloatID : mIntegerID, bits, sampleRate, audioChannelMap);
}

//----------------------------------------------------------------------------------------------------------------------
SMedia::SegmentInfo CPCMAudioCodec::composeMediaSegmentInfo(const SAudio::Format& audioFormat, UInt64 byteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	return SAudio::composeMediaSegmentInfo(audioFormat, 1,
			*audioFormat.getBits() / 8 * audioFormat.getChannelMap().getChannelCount(), byteCount);
}

//----------------------------------------------------------------------------------------------------------------------
I<CDecodeAudioCodec> CPCMAudioCodec::create(const SAudio::Format& audioFormat,
		const I<CRandomAccessDataSource>& randomAccessDataSource, UInt64 startByteOffset, UInt64 byteCount,
		Format format)
//----------------------------------------------------------------------------------------------------------------------
{
	return I<CDecodeAudioCodec>(
			new CPCMDecodeAudioCodec(randomAccessDataSource, startByteOffset, byteCount,
					*audioFormat.getBits() / 8 * audioFormat.getChannelMap().getChannelCount(), format));
}
