//----------------------------------------------------------------------------------------------------------------------
//	CMediaFoundationAudioCodecs.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAACAudioCodec.h"

#include "CLogServices-Windows.h"
#include "CMediaFoundationServices.h"
#include "SError-Windows.h"

#undef Delete

#include <mfapi.h>

#define Delete(x)	{ delete x; x = nil; }

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CString	sErrorDomain(OSSTR("CMediaFoundationAudioCodecs"));
static	SError	sSetupDidNotCompleteError(sErrorDomain, 1, CString(OSSTR("Setup did not complete")));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAACDecodeAudioCodec

class CAACDecodeAudioCodec : public CDecodeAudioCodec {
	public:
												// Lifecycle methods
												CAACDecodeAudioCodec(OSType codecID,
														const I<CMediaPacketSource>& mediaPacketSource,
														const CData& configurationData) :
													mCodecID(codecID),
														mDecodeInfo(mediaPacketSource, configurationData),
														mDecodeFramesToIgnore(0)
													{}

												// CAudioCodec methods - Decoding
				TArray<SAudioProcessingSetup>	getAudioProcessingSetups(const SAudioStorageFormat& audioStorageFormat);
				OI<SError>						setup(const SAudioProcessingFormat& audioProcessingFormat);
				CAudioFrames::Requirements		getRequirements() const
													{ return CAudioFrames::Requirements(1024, 1024 * 2); }
				void							seek(UniversalTimeInterval timeInterval);
				OI<SError>						decodeInto(CAudioFrames& audioFrames);

												// Class methods
		static	OI<SError>						fillInputBuffer(IMFSample* sample, IMFMediaBuffer* mediaBuffer,
														void* userData);

	private:
		OSType						mCodecID;
		CAACAudioCodec::DecodeInfo	mDecodeInfo;

		OI<SAudioProcessingFormat>	mAudioProcessingFormat;

		OCI<IMFTransform>			mAudioDecoderTransform;
		UInt32						mDecodeFramesToIgnore;
		OCI<IMFSample>				mInputSample;
		OCI<IMFSample>				mOutputSample;
};

// MARK: CAudioCodec methods - Decoding

//----------------------------------------------------------------------------------------------------------------------
TArray<SAudioProcessingSetup> CAACDecodeAudioCodec::getAudioProcessingSetups(
		const SAudioStorageFormat& audioStorageFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	return TNArray<SAudioProcessingSetup>(
			SAudioProcessingSetup(32, audioStorageFormat.getSampleRate(), audioStorageFormat.getChannelMap(),
					SAudioProcessingSetup::kSampleTypeFloat));
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CAACDecodeAudioCodec::setup(const SAudioProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mAudioProcessingFormat = OI<SAudioProcessingFormat>(audioProcessingFormat);

	// Create audio decoder
#pragma pack(push, 1)

	struct UserData {
		WORD	mPayloadType;
		WORD	mAudioProfileLevelIndication;
		WORD	mStructType;
		WORD	mReserved1;
		DWORD	mReserved2;
		WORD	mAudioSpecificConfig;
	} userData = {0};

#pragma pack(pop)

	userData.mAudioSpecificConfig = EndianU16_NtoB(mDecodeInfo.getStartCodes());

	TCIResult<IMFTransform>	audioDecoderTransform =
									CMediaFoundationServices::createTransformForAudioDecoder(MFAudioFormat_AAC,
											audioProcessingFormat,
											OI<CData>(new CData(&userData, sizeof(UserData), false)));
	if (audioDecoderTransform.hasError())
		return OI<SError>(audioDecoderTransform.getError());
	mAudioDecoderTransform = audioDecoderTransform.getInstance();

	// Create input sample
	TCIResult<IMFSample>	sample = CMediaFoundationServices::createSample(10 * 1024);
	ReturnErrorIfResultError(sample);
	mInputSample = sample.getInstance();

	// Create output sample
	MFT_OUTPUT_STREAM_INFO	outputStreamInfo;
	HRESULT					result = mAudioDecoderTransform->GetOutputStreamInfo(0, &outputStreamInfo);
	ReturnErrorIfFailed(result, "GetOutputStreamInfo");

	sample = CMediaFoundationServices::createSample(outputStreamInfo.cbSize);
	ReturnErrorIfResultError(sample);
	mOutputSample = sample.getInstance();

	// Begin streaming!
	result = mAudioDecoderTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0);
	ReturnErrorIfFailed(result, "ProcessMessage to begin streaming");

	return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
void CAACDecodeAudioCodec::seek(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	if (!mAudioDecoderTransform.hasInstance())
		// Can't seek
		return;

	// Flush
	mAudioDecoderTransform->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, 0);

	// Seek
	mDecodeFramesToIgnore =
			mDecodeInfo.getMediaPacketSource()->seekToDuration(
					(UInt32) (timeInterval * mAudioProcessingFormat->getSampleRate()));
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CAACDecodeAudioCodec::decodeInto(CAudioFrames& audioFrames)
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	AssertFailIf(audioFrames.getAvailableFrameCount() < (1024 * 2));

	if (!mAudioDecoderTransform.hasInstance() || !mInputSample.hasInstance() || !mOutputSample.hasInstance())
		// Can't decode
		return OI<SError>(sSetupDidNotCompleteError);

	// Fill audio frames as much as we can
	while (audioFrames.getAvailableFrameCount() >= 1024) {
		// Process output
		OI<SError>	error =
							CMediaFoundationServices::processOutput(*mAudioDecoderTransform, *mOutputSample,
									CMediaFoundationServices::ProcessOutputInfo(fillInputBuffer, mInputSample, this));
		ReturnErrorIfError(error);

		// Complete write
		error =
				CMediaFoundationServices::completeWrite(*mOutputSample, mDecodeFramesToIgnore, audioFrames,
						*mAudioProcessingFormat);
		ReturnErrorIfError(error);

		// Update
		mDecodeFramesToIgnore = 0;
	}

	return OI<SError>();
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CAACDecodeAudioCodec::fillInputBuffer(IMFSample* sample, IMFMediaBuffer* mediaBuffer, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CAACDecodeAudioCodec&	audioCodec = *((CAACDecodeAudioCodec*) userData);

	return CMediaFoundationServices::load(mediaBuffer, *audioCodec.mDecodeInfo.getMediaPacketSource());
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAACAudioCodec

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
I<CDecodeAudioCodec> CAACAudioCodec::create(const SAudioStorageFormat& audioStorageFormat,
		const I<CSeekableDataSource>& seekableDataSource, const TArray<SMediaPacketAndLocation>& packetAndLocations,
		const CData& configurationData)
//----------------------------------------------------------------------------------------------------------------------
{
	return I<CDecodeAudioCodec>(
			new CAACDecodeAudioCodec(audioStorageFormat.getCodecID(),
					I<CMediaPacketSource>(
							new CSeekableVaryingMediaPacketSource(seekableDataSource, packetAndLocations)),
					configurationData));
}
