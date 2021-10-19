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
											mOutputSampleRequiredByteCount(0), mCurrentFrameNumberBitCount(0),
													mCurrentPicOrderCountLSBBitCount(0),
													mPicOrderCountMSBChangeThreshold(0),
													mPicOrderCountMSB(0), mPreviousPicOrderCountLSB(0),
													mLastIDRFrameTime(0), mNextFrameTime(0)
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

		DWORD										mOutputSampleRequiredByteCount;
		GUID										mOutputSampleDataFormatGUID;
		S2DSizeU16									mOutputSampleFrameSize;
		S2DRectU16									mOutputSampleViewRect;
		OCI<IMFSample>								mOutputSample;

		OI<CH264VideoCodec::DecodeInfo::SPSPPSInfo>	mCurrentSPSPPSInfo;
		UInt8										mCurrentFrameNumberBitCount;
		UInt8										mCurrentPicOrderCountLSBBitCount;
		UInt8										mPicOrderCountMSBChangeThreshold;

		UInt64										mPicOrderCountMSB;
		UInt64										mPreviousPicOrderCountLSB;
		UInt64										mLastIDRFrameTime;
		UInt64										mNextFrameTime;
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

UInt8	naluUnitType;
UInt8	sliceType;
UInt8	frameNum;
UInt8	picOrderCntLSB;
UInt8	deltaPicOrderCntBottom;

const	CData&		data = dataInfo.getValue().getData();
		CBitReader	bitReader(I<CSeekableDataSource>(new CDataDataSource(data)), true);

		UInt32		duration = dataInfo.getValue().getDuration();

while (true) {
	//
	OV<UInt32>						size = bitReader.readUInt32().getValue();
	UInt64							pos = bitReader.getPos();

	OV<UInt8>						forbidden_zero_bit = bitReader.readUInt8(1).getValue();
	OV<UInt8>						nal_ref_idc = bitReader.readUInt8(2).getValue();
	OV<UInt8>						nal_unit_type = bitReader.readUInt8(5).getValue();
	CH264VideoCodec::NALUInfo::Type	naluType = (CH264VideoCodec::NALUInfo::Type) *nal_unit_type;

	if (naluType == CH264VideoCodec::NALUInfo::kTypeCodedSliceNonIDRPicture) {
		// Coded Slice Non-IDR Picture
		OV<UInt32>	first_mb_in_slice = bitReader.readUEColumbusCode().getValue();
		OV<UInt32>	slice_type = bitReader.readUEColumbusCode().getValue();
		OV<UInt32>	pic_parameter_set_id = bitReader.readUEColumbusCode().getValue();
		OV<UInt8>	frame_num = bitReader.readUInt8(internals.mCurrentFrameNumberBitCount).getValue();
		OV<UInt8>	pic_order_cnt_lsb = bitReader.readUInt8(internals.mCurrentPicOrderCountLSBBitCount).getValue();
		OV<UInt32>	delta_pic_order_cnt_bottom = bitReader.readUEColumbusCode().getValue();

		naluUnitType = *nal_unit_type;
		sliceType = *slice_type;
		frameNum = *frame_num;
		picOrderCntLSB = *pic_order_cnt_lsb;
		deltaPicOrderCntBottom = *delta_pic_order_cnt_bottom;
		break;
	} else if (naluType == CH264VideoCodec::NALUInfo::kTypeCodedSliceIDRPicture) {
		// Coded Slice IDR Picture
		OV<UInt32>	first_mb_in_slice = bitReader.readUEColumbusCode().getValue();
		OV<UInt32>	slice_type = bitReader.readUEColumbusCode().getValue();
		OV<UInt32>	pic_parameter_set_id = bitReader.readUEColumbusCode().getValue();
		OV<UInt8>	frame_num = bitReader.readUInt8(internals.mCurrentFrameNumberBitCount).getValue();
		OV<UInt32>	idr_pic_id = bitReader.readUEColumbusCode().getValue();
		OV<UInt8>	pic_order_cnt_lsb = bitReader.readUInt8(internals.mCurrentPicOrderCountLSBBitCount).getValue();
		OV<UInt32>	delta_pic_order_cnt_bottom = bitReader.readUEColumbusCode().getValue();

		naluUnitType = *nal_unit_type;
		sliceType = *slice_type;
		frameNum = *frame_num;
		picOrderCntLSB = *pic_order_cnt_lsb;
		deltaPicOrderCntBottom = *delta_pic_order_cnt_bottom;
		break;
	} else if (naluType == CH264VideoCodec::NALUInfo::kTypeSupplementalEnhancementInformation) {
		// SEI
	} else {
		// Unhandled
		CLogServices::logMessage(CString("Unhandled NALU type: ") + CString(naluType));
	}

	// Next NALU
	OI<SError>	error = bitReader.setPos(CBitReader::kPositionFromBeginning, pos + *size);
}
//CLogServices::logMessage(
//		CString("Packet ") + CString(mInternals->mNextFrameIndex) + CString(" (") + CString(data->getSize()) +
//				CString("), dts ") + CString((SInt32) mInternals->mNextFrameIndex * 100 - 100) +
//				CString(", nal_unit_type: ") + CString(naluUnitType) +
//				CString(", slice_type: ") + CString(sliceType) +
//				CString(", frame_num: ") + CString(frameNum) +
//				CString(", pic_order_cnt_lsb: ") + CString(picOrderCntLSB) +
////				CString(", delta_pic_order_cnt_bottom: ") + CString(deltaPicOrderCntBottom) +
//				CString::mEmpty
//				);

	// Create input sample
	TArray<CH264VideoCodec::NALUInfo>	naluInfos = CH264VideoCodec::NALUInfo::getNALUInfos(data);
	CData								annexBData =
												CH264VideoCodec::NALUInfo::composeAnnexB(
														internals.mCurrentSPSPPSInfo->getSPSNALUInfos(),
														internals.mCurrentSPSPPSInfo->getPPSNALUInfos(), naluInfos);
	TCIResult<IMFSample>				sample = CMediaFoundationServices::createSample(annexBData);
	ReturnValueIfResultError(sample, sample);

