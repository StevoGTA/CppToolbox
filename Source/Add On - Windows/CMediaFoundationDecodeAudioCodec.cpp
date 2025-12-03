//----------------------------------------------------------------------------------------------------------------------
//	CMediaFoundationDecodeAudioCodec.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CMediaFoundationDecodeAudioCodec.h"

#include "CLogServices-Windows.h"
#include "CMediaFoundationServices.h"
#include "SError-Windows.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CString	sErrorDomain(OSSTR("CMediaFoundationDecodeAudioCodec"));
static	SError	sSetupDidNotCompleteError(sErrorDomain, 1, CString(OSSTR("Setup did not complete")));
static	SError	sNoMatchingOutputMediaTypes(sErrorDomain, 2, CString(OSSTR("No matching output media types")));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMediaFoundationDecodeAudioCodec::Internals

class CMediaFoundationDecodeAudioCodec::Internals {
	public:
							Internals(OSType codecID, const I<CMediaPacketSource>& mediaPacketSource) :
								mCodecID(codecID), mMediaPacketSource(mediaPacketSource),
										mDecodeFramesToIgnore(0)
								{}

		static	OV<SError>	fillInputBuffer(IMFSample* sample, IMFMediaBuffer* mediaBuffer, void* userData)
								{
									// Setup
									Internals&	internals = *((Internals*) userData);

									return CMediaFoundationServices::load(mediaBuffer, *internals.mMediaPacketSource);
								}

		OSType							mCodecID;
		I<CMediaPacketSource>			mMediaPacketSource;

		OV<SAudio::ProcessingFormat>	mAudioProcessingFormat;

		OCI<IMFTransform>				mAudioDecoderTransform;
		UInt32							mDecodeFramesToIgnore;
		OCI<IMFSample>					mInputSample;
		OCI<IMFSample>					mOutputSample;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMediaFoundationDecodeAudioCodec

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CMediaFoundationDecodeAudioCodec::CMediaFoundationDecodeAudioCodec(OSType codecID,
		const I<CMediaPacketSource>& mediaPacketSource)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(codecID, mediaPacketSource);
}

//----------------------------------------------------------------------------------------------------------------------
CMediaFoundationDecodeAudioCodec::~CMediaFoundationDecodeAudioCodec()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CDecodeAudioCodec methods

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CMediaFoundationDecodeAudioCodec::setup(const SAudio::ProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get and check GUID
	OR<const GUID>	guid = getGUID(mInternals->mCodecID);
	if (!guid.hasReference())
		return OV<SError>(CCodec::unsupportedError(CString(mInternals->mCodecID, true)));

	// Setup
	CAudioFrames::Requirements	requirements = getRequirements();

	// Store
	mInternals->mAudioProcessingFormat.setValue(audioProcessingFormat);

	// Enum Audio Codecs to find Audio Decoder
	MFT_REGISTER_TYPE_INFO	info = {MFMediaType_Audio, *guid};
	IMFActivate**			activates;
	UINT32					count;
	HRESULT					result =
									::MFTEnumEx(MFT_CATEGORY_AUDIO_DECODER,
											MFT_ENUM_FLAG_SYNCMFT | MFT_ENUM_FLAG_LOCALMFT |
													MFT_ENUM_FLAG_SORTANDFILTER,
											&info, NULL, &activates, &count);
	ReturnErrorIfFailed(result, CString(OSSTR("MFTEnumEx")));

	// Create the Audio Decoder
	IMFTransform*	transform;
	result = activates[0]->ActivateObject(IID_PPV_ARGS(&transform));
	mInternals->mAudioDecoderTransform = OCI<IMFTransform>(transform);

	for (UINT32 i = 0; i < count; i++)
		// Release
		activates[i]->Release();
	::CoTaskMemFree(activates);

	ReturnErrorIfFailed(result, CString(OSSTR("ActivateObject")));

	// Setup input media type
	OV<SError>	error =
						CMediaFoundationServices::setInputType(transform, *guid, 32,
								audioProcessingFormat.getSampleRate(), audioProcessingFormat.getChannelMap(),
								OV<UInt32>(), OV<UInt32>(), getUserData());
	ReturnErrorIfError(error);

	// Setup output media type
	error = CMediaFoundationServices::setOutputType(transform, audioProcessingFormat);
	ReturnErrorIfError(error);

	// Create input sample
	TCIResult<IMFSample>	sample = CMediaFoundationServices::createSample(10 * requirements.mFrameCountInterval);
	ReturnErrorIfResultError(sample);
	mInternals->mInputSample = sample.getInstance();

	// Create output sample
	MFT_OUTPUT_STREAM_INFO	outputStreamInfo;
	result = mInternals->mAudioDecoderTransform->GetOutputStreamInfo(0, &outputStreamInfo);
	ReturnErrorIfFailed(result, CString(OSSTR("GetOutputStreamInfo")));

	sample = CMediaFoundationServices::createSample(outputStreamInfo.cbSize);
	ReturnErrorIfResultError(sample);
	mInternals->mOutputSample = sample.getInstance();

	// Begin streaming!
	result = mInternals->mAudioDecoderTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0);
	ReturnErrorIfFailed(result, CString(OSSTR("ProcessMessage to begin streaming")));

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaFoundationDecodeAudioCodec::seek(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	if (!mInternals->mAudioDecoderTransform.hasInstance())
		// Can't seek
		return;

	// Flush
	mInternals->mAudioDecoderTransform->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, 0);

	// Seek
	mInternals->mDecodeFramesToIgnore =
			mInternals->mMediaPacketSource->seekToDuration(
					(UInt32) (timeInterval * mInternals->mAudioProcessingFormat->getSampleRate()));
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CMediaFoundationDecodeAudioCodec::decodeInto(CAudioFrames& audioFrames)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CAudioFrames::Requirements	requirements = getRequirements();

	// Preflight
	AssertFailIf(audioFrames.getAllocatedFrameCount() < requirements.mFrameCountMinimum);

	if (!mInternals->mAudioDecoderTransform.hasInstance() || !mInternals->mInputSample.hasInstance() ||
			!mInternals->mOutputSample.hasInstance())
		// Can't decode
		return OV<SError>(sSetupDidNotCompleteError);

	// Fill audio frames as much as we can
	while (audioFrames.getAllocatedFrameCount() >= requirements.mFrameCountInterval) {
		// Process output
		OV<SError>	error =
							CMediaFoundationServices::processOutput(*mInternals->mAudioDecoderTransform,
									*mInternals->mOutputSample,
									CMediaFoundationServices::ProcessOutputInfo(Internals::fillInputBuffer,
											mInternals->mInputSample, mInternals));
		ReturnErrorIfError(error);

		// Complete write
		TVResult<UInt32>	result =
									CMediaFoundationServices::completeWrite(*mInternals->mOutputSample,
											mInternals->mDecodeFramesToIgnore, audioFrames,
											*mInternals->mAudioProcessingFormat);
		ReturnErrorIfResultError(result);

		// Update
		mInternals->mDecodeFramesToIgnore =
				(*result < mInternals->mDecodeFramesToIgnore) ? mInternals->mDecodeFramesToIgnore - *result : 0;
	}

	return OV<SError>();
}
