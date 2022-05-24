//----------------------------------------------------------------------------------------------------------------------
//	CMediaFoundationVideoCodec.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CMediaFoundationVideoCodec.h"

#include "CLogServices-Windows.h"
#include "CMediaFoundationServices.h"
#include "SError-Windows.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CString	sErrorDomain(OSSTR("CMediaFoundationVideoCodecs"));
static	SError	sSetupDidNotCompleteError(sErrorDomain, 1, CString(OSSTR("Setup did not complete")));
static	SError	sNoMatchingOutputMediaTypes(sErrorDomain, 2, CString(OSSTR("No matching output media types")));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMediaFoundationDecodeVideoCodecInternals

class CMediaFoundationDecodeVideoCodecInternals {
	public:
										CMediaFoundationDecodeVideoCodecInternals(
												CMediaFoundationDecodeVideoCodec& mediaFoundationDecodeVideoCodec,
												OSType codecID, const I<CMediaPacketSource>& mediaPacketSource,
												UInt32 timeScale, const TNumericArray<UInt32>& keyframeIndexes,
												CMediaFoundationDecodeVideoCodec::ReadInputSampleProc
														readInputSampleProc);

		static	TCIResult<IMFSample>	readInputSample(void* userData);
		static	OI<SError>				noteFormatChanged(IMFMediaType* mediaType, void* userData);

		CMediaFoundationDecodeVideoCodec&						mMediaFoundationDecodeVideoCodec;
		OSType													mCodecID;
		I<CMediaPacketSource>									mMediaPacketSource;
		UInt32													mTimeScale;
		TNumericArray<UInt32>									mKeyframeIndexes;
		CMediaFoundationDecodeVideoCodec::ReadInputSampleProc	mReadInputSampleProc;

		OI<SVideoProcessingFormat>								mVideoProcessingFormat;

		OCI<IMFTransform>										mVideoDecoder;

		DWORD													mOutputSampleRequiredByteCount;
		GUID													mOutputSampleDataFormatGUID;
		S2DSizeU16												mOutputSampleFrameSize;
		S2DRectU16												mOutputSampleViewRect;
		OCI<IMFSample>											mOutputSample;
};

//----------------------------------------------------------------------------------------------------------------------
CMediaFoundationDecodeVideoCodecInternals::CMediaFoundationDecodeVideoCodecInternals(
		CMediaFoundationDecodeVideoCodec& mediaFoundationDecodeVideoCodec, OSType codecID,
		const I<CMediaPacketSource>& mediaPacketSource, UInt32 timeScale,
		const TNumericArray<UInt32>& keyframeIndexes,
		CMediaFoundationDecodeVideoCodec::ReadInputSampleProc readInputSampleProc) :
				mMediaFoundationDecodeVideoCodec(mediaFoundationDecodeVideoCodec), mCodecID(codecID),
				mMediaPacketSource(mediaPacketSource), mTimeScale(timeScale), mKeyframeIndexes(keyframeIndexes),
				mReadInputSampleProc(readInputSampleProc)
//----------------------------------------------------------------------------------------------------------------------
{
	// Finish setup
	::memset(&mOutputSampleDataFormatGUID, 0, sizeof(GUID));
}

