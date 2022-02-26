//----------------------------------------------------------------------------------------------------------------------
//	CH264VideoCodec-Windows.cpp			©2021 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CH264VideoCodec.h"

#include "CLogServices-Windows.h"
#include "CMediaFoundationServices.h"
#include "SError-Windows.h"

#include <mfapi.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CString	sErrorDomain(OSSTR("CH264VideoCodec-Windows"));
static	SError	sSetupDidNotCompleteError(sErrorDomain, 1, CString(OSSTR("Setup did not complete")));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CH264VideoCodecInternals

class CH264VideoCodecInternals {
	public:
										CH264VideoCodecInternals() :
											mOutputSampleRequiredByteCount(0)
											{
												// Finish setup
												::memset(&mOutputSampleDataFormatGUID, 0, sizeof(GUID));
											}

		static	TCIResult<IMFSample>	readInputSample(void* userData);
		static	OI<SError>				noteFormatChanged(IMFMediaType* mediaType, void* userData);

		OV<UInt32>									mTimeScale;
		OI<SVideoProcessingFormat>					mVideoProcessingFormat;
		OI<I<CCodec::DecodeInfo> >					mDecodeInfo;

		OCI<IMFTransform>							mVideoDecoder;
		OI<CH264VideoCodec::DecodeInfo::SPSPPSInfo>	mCurrentSPSPPSInfo;
		OI<CH264VideoCodec::FrameTiming>			mFrameTiming;

		DWORD										mOutputSampleRequiredByteCount;
		GUID										mOutputSampleDataFormatGUID;
		S2DSizeU16									mOutputSampleFrameSize;
		S2DRectU16									mOutputSampleViewRect;
		OCI<IMFSample>								mOutputSample;
};

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
TCIResult<IMFSample> CH264VideoCodecInternals::readInputSample(void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CH264VideoCodecInternals&	internals = *((CH264VideoCodecInternals*) userData);

	// Get next packet
	TIResult<CMediaPacketSource::DataInfo>	dataInfo = (*internals.mDecodeInfo)->getMediaPacketSource()->readNext();
	ReturnValueIfResultError(dataInfo, TCIResult<IMFSample>(dataInfo.getError()));

	// Update frame timing
	TIResult<CH264VideoCodec::FrameTiming::Times>	times = internals.mFrameTiming->updateFrom(dataInfo.getValue());
	ReturnValueIfResultError(dataInfo, TCIResult<IMFSample>(times.getError()));

	// Create input sample
	TArray<CH264VideoCodec::NALUInfo>	naluInfos =
												CH264VideoCodec::NALUInfo::getNALUInfos(dataInfo.getValue().getData());
	CData								annexBData =
												CH264VideoCodec::NALUInfo::composeAnnexB(
														internals.mCurrentSPSPPSInfo->getSPSNALUInfos(),
														internals.mCurrentSPSPPSInfo->getPPSNALUInfos(), naluInfos);
	TCIResult<IMFSample>				sample = CMediaFoundationServices::createSample(annexBData);
	ReturnValueIfResultError(sample, sample);

	HRESULT	result =
					sample.getInstance()->SetSampleTime(
							times.getValue().mPresentationTime * 10000 / *internals.mTimeScale);
	ReturnValueIfFailed(result, "SetSampleTime", TCIResult<IMFSample>(SErrorFromHRESULT(result)));

	result = sample.getInstance()->SetSampleDuration(dataInfo.getValue().getDuration());
	ReturnValueIfFailed(result, "SetSampleDuration", TCIResult<IMFSample>(SErrorFromHRESULT(result)));

	return sample;
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CH264VideoCodecInternals::noteFormatChanged(IMFMediaType* mediaType, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CH264VideoCodecInternals&	internals = *((CH264VideoCodecInternals*) userData);

	// Update Media Type
	HRESULT	result = mediaType->GetGUID(MF_MT_SUBTYPE, &internals.mOutputSampleDataFormatGUID);
	ReturnErrorIfFailed(result, "GetGUID for mediaType in CH264VideoCodecInternals::noteFormatChanged");

	// Get Frame Size
	UINT	width, height;
	result = MFGetAttributeSize(mediaType, MF_MT_FRAME_SIZE, &width, &height);
	ReturnErrorIfFailed(result,
			"MFGetAttributeSize for frame size in CH264VideoCodecInternals::noteFormatChanged");
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
	ReturnErrorIfFailed(result, "GetOutputStreamInfo in CH264VideoCodecInternals::noteFormatChanged");

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
// MARK: - CH264VideoCodec

//----------------------------------------------------------------------------------------------------------------------
CH264VideoCodec::CH264VideoCodec() : CDecodeOnlyVideoCodec()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CH264VideoCodecInternals();
}

//----------------------------------------------------------------------------------------------------------------------
CH264VideoCodec::~CH264VideoCodec()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CVideoCodec methods

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CH264VideoCodec::setupForDecode(const SVideoProcessingFormat& videoProcessingFormat,
		const I<CCodec::DecodeInfo>& decodeInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	DecodeInfo&	h264DecodeInfo = *((DecodeInfo*) &*decodeInfo);

	// Create video decoder
	TCIResult<IMFTransform>	videoDecoder =
									CMediaFoundationServices::createTransformForVideoDecode(MFVideoFormat_H264,
											MFVideoFormat_NV12);
	if (videoDecoder.hasError())
		return OI<SError>(videoDecoder.getError());
	mInternals->mVideoDecoder = videoDecoder.getInstance();

	// Get output stream info
	MFT_OUTPUT_STREAM_INFO	outputStreamInfo;
	HRESULT					result = mInternals->mVideoDecoder->GetOutputStreamInfo(0, &outputStreamInfo);
	ReturnErrorIfFailed(result, "GetOutputStreamInfo");

	mInternals->mOutputSampleRequiredByteCount = outputStreamInfo.cbSize;

	// Finish setup
	mInternals->mTimeScale = OV<UInt32>(h264DecodeInfo.getTimeScale());
	mInternals->mVideoProcessingFormat = OI<SVideoProcessingFormat>(videoProcessingFormat);
	mInternals->mDecodeInfo = OI<I<CCodec::DecodeInfo> >(decodeInfo);

	mInternals->mCurrentSPSPPSInfo = OI<DecodeInfo::SPSPPSInfo>(h264DecodeInfo.getSPSPPSInfo());

	const	NALUInfo&					spsNALUInfo = mInternals->mCurrentSPSPPSInfo->getSPSNALUInfos().getFirst();
			SequenceParameterSetPayload	spsPayload(CData(spsNALUInfo.getBytePtr(), spsNALUInfo.getByteCount(), false));
	mInternals->mFrameTiming = OI<CH264VideoCodec::FrameTiming>(new CH264VideoCodec::FrameTiming(spsPayload));

	// Flush
	OI<SError>	error = CMediaFoundationServices::flush(*mInternals->mVideoDecoder);
	ReturnErrorIfError(error);

	// Begin streaming!
	result = mInternals->mVideoDecoder->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0);
	ReturnErrorIfFailed(result, "ProcessMessage to begin streaming");

	result = mInternals->mVideoDecoder->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, 0);
	ReturnErrorIfFailed(result, "ProcessMessage to begin streaming");

	return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
