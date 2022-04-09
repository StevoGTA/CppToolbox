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
// MARK: - CH264DecodeVideoCodec

class CH264DecodeVideoCodec : public CDecodeVideoCodec {
	public:
										// Lifecycle methods
										CH264DecodeVideoCodec(const I<CMediaPacketSource>& mediaPacketSource,
												const CData& configurationData, UInt32 timeScale,
												const TNumericArray<UInt32>& keyframeIndexes) :
											mDecodeInfo(mediaPacketSource, configurationData, timeScale,
															keyframeIndexes),
													mOutputSampleRequiredByteCount(0)
											{
												// Finish setup
												::memset(&mOutputSampleDataFormatGUID, 0, sizeof(GUID));
											}

										// CVideoCodec methods
				OI<SError>				setup(const SVideoProcessingFormat& videoProcessingFormat);
				void					seek(UniversalTimeInterval timeInterval);
				TIResult<CVideoFrame>	decode();

										// Class methods
		static	TCIResult<IMFSample>	readInputSample(void* userData);
		static	OI<SError>				noteFormatChanged(IMFMediaType* mediaType, void* userData);

	private:
		CH264VideoCodec::DecodeInfo					mDecodeInfo;

		OV<UInt32>									mTimeScale;
		OI<SVideoProcessingFormat>					mVideoProcessingFormat;

		OCI<IMFTransform>							mVideoDecoder;
		OI<CH264VideoCodec::DecodeInfo::SPSPPSInfo>	mCurrentSPSPPSInfo;
		OI<CH264VideoCodec::FrameTiming>			mFrameTiming;

		DWORD										mOutputSampleRequiredByteCount;
		GUID										mOutputSampleDataFormatGUID;
		S2DSizeU16									mOutputSampleFrameSize;
		S2DRectU16									mOutputSampleViewRect;
		OCI<IMFSample>								mOutputSample;
};

// MARK: CVideoCodec methods

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CH264DecodeVideoCodec::setup(const SVideoProcessingFormat& videoProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Create video decoder
	TCIResult<IMFTransform>	videoDecoder =
									CMediaFoundationServices::createTransformForVideoDecode(MFVideoFormat_H264,
											MFVideoFormat_NV12);
	if (videoDecoder.hasError())
		return OI<SError>(videoDecoder.getError());
	mVideoDecoder = videoDecoder.getInstance();

	// Get output stream info
	MFT_OUTPUT_STREAM_INFO	outputStreamInfo;
	HRESULT					result = mVideoDecoder->GetOutputStreamInfo(0, &outputStreamInfo);
	ReturnErrorIfFailed(result, "GetOutputStreamInfo");

	mOutputSampleRequiredByteCount = outputStreamInfo.cbSize;

	// Finish setup
	mTimeScale = OV<UInt32>(mDecodeInfo.getTimeScale());
	mVideoProcessingFormat = OI<SVideoProcessingFormat>(videoProcessingFormat);

	mCurrentSPSPPSInfo = OI<CH264VideoCodec::DecodeInfo::SPSPPSInfo>(mDecodeInfo.getSPSPPSInfo());

	const	CH264VideoCodec::NALUInfo&						spsNALUInfo =
																	mCurrentSPSPPSInfo->getSPSNALUInfos().getFirst();
			CH264VideoCodec::SequenceParameterSetPayload	spsPayload(CData(spsNALUInfo.getBytePtr(),
																	spsNALUInfo.getByteCount(), false));
	mFrameTiming = OI<CH264VideoCodec::FrameTiming>(new CH264VideoCodec::FrameTiming(spsPayload));

	// Flush
	OI<SError>	error = CMediaFoundationServices::flush(*mVideoDecoder);
	ReturnErrorIfError(error);

	// Begin streaming!
	result = mVideoDecoder->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0);
	ReturnErrorIfFailed(result, "ProcessMessage to begin streaming");

	result = mVideoDecoder->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, 0);
	ReturnErrorIfFailed(result, "ProcessMessage to begin streaming");

	return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
void CH264DecodeVideoCodec::seek(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Flush
	CMediaFoundationServices::flush(*mVideoDecoder);

	// Seek
	UInt32	frameIndex =
					mDecodeInfo.getMediaPacketSource()->seekToKeyframe(
							(UInt32) (timeInterval * mVideoProcessingFormat->getFramerate() + 0.5),
							mDecodeInfo.getKeyframeIndexes());
	mFrameTiming->seek(
			(UInt64) ((UniversalTimeInterval) frameIndex / mVideoProcessingFormat->getFramerate() *
					(UniversalTimeInterval) *mTimeScale));
}