//----------------------------------------------------------------------------------------------------------------------
TCIResult<IMFSample> CMediaFoundationDecodeVideoCodecInternals::readInputSample(void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CMediaFoundationDecodeVideoCodecInternals&	internals = *((CMediaFoundationDecodeVideoCodecInternals*) userData);

	// Call proc
	return internals.mReadInputSampleProc(internals.mMediaFoundationDecodeVideoCodec);
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CMediaFoundationDecodeVideoCodecInternals::noteFormatChanged(IMFMediaType* mediaType, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CMediaFoundationDecodeVideoCodecInternals&	internals = *((CMediaFoundationDecodeVideoCodecInternals*) userData);

	// Update Media Type
	HRESULT	result = mediaType->GetGUID(MF_MT_SUBTYPE, &internals.mOutputSampleDataFormatGUID);
	ReturnErrorIfFailed(result, OSSTR("GetGUID for mediaType in CH264VideoCodecInternals::noteFormatChanged"));

	// Get Frame Size
	UINT	width, height;
	result = MFGetAttributeSize(mediaType, MF_MT_FRAME_SIZE, &width, &height);
	ReturnErrorIfFailed(result,
			OSSTR("MFGetAttributeSize for frame size in CH264VideoCodecInternals::noteFormatChanged"));
	internals.mOutputSampleFrameSize = S2DSizeU16((UInt16) width, (UInt16) height);

	// Try to get Geometric Aperture
	MFVideoArea	videoArea;
	result = mediaType->GetBlob(MF_MT_GEOMETRIC_APERTURE, (UINT8*) &videoArea, sizeof(MFVideoArea), NULL);
	if (SUCCEEDED(result))
		// Have Geometric Aperture
		internals.mOutputSampleViewRect =
				S2DRectU16(S2DPointU16(videoArea.OffsetX.value, videoArea.OffsetY.value),
						S2DSizeU16((UInt16) videoArea.Area.cx, (UInt16) videoArea.Area.cy));
	else {
		// Try to get Pan & Scan Aperture
		result = mediaType->GetBlob(MF_MT_PAN_SCAN_APERTURE, (UINT8*) &videoArea, sizeof(MFVideoArea), NULL);
		if (SUCCEEDED(result))
			// Have Pan & Scan Apertrue
			internals.mOutputSampleViewRect =
					S2DRectU16(S2DPointU16(videoArea.OffsetX.value, videoArea.OffsetY.value),
							S2DSizeU16((UInt16) videoArea.Area.cx, (UInt16) videoArea.Area.cy));
		else
			// Don't have aperture
			internals.mOutputSampleViewRect = S2DRectU16(S2DPointU16(), internals.mOutputSampleFrameSize);
	}

	// Get output stream info
	MFT_OUTPUT_STREAM_INFO	outputStreamInfo;
	result = internals.mVideoDecoder->GetOutputStreamInfo(0, &outputStreamInfo);
	ReturnErrorIfFailed(result, OSSTR("GetOutputStreamInfo in CH264VideoCodecInternals::noteFormatChanged"));

	// Store
	internals.mOutputSampleRequiredByteCount = outputStreamInfo.cbSize;

	// Resize current sample
	OI<SError>	error =
						CMediaFoundationServices::resizeSample(*internals.mOutputSample,
								internals.mOutputSampleRequiredByteCount);
	ReturnErrorIfError(error);

	return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMediaFoundationDecodeVideoCodec

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CMediaFoundationDecodeVideoCodec::CMediaFoundationDecodeVideoCodec(OSType codecID,
		const I<CMediaPacketSource>& mediaPacketSource, UInt32 timeScale, const TNumericArray<UInt32>& keyframeIndexes,
		ReadInputSampleProc readInputSampleProc)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals =
			new CMediaFoundationDecodeVideoCodecInternals(*this, codecID, mediaPacketSource, timeScale, keyframeIndexes,
					readInputSampleProc);
}

//----------------------------------------------------------------------------------------------------------------------
CMediaFoundationDecodeVideoCodec::~CMediaFoundationDecodeVideoCodec()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CDecodeVideoCodec methods

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CMediaFoundationDecodeVideoCodec::setup(const SVideoProcessingFormat& videoProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	GUID&	guid = getGUID();
			HRESULT	result;

	// Enum Video Codecs to find Video Decoder
	MFT_REGISTER_TYPE_INFO	info = {MFMediaType_Video, guid};
	IMFActivate**			activates;
	UINT32					count;
	result =
			::MFTEnumEx(MFT_CATEGORY_VIDEO_DECODER,
					MFT_ENUM_FLAG_SYNCMFT | MFT_ENUM_FLAG_LOCALMFT | MFT_ENUM_FLAG_SORTANDFILTER, &info, NULL,
					&activates, &count);
	ReturnErrorIfFailed(result, OSSTR("MFTEnumEx"));

	// Create the Video Decoder
	IMFTransform*	transform;
	result = activates[0]->ActivateObject(IID_PPV_ARGS(&transform));
	OCI<IMFTransform>	videoDecoder(transform);

	for (UINT32 i = 0; i < count; i++)
		// Release
		activates[i]->Release();
	::CoTaskMemFree(activates);

	ReturnErrorIfFailed(result, OSSTR("ActivateObject"));

	// Setup input media type
	IMFMediaType*	mediaType;
	result = ::MFCreateMediaType(&mediaType);
	ReturnErrorIfFailed(result, OSSTR("MFCreateMediaType"));
	TCIResult<IMFMediaType>	inputMediaType(mediaType);

	result = mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	ReturnErrorIfFailed(result, OSSTR("SetGUID of MF_MT_MAJOR_TYPE for input"));

	result = mediaType->SetGUID(MF_MT_SUBTYPE, guid);
	ReturnErrorIfFailed(result, OSSTR("SetGUID of MF_MT_SUBTYPE for input"));

	const	S2DSizeU16	frameSize = videoProcessingFormat.getFrameSize();
	result = ::MFSetAttributeSize(mediaType, MF_MT_FRAME_SIZE, frameSize.mWidth, frameSize.mHeight);
	ReturnErrorIfFailed(result, OSSTR("MFSetAttributeSize for input"));

	result = transform->SetInputType(0, *(inputMediaType.getInstance()), 0);
	ReturnErrorIfFailed(result, OSSTR("SetInputType"));

	// Iterate output media types to find matching
	DWORD	index = 0;
	while (true) {
		// Get next media type
		result = transform->GetOutputAvailableType(0, index, &mediaType);
		if (result != S_OK)
			// No matching output media types
			return OI<SError>(sNoMatchingOutputMediaTypes);

		// Get info
		GUID	codecSubType;
		result = mediaType->GetGUID(MF_MT_SUBTYPE, &codecSubType);
		if (result != S_OK)
			continue;

		// Compare codec subtype, bits, and channels
		if (codecSubType == MFVideoFormat_NV12) {
			// Found match
			result = transform->SetOutputType(0, mediaType, 0);
			if (result == S_OK) {
				// Success
				mInternals->mVideoDecoder = videoDecoder;

				break;
			}
		}

		// Next
		index++;
	}

	// Get output stream info
	MFT_OUTPUT_STREAM_INFO	outputStreamInfo;
	result = mInternals->mVideoDecoder->GetOutputStreamInfo(0, &outputStreamInfo);
	ReturnErrorIfFailed(result, OSSTR("GetOutputStreamInfo"));

	mInternals->mOutputSampleRequiredByteCount = outputStreamInfo.cbSize;

	// Finish setup
	mInternals->mVideoProcessingFormat = OI<SVideoProcessingFormat>(videoProcessingFormat);

	// Flush
	OI<SError>	error = CMediaFoundationServices::flush(*mInternals->mVideoDecoder);
	ReturnErrorIfError(error);

	// Begin streaming!
	result = mInternals->mVideoDecoder->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0);
	ReturnErrorIfFailed(result, OSSTR("ProcessMessage to begin streaming"));

	result = mInternals->mVideoDecoder->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, 0);
	ReturnErrorIfFailed(result, OSSTR("ProcessMessage to begin streaming"));

	return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaFoundationDecodeVideoCodec::seek(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Flush
	CMediaFoundationServices::flush(*mInternals->mVideoDecoder);

	// Seek
	UInt32	frameIndex =
					mInternals->mMediaPacketSource->seekToKeyframe(
							(UInt32) (timeInterval * mInternals->mVideoProcessingFormat->getFramerate() + 0.5),
							mInternals->mKeyframeIndexes);
	seek(
			(UInt64) ((UniversalTimeInterval) frameIndex / mInternals->mVideoProcessingFormat->getFramerate() *
					(UniversalTimeInterval) mInternals->mTimeScale));
}