void CH264VideoCodec::seek(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	DecodeInfo&	h264DecodeInfo = *((DecodeInfo*) &**mInternals->mDecodeInfo);

	// Flush
	CMediaFoundationServices::flush(*mInternals->mVideoDecoder);

	// Seek
	UInt32	frameIndex =
					h264DecodeInfo.getMediaPacketSource()->seekToKeyframe(
							(UInt32) (timeInterval * mInternals->mVideoProcessingFormat->getFramerate() + 0.5),
							h264DecodeInfo.getKeyframeIndexes());
	//mInternals->mNextFrameTime =
	mInternals->mFrameTiming->seek(
			(UInt64) ((UniversalTimeInterval) frameIndex / mInternals->mVideoProcessingFormat->getFramerate() *
					(UniversalTimeInterval) *mInternals->mTimeScale));
}

//----------------------------------------------------------------------------------------------------------------------
TIResult<CVideoFrame> CH264VideoCodec::decode()
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
								CMediaFoundationServices::ProcessOutputInfo(CH264VideoCodecInternals::readInputSample,
										CH264VideoCodecInternals::noteFormatChanged, mInternals));
	mInternals->mOutputSample = OCI<IMFSample>();
	ReturnValueIfError(error, TIResult<CVideoFrame>(*error));

	// Success
	LONGLONG	sampleTime;
	HRESULT		result = sample.getInstance()->GetSampleTime(&sampleTime);
	ReturnValueIfFailed(result, "GetSampleTime", TIResult<CVideoFrame>(SErrorFromHRESULT(result)));

	return TIResult<CVideoFrame>(
			CVideoFrame((UniversalTimeInterval) sampleTime / 10000.0, *sample.getInstance(),
					mInternals->mOutputSampleDataFormatGUID, mInternals->mOutputSampleFrameSize,
					mInternals->mOutputSampleViewRect));
}