UInt64	frameTime;
if (sliceType == 2) {
	// IDR
	frameTime = internals.mNextFrameTime;
	internals.mPicOrderCountMSB = 0;
	internals.mPreviousPicOrderCountLSB = 0;
	internals.mLastIDRFrameTime = internals.mNextFrameTime;
} else {
	// Non-IDR
	if ((picOrderCntLSB > internals.mPreviousPicOrderCountLSB) &&
			((picOrderCntLSB - internals.mPreviousPicOrderCountLSB) > internals.mPicOrderCountMSBChangeThreshold))
		//
		internals.mPicOrderCountMSB -= (UInt64) 1 << internals.mCurrentPicOrderCountLSBBitCount;
	else if ((internals.mPreviousPicOrderCountLSB > picOrderCntLSB) &&
			((internals.mPreviousPicOrderCountLSB - picOrderCntLSB) > internals.mPicOrderCountMSBChangeThreshold))
		//
		internals.mPicOrderCountMSB += (UInt64) 1 << internals.mCurrentPicOrderCountLSBBitCount;

	frameTime = internals.mLastIDRFrameTime + (internals.mPicOrderCountMSB + picOrderCntLSB) / 2 * duration;

	internals.mPreviousPicOrderCountLSB = picOrderCntLSB;
}

	HRESULT	result = sample.getInstance()->SetSampleTime(frameTime * 10000 / *internals.mTimeScale);
	LogHRESULTIfFailed(result, "SetSampleTime");

	result = sample.getInstance()->SetSampleDuration(duration);
	LogHRESULTIfFailed(result, "SetSampleDuration");

	// Update
	internals.mNextFrameTime += duration;

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
	internals.mOutputSampleFrameSize = S2DSizeU16(width, height);

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
void CH264VideoCodec::setupForDecode(const SVideoProcessingFormat& videoProcessingFormat,
		const I<CCodec::DecodeInfo>& decodeInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	DecodeInfo&	h264DecodeInfo = *((DecodeInfo*) &*decodeInfo);

	// Create video decoder
	TCIResult<IMFTransform>	videoDecoder = CMediaFoundationServices::createTransformForVideoDecode(MFVideoFormat_H264);
	if (videoDecoder.hasError())
		return;
	mInternals->mVideoDecoder = videoDecoder.getInstance();

	// Get output stream info
	MFT_OUTPUT_STREAM_INFO	outputStreamInfo;
	HRESULT					result = mInternals->mVideoDecoder->GetOutputStreamInfo(0, &outputStreamInfo);
	if (FAILED(result)) {
		// Failed
		LogHRESULT(result, "GetOutputStreamInfo");

		return;
	}
	mInternals->mOutputSampleRequiredByteCount = outputStreamInfo.cbSize;

	// Finish setup
	mInternals->mTimeScale = OV<UInt32>(h264DecodeInfo.getTimeScale());
	mInternals->mVideoProcessingFormat = OI<SVideoProcessingFormat>(videoProcessingFormat);
	mInternals->mDecodeInfo = OI<I<CCodec::DecodeInfo> >(decodeInfo);

	mInternals->mCurrentSPSPPSInfo = OI<DecodeInfo::SPSPPSInfo>(h264DecodeInfo.getSPSPPSInfo());

	const	NALUInfo&					spsNALUInfo = mInternals->mCurrentSPSPPSInfo->getSPSNALUInfos().getFirst();
			SequenceParameterSetPayload	spsPayload(CData(spsNALUInfo.getBytePtr(), spsNALUInfo.getSize(), false));
	mInternals->mCurrentFrameNumberBitCount = spsPayload.mFrameNumberBitCount;
	mInternals->mCurrentPicOrderCountLSBBitCount = spsPayload.mPicOrderCountLSBBitCount;
	mInternals->mPicOrderCountMSBChangeThreshold = 1 << (mInternals->mCurrentPicOrderCountLSBBitCount - 1);

	// Flush
	OI<SError>	error = CMediaFoundationServices::flush(*mInternals->mVideoDecoder);

	// Begin streaming!
	result = mInternals->mVideoDecoder->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0);
	LogHRESULTIfFailed(result, "ProcessMessage to begin streaming");

	result = mInternals->mVideoDecoder->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, 0);
	LogHRESULTIfFailed(result, "ProcessMessage to begin streaming");
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
	mInternals->mNextFrameTime =
			(UInt64) ((UniversalTimeInterval) frameIndex / mInternals->mVideoProcessingFormat->getFramerate() *
					(UniversalTimeInterval) *mInternals->mTimeScale);
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