//----------------------------------------------------------------------------------------------------------------------
TIResult<CVideoFrame> CMediaFoundationDecodeVideoCodec::decode()
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	if (!mInternals->mVideoDecoder.hasInstance())
		// Can't decode
		return TIResult<CVideoFrame>(sSetupDidNotCompleteError);

	// Setup sample
	TCIResult<IMFSample>	sample = CMediaFoundationServices::createSample(mInternals->mOutputSampleRequiredByteCount);
	ReturnValueIfResultError(sample, TIResult<CVideoFrame>(sample.getError()));
	mInternals->mOutputSample = sample.getInstance();

	// Process output
	OI<SError>	error =
						CMediaFoundationServices::processOutput(*mInternals->mVideoDecoder, *mInternals->mOutputSample,
								CMediaFoundationServices::ProcessOutputInfo(
										CMediaFoundationDecodeVideoCodecInternals::readInputSample,
										CMediaFoundationDecodeVideoCodecInternals::noteFormatChanged, mInternals));
	mInternals->mOutputSample = OCI<IMFSample>();
	ReturnValueIfError(error, TIResult<CVideoFrame>(*error));

	// Success
	LONGLONG	sampleTime;
	HRESULT		result = sample.getInstance()->GetSampleTime(&sampleTime);
	ReturnValueIfFailed(result, OSSTR("GetSampleTime"), TIResult<CVideoFrame>(SErrorFromHRESULT(result)));

	return TIResult<CVideoFrame>(
			CVideoFrame((UniversalTimeInterval) sampleTime / 10000.0, *sample.getInstance(),
					mInternals->mOutputSampleDataFormatGUID, mInternals->mOutputSampleFrameSize,
					mInternals->mOutputSampleViewRect));
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CMediaFoundationDecodeVideoCodec::getTimeScale() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mTimeScale;
}