//----------------------------------------------------------------------------------------------------------------------
TIResult<CVideoFrame> CH264DecodeVideoCodec::decode()
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	if (!mVideoDecoder.hasInstance())
		// Can't decode
		return TIResult<CVideoFrame>(sSetupDidNotCompleteError);

	// Setup sample
	TCIResult<IMFSample>	sample = CMediaFoundationServices::createSample(mOutputSampleRequiredByteCount);
	ReturnValueIfResultError(sample, TIResult<CVideoFrame>(sample.getError()));
	mOutputSample = sample.getInstance();

	// Process output
	OI<SError>	error =
						CMediaFoundationServices::processOutput(*mVideoDecoder, *mOutputSample,
								CMediaFoundationServices::ProcessOutputInfo(readInputSample, noteFormatChanged, this));
	mOutputSample = OCI<IMFSample>();
	ReturnValueIfError(error, TIResult<CVideoFrame>(*error));

	// Success
	LONGLONG	sampleTime;
	HRESULT		result = sample.getInstance()->GetSampleTime(&sampleTime);
	ReturnValueIfFailed(result, "GetSampleTime", TIResult<CVideoFrame>(SErrorFromHRESULT(result)));

	return TIResult<CVideoFrame>(
			CVideoFrame((UniversalTimeInterval) sampleTime / 10000.0, *sample.getInstance(),
					mOutputSampleDataFormatGUID, mOutputSampleFrameSize, mOutputSampleViewRect));
}
// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
TCIResult<IMFSample> CH264DecodeVideoCodec::readInputSample(void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CH264DecodeVideoCodec&	videoCodec = *((CH264DecodeVideoCodec*) userData);

	// Get next packet
	TIResult<CMediaPacketSource::DataInfo>	dataInfo = videoCodec.mDecodeInfo.getMediaPacketSource()->readNext();
	ReturnValueIfResultError(dataInfo, TCIResult<IMFSample>(dataInfo.getError()));

	// Update frame timing
	TIResult<CH264VideoCodec::FrameTiming::Times>	times = videoCodec.mFrameTiming->updateFrom(*dataInfo);
	ReturnValueIfResultError(dataInfo, TCIResult<IMFSample>(times.getError()));

	// Create input sample
	TArray<CH264VideoCodec::NALUInfo>	naluInfos = CH264VideoCodec::NALUInfo::getNALUInfos(dataInfo->getData());
	CData								annexBData =
												CH264VideoCodec::NALUInfo::composeAnnexB(
														videoCodec.mCurrentSPSPPSInfo->getSPSNALUInfos(),
														videoCodec.mCurrentSPSPPSInfo->getPPSNALUInfos(), naluInfos);
	TCIResult<IMFSample>				sample = CMediaFoundationServices::createSample(annexBData);
	ReturnValueIfResultError(sample, sample);

	HRESULT	result = sample.getInstance()->SetSampleTime(times->mPresentationTime * 10000 / *videoCodec.mTimeScale);
	ReturnValueIfFailed(result, "SetSampleTime", TCIResult<IMFSample>(SErrorFromHRESULT(result)));

	result = sample.getInstance()->SetSampleDuration(dataInfo->getDuration());
	ReturnValueIfFailed(result, "SetSampleDuration", TCIResult<IMFSample>(SErrorFromHRESULT(result)));

	return sample;
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CH264DecodeVideoCodec::noteFormatChanged(IMFMediaType* mediaType, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CH264DecodeVideoCodec&	videoCodec = *((CH264DecodeVideoCodec*) userData);

	// Update Media Type
	HRESULT	result = mediaType->GetGUID(MF_MT_SUBTYPE, &videoCodec.mOutputSampleDataFormatGUID);
	ReturnErrorIfFailed(result, "GetGUID for mediaType in CH264VideoCodecInternals::noteFormatChanged");

	// Get Frame Size
	UINT	width, height;
	result = MFGetAttributeSize(mediaType, MF_MT_FRAME_SIZE, &width, &height);
	ReturnErrorIfFailed(result,
			"MFGetAttributeSize for frame size in CH264VideoCodecInternals::noteFormatChanged");
	videoCodec.mOutputSampleFrameSize = S2DSizeU16((UInt16) width, (UInt16) height);

	// Try to get Geometric Aperture
	MFVideoArea	videoArea;
	result = mediaType->GetBlob(MF_MT_GEOMETRIC_APERTURE, (UINT8*) &videoArea, sizeof(MFVideoArea), NULL);
	if (SUCCEEDED(result))
		// Have Geometric Aperture
		videoCodec.mOutputSampleViewRect =
				S2DRectU16(S2DPointU16(videoArea.OffsetX.value, videoArea.OffsetY.value),
						S2DSizeU16((UInt16) videoArea.Area.cx, (UInt16) videoArea.Area.cy));
	else {
		// Try to get Pan & Scan Aperture
		result = mediaType->GetBlob(MF_MT_PAN_SCAN_APERTURE, (UINT8*) &videoArea, sizeof(MFVideoArea), NULL);
		if (SUCCEEDED(result))
			// Have Pan & Scan Apertrue
			videoCodec.mOutputSampleViewRect =
					S2DRectU16(S2DPointU16(videoArea.OffsetX.value, videoArea.OffsetY.value),
							S2DSizeU16((UInt16) videoArea.Area.cx, (UInt16) videoArea.Area.cy));
		else
			// Don't have aperture
			videoCodec.mOutputSampleViewRect = S2DRectU16(S2DPointU16(), videoCodec.mOutputSampleFrameSize);
	}

	// Get output stream info
	MFT_OUTPUT_STREAM_INFO	outputStreamInfo;
	result = videoCodec.mVideoDecoder->GetOutputStreamInfo(0, &outputStreamInfo);
	ReturnErrorIfFailed(result, "GetOutputStreamInfo in CH264VideoCodecInternals::noteFormatChanged");

	// Store
	videoCodec.mOutputSampleRequiredByteCount = outputStreamInfo.cbSize;

	// Resize current sample
	OI<SError>	error =
						CMediaFoundationServices::resizeSample(*videoCodec.mOutputSample,
								videoCodec.mOutputSampleRequiredByteCount);
	ReturnErrorIfError(error);

	return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CH264VideoCodec

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
I<CDecodeVideoCodec> CH264VideoCodec::create(const I<CSeekableDataSource>& seekableDataSource,
		const TArray<SMediaPacketAndLocation>& packetAndLocations, const CData& configurationData, UInt32 timeScale,
		const TNumericArray<UInt32>& keyframeIndexes)
//----------------------------------------------------------------------------------------------------------------------
{
	return I<CDecodeVideoCodec>(
			new CH264DecodeVideoCodec(
					I<CMediaPacketSource>(
							new CSeekableVaryingMediaPacketSource(seekableDataSource, packetAndLocations)),
					configurationData, timeScale, keyframeIndexes));
}
