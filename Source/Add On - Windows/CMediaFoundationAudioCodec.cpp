//----------------------------------------------------------------------------------------------------------------------
//	CMediaFoundationAudioCodec.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CMediaFoundationAudioCodec.h"

#include "CLogServices-Windows.h"
#include "CMediaFoundationServices.h"
#include "SError-Windows.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CString	sErrorDomain(OSSTR("CMediaFoundationAudioCodecs"));
static	SError	sSetupDidNotCompleteError(sErrorDomain, 1, CString(OSSTR("Setup did not complete")));
static	SError	sNoMatchingOutputMediaTypes(sErrorDomain, 2, CString(OSSTR("No matching output media types")));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMediaFoundationDecodeAudioCodecInternals

class CMediaFoundationDecodeAudioCodecInternals {
	public:
							CMediaFoundationDecodeAudioCodecInternals(OSType codecID,
									const I<CMediaPacketSource>& mediaPacketSource) :
								mCodecID(codecID), mMediaPacketSource(mediaPacketSource),
										mDecodeFramesToIgnore(0)
								{}

		static	OI<SError>	fillInputBuffer(IMFSample* sample, IMFMediaBuffer* mediaBuffer, void* userData)
								{
									// Setup
									CMediaFoundationDecodeAudioCodecInternals&	internals =
																					*((CMediaFoundationDecodeAudioCodecInternals*)
																							userData);

									return CMediaFoundationServices::load(mediaBuffer, *internals.mMediaPacketSource);
								}

		OSType						mCodecID;
		I<CMediaPacketSource>		mMediaPacketSource;

		OI<SAudioProcessingFormat>	mAudioProcessingFormat;

		OCI<IMFTransform>			mAudioDecoderTransform;
		UInt32						mDecodeFramesToIgnore;
		OCI<IMFSample>				mInputSample;
		OCI<IMFSample>				mOutputSample;
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
	mInternals = new CMediaFoundationDecodeAudioCodecInternals(codecID, mediaPacketSource);
}

//----------------------------------------------------------------------------------------------------------------------
CMediaFoundationDecodeAudioCodec::~CMediaFoundationDecodeAudioCodec()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CDecodeAudioCodec methods

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CMediaFoundationDecodeAudioCodec::setup(const SAudioProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
			CAudioFrames::Requirements	requirements = getRequirements();
	const	GUID&						guid = getGUID();
			HRESULT						result;

	// Store
	mInternals->mAudioProcessingFormat = OI<SAudioProcessingFormat>(audioProcessingFormat);

	// Enum Audio Codecs to find Audio Decoder
	MFT_REGISTER_TYPE_INFO	info = {MFMediaType_Audio, guid};
	IMFActivate**			activates;
	UINT32					count;
	result =
			::MFTEnumEx(MFT_CATEGORY_AUDIO_DECODER,
					MFT_ENUM_FLAG_SYNCMFT | MFT_ENUM_FLAG_LOCALMFT | MFT_ENUM_FLAG_SORTANDFILTER, &info, NULL,
					&activates, &count);
	ReturnErrorIfFailed(result, OSSTR("MFTEnumEx"));

	// Create the Audio Decoder
	IMFTransform*	transform;
	result = activates[0]->ActivateObject(IID_PPV_ARGS(&transform));
	OCI<IMFTransform>	audioDecoder(transform);

	for (UINT32 i = 0; i < count; i++)
		// Release
		activates[i]->Release();
	::CoTaskMemFree(activates);

	ReturnErrorIfFailed(result, OSSTR("ActivateObject"));

	// Setup input media type
	TCIResult<IMFMediaType>	inputMediaType =
									CMediaFoundationServices::createMediaType(guid, 32,
											audioProcessingFormat.getSampleRate(),
											audioProcessingFormat.getChannelMap(), OV<UInt32>(), OV<UInt32>(),
											getUserData());
	ReturnErrorIfResultError(inputMediaType);

	result = transform->SetInputType(0, *(inputMediaType.getInstance()), 0);
	ReturnErrorIfFailed(result, OSSTR("SetInputType"));

	// Iterate output media types to find matching
	DWORD	index = 0;
	GUID	targetCodecSubType = audioProcessingFormat.getIsFloat() ? MFAudioFormat_Float : MFAudioFormat_PCM;
	while (true) {
		// Get next media type
		IMFMediaType*	mediaType;
		result = transform->GetOutputAvailableType(0, index, &mediaType);
		if (result != S_OK)
			// No matching output media types
			return OI<SError>(sNoMatchingOutputMediaTypes);

		// Get info
		GUID	codecSubType;
		result = mediaType->GetGUID(MF_MT_SUBTYPE, &codecSubType);
		if (result != S_OK)
			continue;

		UINT32	bits = 0;
		result = mediaType->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &bits);
		if (result != S_OK)
			continue;

		UINT32	channels = 0;
		result = mediaType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &channels);
		if (result != S_OK)
			continue;

		// Compare codec subtype, bits, and channels
		if ((codecSubType == targetCodecSubType) &&
				(bits == (UINT32) audioProcessingFormat.getBits()) &&
				(channels == (UINT32)audioProcessingFormat.getChannels())) {
			// Found match
			result = transform->SetOutputType(0, mediaType, 0);
			if (result == S_OK) {
				// Success
				mInternals->mAudioDecoderTransform = audioDecoder;

				break;
			}
		}

		// Next
		index++;
	}

	// Create input sample
	TCIResult<IMFSample>	sample = CMediaFoundationServices::createSample(10 * requirements.mFrameCountInterval);
	ReturnErrorIfResultError(sample);
	mInternals->mInputSample = sample.getInstance();

	// Create output sample
	MFT_OUTPUT_STREAM_INFO	outputStreamInfo;
	result = mInternals->mAudioDecoderTransform->GetOutputStreamInfo(0, &outputStreamInfo);
	ReturnErrorIfFailed(result, OSSTR("GetOutputStreamInfo"));

	sample = CMediaFoundationServices::createSample(outputStreamInfo.cbSize);
	ReturnErrorIfResultError(sample);
	mInternals->mOutputSample = sample.getInstance();

	// Begin streaming!
	result = mInternals->mAudioDecoderTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0);
	ReturnErrorIfFailed(result, OSSTR("ProcessMessage to begin streaming"));

	return OI<SError>();
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
OI<SError> CMediaFoundationDecodeAudioCodec::decodeInto(CAudioFrames& audioFrames)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CAudioFrames::Requirements	requirements = getRequirements();

	// Preflight
	AssertFailIf(audioFrames.getAvailableFrameCount() < requirements.mFrameCountMinimum);

	if (!mInternals->mAudioDecoderTransform.hasInstance() || !mInternals->mInputSample.hasInstance() ||
			!mInternals->mOutputSample.hasInstance())
		// Can't decode
		return OI<SError>(sSetupDidNotCompleteError);

	// Fill audio frames as much as we can
	while (audioFrames.getAvailableFrameCount() >= requirements.mFrameCountInterval) {
		// Process output
		OI<SError>	error =
							CMediaFoundationServices::processOutput(*mInternals->mAudioDecoderTransform,
									*mInternals->mOutputSample,
									CMediaFoundationServices::ProcessOutputInfo(
											CMediaFoundationDecodeAudioCodecInternals::fillInputBuffer,
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

	return OI<SError>();
}
