//----------------------------------------------------------------------------------------------------------------------
//	CMediaFoundationAudioCodecs.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAACAudioCodec.h"

#include "CLogServices-Windows.h"
#include "CMediaFoundationServices.h"
#include "SError-Windows.h"

#undef Delete

#include <mfapi.h>
#include <mftransform.h>
#include <Mferror.h>

#define Delete(x)	{ delete x; x = nil; }

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CString	sErrorDomain(OSSTR("CMediaFoundationAudioCodecs"));
static	SError	sSetupDidNotCompleteError(sErrorDomain, 1, CString(OSSTR("Setup did not complete")));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAACAudioCodecInternals

class CAACAudioCodecInternals {
	public:
							CAACAudioCodecInternals(OSType codecID) :
								mCodecID(codecID), mDecodeFramesToIgnore(0), mNextPacketIndex(0)
								{}

		static	OI<SError>	fillInputBuffer(IMFSample* sample, IMFMediaBuffer* mediaBuffer, void* userData)
								{
									// Setup
									CAACAudioCodecInternals&	internals = *((CAACAudioCodecInternals*) userData);

									return CMediaFoundationServices::load(mediaBuffer,
											*(*internals.mDecodeInfo)->getMediaPacketSource());
								}


		OSType						mCodecID;
		OI<SAudioProcessingFormat>	mAudioProcessingFormat;
		OI<I<CCodec::DecodeInfo> >	mDecodeInfo;

		OCI<IMFTransform>			mAudioDecoder;
		UInt32						mDecodeFramesToIgnore;
		OCI<IMFSample>				mInputSample;
		OCI<IMFSample>				mOutputSample;
		UInt32						mNextPacketIndex;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAACAudioCodec

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAACAudioCodec::CAACAudioCodec(OSType codecID)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CAACAudioCodecInternals(codecID);
}

//----------------------------------------------------------------------------------------------------------------------
CAACAudioCodec::~CAACAudioCodec()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CAudioCodec methods - Decoding

//----------------------------------------------------------------------------------------------------------------------
void CAACAudioCodec::setupForDecode(const SAudioProcessingFormat& audioProcessingFormat,
		const I<CCodec::DecodeInfo>& decodeInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	DecodeInfo&	aacDecodeInfo = *((DecodeInfo*) &*decodeInfo);

	// Store
	mInternals->mAudioProcessingFormat = OI<SAudioProcessingFormat>(audioProcessingFormat);
	mInternals->mDecodeInfo = OI<I<CCodec::DecodeInfo> >(decodeInfo);

	// Create audio decoder
#pragma pack(push,1)
	struct UserData {
		WORD	mPayloadType;
		WORD	mAudioProfileLevelIndication;
		WORD	mStructType;
		WORD	mReserved1;
		DWORD	mReserved2;
		WORD	mAudioSpecificConfig;
	} userData = {0};
#pragma pack(pop)
	userData.mAudioSpecificConfig = EndianU16_NtoB(aacDecodeInfo.getStartCodes());

	TCIResult<IMFTransform>	audioDecoder =
									CMediaFoundationServices::createTransformForAudioDecode(MFAudioFormat_AAC,
											audioProcessingFormat,
											OI<CData>(new CData(&userData, sizeof(UserData), false)));
	if (audioDecoder.hasError())
		return;
	mInternals->mAudioDecoder = audioDecoder.getInstance();

	// Create input sample
	TCIResult<IMFSample>	sample = CMediaFoundationServices::createSample(10 * 1024);
	if (sample.hasError())
		return;
	mInternals->mInputSample = sample.getInstance();

	// Create output sample
	MFT_OUTPUT_STREAM_INFO	outputStreamInfo;
	HRESULT					result = mInternals->mAudioDecoder->GetOutputStreamInfo(0, &outputStreamInfo);
	if (FAILED(result)) {
		// Failed
		LogHRESULT(result, "GetOutputStreamInfo");

		return;
	}

	sample = CMediaFoundationServices::createSample(outputStreamInfo.cbSize);
	if (sample.hasError())
		return;
	mInternals->mOutputSample = sample.getInstance();

	// Begin streaming!
	result = mInternals->mAudioDecoder->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0);
	LogHRESULTIfFailed(result, "ProcessMessage to begin streaming");
}

//----------------------------------------------------------------------------------------------------------------------
void CAACAudioCodec::seek(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Flush
	mInternals->mAudioDecoder->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, 0);

	// Seek
	mInternals->mDecodeFramesToIgnore =
			(*mInternals->mDecodeInfo)->getMediaPacketSource()->seekToDuration(
					(UInt32) (timeInterval * mInternals->mAudioProcessingFormat->getSampleRate()));
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CAACAudioCodec::decode(CAudioFrames& audioFrames)
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	AssertFailIf(audioFrames.getAvailableFrameCount() < (1024 * 2));

	if (!mInternals->mAudioDecoder.hasInstance() || !mInternals->mInputSample.hasInstance() ||
			!mInternals->mOutputSample.hasInstance())
		// Can't decode
		return OI<SError>(sSetupDidNotCompleteError);

	// Fill audio frames as much as we can
	while (audioFrames.getAvailableFrameCount() >= 1024) {
		// Process output
		OI<SError>	error =
							CMediaFoundationServices::processOutput(*mInternals->mAudioDecoder,
									*mInternals->mOutputSample,
									CMediaFoundationServices::ProcessOutputInfo(
											CAACAudioCodecInternals::fillInputBuffer, mInternals->mInputSample,
											mInternals));
		ReturnErrorIfError(error);

		// Complete write
		error =
				CMediaFoundationServices::completeWrite(*mInternals->mOutputSample, mInternals->mDecodeFramesToIgnore,
						audioFrames, *mInternals->mAudioProcessingFormat);
		ReturnErrorIfError(error);

		// Update
		mInternals->mDecodeFramesToIgnore = 0;
	}

	return OI<SError>();
}
