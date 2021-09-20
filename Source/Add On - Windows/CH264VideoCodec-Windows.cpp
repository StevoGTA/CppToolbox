//----------------------------------------------------------------------------------------------------------------------
//	CH264VideoCodec-Windows.cpp			©2021 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CH264VideoCodec.h"


#include "CLogServices-Windows.h"
#include "CMediaFoundationServices.h"
#include "SError-Windows.h"

//#undef Delete

#include <mfapi.h>

//#define Delete(x)	{ delete x; x = nil; }

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
											mTimeScale(0),
													mOutputSampleRequiredByteCount(0),
													mCurrentFrameNumberBitCount(0), mCurrentPicOrderCountLSBBitCount(0),
													mPicOrderCountMSBChangeThreshold(0),
													mPicOrderCountMSB(0), mPreviousPicOrderCountLSB(0),
													mLastIDRFrameTime(0), mNextFrameTime(0)
											{
												// Finish setup
												::memset(&mOutputSampleDataFormatGUID, 0, sizeof(GUID));
											}

		static	TCIResult<IMFSample>	readInputSample(void* userData);
		static	OI<SError>				noteFormatChanged(IMFMediaType* mediaType, void* userData);

		OR<CPacketMediaReader>						mPacketMediaReader;
		UInt32										mTimeScale;

		OCI<IMFTransform>							mVideoDecoder;

		DWORD										mOutputSampleRequiredByteCount;
		GUID										mOutputSampleDataFormatGUID;
		S2DSizeU16									mOutputSampleFrameSize;
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

	// Read next packet
	TIResult<CPacketMediaReader::MediaPacketDataInfo>	mediaPacketDataInfo =
																internals.mPacketMediaReader->
																		readNextMediaPacketDataInfo();
	ReturnValueIfResultError(mediaPacketDataInfo, TCIResult<IMFSample>(mediaPacketDataInfo.getError()));

//CLogServices::logMessage(
//		CString("Packet ") + CString(mInternals->mNextFrameIndex) + CString(" (") + CString(data->getSize()) +
//				CString("), dts ") + CString((SInt32) mInternals->mNextFrameIndex * 100 - 100));
//	// Get NALUs
//	TArray<NALUInfo>	naluInfos = getNALUInfos(*data);
//	for (CArray::ItemIndex i = 0; i < naluInfos.getCount(); i++) {
//		//
//		NALUInfo::Type	naluType = naluInfos[i].getType();
//		if (naluType == NALUInfo::kTypeCodedSliceNonIDRPicture) {
//			 // Coded Slice Non-IDR Picture
//			CodedSliceNonIDRPicturePayload	payload(naluInfos[i]);
//			UInt8	frameNum = payload.getFrameNum();
//			UInt8	picOrderCntLSB = payload.getPicOrderCntLsb();
//
//			CLogServices::logMessage(
//					CString("    NALU ") + CString(i) +
//							CString(" (Coded Slice non-IDR Picture), frame_num: ") + CString(frameNum) +
//							CString(", pic_order_cnt_lsb: ") + CString(picOrderCntLSB));
//			break;
//		} else if (naluType == NALUInfo::kTypeCodedSliceIDRPicture) {
//			// Coded Slice IDR Picture
//			CodedSliceIDRPictureNALUPayload	payload(naluInfos[i]);
//			UInt8	frameNum = payload.getFrameNum();
//			UInt8	picOrderCntLSB = payload.getPicOrderCntLsb();
//
//			CLogServices::logMessage(
//					CString("    NALU ") + CString(i) +
//							CString(" (Coded Slice IDR Picture), frame_num: ") + CString(frameNum) +
//							CString(", pic_order_cnt_lsb: ") + CString(picOrderCntLSB));
//			break;
//		}
//	}

UInt8	naluUnitType;
UInt8	sliceType;
UInt8	frameNum;
UInt8	picOrderCntLSB;
UInt8	deltaPicOrderCntBottom;

const	CData&		data = mediaPacketDataInfo.getValue().getData();
		CBitReader	bitReader(I<CSeekableDataSource>(new CDataDataSource(data)), true);

		UInt32		duration = mediaPacketDataInfo.getValue().getDuration();

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
	CData				annexBData =
								CH264VideoCodec::NALUInfo::composeAnnexB(internals.mCurrentSPSPPSInfo->getSPSNALUInfos(),
										internals.mCurrentSPSPPSInfo->getPPSNALUInfos(), naluInfos);
	//internals.mInputSample = CMediaFoundationServices::createSample(annexBData);
	TCIResult<IMFSample>	sample = CMediaFoundationServices::createSample(annexBData);
	ReturnValueIfResultError(sample, sample);
//IMFSample*	inputSample = *sample;
//inputSample->AddRef();



	//if (!inputSample.hasInstance())
	//	return false;

	//CMSampleTimingInfo	sampleTimingInfo;
	//sampleTimingInfo.duration = ::CMTimeMake(packetAndLocation.mPacket.mDuration, mInternals->mTimeScale);

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

//sampleTimingInfo.presentationTimeStamp = ::CMTimeMake(frameTime, mInternals->mTimeScale);

	//HRESULT	result = internals.mInputSample->SetSampleTime(frameTime * internals.mTimeScale);
	HRESULT	result = sample.getInstance()->SetSampleTime(frameTime * internals.mTimeScale);
	LogHRESULTIfFailed(result, "SetSampleTime");

	//result = internals.mInputSample->SetSampleDuration(duration);
	result = sample.getInstance()->SetSampleDuration(duration);
	LogHRESULTIfFailed(result, "SetSampleDuration");

	// Update
	internals.mNextFrameTime += duration;

	return sample;
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError>	CH264VideoCodecInternals::noteFormatChanged(IMFMediaType* mediaType, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CH264VideoCodecInternals&	internals = *((CH264VideoCodecInternals*) userData);

	//// Update Media Type
	//HRESULT	result = mediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_IYUV);
	//ReturnErrorIfFailed(result, "SetGUID for mediaType in CH264VideoCodecInternals::noteFormatChanged");
	HRESULT	result = mediaType->GetGUID(MF_MT_SUBTYPE, &internals.mOutputSampleDataFormatGUID);
	ReturnErrorIfFailed(result, "GetGUID for mediaType in CH264VideoCodecInternals::noteFormatChanged");

	UINT32	width, height;
	result = MFGetAttributeSize(mediaType, MF_MT_FRAME_SIZE, &width, &height);
	ReturnErrorIfFailed(result, "MFGetAttributeSize for frame size in CH264VideoCodecInternals::noteFormatChanged");
	internals.mOutputSampleFrameSize = S2DSizeU16(width, height);

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
void CH264VideoCodec::setupForDecode(const I<CMediaReader>& mediaReader, const I<CCodec::DecodeInfo>& decodeInfo,
		CVideoFrame::Compatibility compatibility)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	DecodeInfo&	h264DecodeInfo = *((DecodeInfo*) &*decodeInfo);

	mInternals->mPacketMediaReader = OR<CPacketMediaReader>(*((CPacketMediaReader*) &*mediaReader));
	mInternals->mTimeScale = h264DecodeInfo.getTimeScale();

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
			CVideoFrame((UniversalTimeInterval) sampleTime / 10000.0, mInternals->mOutputSampleFrameSize,
					mInternals->mOutputSampleDataFormatGUID, *sample.getInstance()));




//	// Process input
//	result = mInternals->mVideoDecoder->ProcessInput(0, *inputSample, 0);
//	LogHRESULTIfFailed(result, "ProcessInput");
//
//	//// Get output status
//	//DWORD	status = 0;
//	//result = mInternals->mVideoDecoder->GetOutputStatus(&status);
//	//LogHRESULTIfFailed(result, "GetOutputStatus");
//
//	// Get output stream info
//	MFT_OUTPUT_STREAM_INFO	outputStreamInfo;
//	result = mInternals->mVideoDecoder->GetOutputStreamInfo(0, &outputStreamInfo);
//
//	// Process output
//	OCI<IMFSample>	outputSample;
//	outputSample = CMediaFoundationServices::createSample(outputStreamInfo.cbSize);
//	//if (!outputSample.hasInstance())
//	//	return;
//
//	MFT_OUTPUT_DATA_BUFFER	outputDataBuffer = {0, *outputSample, 0, NULL};
//	DWORD					status = 0;
//	result = mInternals->mVideoDecoder->ProcessOutput(0, 1, &outputDataBuffer, &status);
//
//
//
//
////else if (sliceType == 1)
////	sampleTimingInfo.presentationTimeStamp = ::CMTimeMake(mInternals->mNextFrameTime - packetAndLocation.mPacket.mDuration, mInternals->mTimeScale);
////else
////	sampleTimingInfo.presentationTimeStamp = ::CMTimeMake(mInternals->mNextFrameTime + packetAndLocation.mPacket.mDuration, mInternals->mTimeScale);
//
//	//sampleTimingInfo.decodeTimeStamp = ::CMTimeMake(mInternals->mNextFrameTime, mInternals->mTimeScale);
//
//	//size_t				sampleSize = data->getSize();
//	//CMSampleBufferRef	sampleBufferRef;
//	//status =
//	//		::CMSampleBufferCreate(kCFAllocatorDefault, blockBufferRef, true, nil, nil,
//	//				mInternals->mFormatDescriptionRef, 1, 1, &sampleTimingInfo, 1, &sampleSize, &sampleBufferRef);
//	//::CFRelease(blockBufferRef);
//	//LogOSStatusIfFailedAndReturnValue(status, OSSTR("CMSampleBufferCreate"), false);
//
//	//// Decode frame
//	//VTDecodeInfoFlags	decodeInfoFlags;
//	//::VTDecompressionSessionDecodeFrame(mInternals->mDecompressionSessionRef, sampleBufferRef,
//	//		kVTDecodeFrame_EnableAsynchronousDecompression | kVTDecodeFrame_EnableTemporalProcessing,
//	//		nil, &decodeInfoFlags);
//	//::CFRelease(sampleBufferRef);
//
//	// Update
//	//mInternals->mNextFrameIndex++;
//	mInternals->mNextFrameTime += duration;
//
//	//return true;
//while (true) ;
}

//----------------------------------------------------------------------------------------------------------------------
void CH264VideoCodec::decodeReset()
//----------------------------------------------------------------------------------------------------------------------
{
	//// Reset
	//if (mInternals->mDecompressionSessionRef != nil) {
	//	// Invalidate and release
	//	::VTDecompressionSessionWaitForAsynchronousFrames(mInternals->mDecompressionSessionRef);
	//}
	//mInternals->mNextFrameIndex = 0;
}



#if false
/**
* Creates a new single buffer media sample.
* @param[in] bufferSize: size of the media buffer to set on the create media sample.
* @param[out] pSample: pointer to the create single buffer media sample.
* @@Returns S_OK if successful or an error code if not.
*/
HRESULT CreateSingleBufferIMFSample(DWORD bufferSize, IMFSample** pSample)
{
  IMFMediaBuffer* pBuffer = NULL;

  HRESULT hr = S_OK;

  hr = MFCreateSample(pSample);
  CHECK_HR(hr, "Failed to create MF sample.");

  // Adds a ref count to the pBuffer object.
  hr = MFCreateMemoryBuffer(bufferSize, &pBuffer);
  CHECK_HR(hr, "Failed to create memory buffer.");

  // Adds another ref count to the pBuffer object.
  hr = (*pSample)->AddBuffer(pBuffer);
  CHECK_HR(hr, "Failed to add sample to buffer.");

done:
  // Leave the single ref count that will be removed when the pSample is released.
  SAFE_RELEASE(pBuffer);
  return hr;
}

/**
* Attempts to get an output sample from an MFT transform.
* @param[in] pTransform: pointer to the media transform to apply.
* @param[out] pOutSample: pointer to the media sample output by the transform. Can be NULL
*  if the transform did not produce one.
* @param[out] transformFlushed: if set to true means the transform format changed and the
*  contents were flushed. Output format of sample most likely changed.
* @@Returns S_OK if successful or an error code if not.
*/
HRESULT GetTransformOutput(IMFTransform* pTransform, IMFSample** pOutSample, BOOL* transformFlushed)
{
  MFT_OUTPUT_STREAM_INFO StreamInfo = { 0 };
  MFT_OUTPUT_DATA_BUFFER outputDataBuffer = { 0 };
  DWORD processOutputStatus = 0;
  IMFMediaType* pChangedOutMediaType = NULL;

  HRESULT hr = S_OK;
  *transformFlushed = FALSE;

  hr = pTransform->GetOutputStreamInfo(0, &StreamInfo);
  CHECK_HR(hr, "Failed to get output stream info from MFT.");

  outputDataBuffer.dwStreamID = 0;
  outputDataBuffer.dwStatus = 0;
  outputDataBuffer.pEvents = NULL;

  if ((StreamInfo.dwFlags & MFT_OUTPUT_STREAM_PROVIDES_SAMPLES) == 0) {
    hr = CreateSingleBufferIMFSample(StreamInfo.cbSize, pOutSample);
    CHECK_HR(hr, "Failed to create new single buffer IMF sample.");
    outputDataBuffer.pSample = *pOutSample;
  }

  auto mftProcessOutput = pTransform->ProcessOutput(0, 1, &outputDataBuffer, &processOutputStatus);

  //printf("Process output result %.2X, MFT status %.2X.\n", mftProcessOutput, processOutputStatus);

  if (mftProcessOutput == S_OK) {
    // Sample is ready and allocated on the transform output buffer.
    *pOutSample = outputDataBuffer.pSample;
  }
  else if (mftProcessOutput == MF_E_TRANSFORM_STREAM_CHANGE) {
    // Format of the input stream has changed. https://docs.microsoft.com/en-us/windows/win32/medfound/handling-stream-changes
    if (outputDataBuffer.dwStatus == MFT_OUTPUT_DATA_BUFFER_FORMAT_CHANGE) {
      CLogServices::logMessage(CString(OSSTR("MFT stream changed.")));

      hr = pTransform->GetOutputAvailableType(0, 0, &pChangedOutMediaType);
      CHECK_HR(hr, "Failed to get the MFT output media type after a stream change.");

      //std::cout << "MFT output media type: " << GetMediaTypeDescription(pChangedOutMediaType) << std::endl << std::endl;
		//CLogServices::logMessage(CString(OSSTR("MFT format change...")));
		LogMediaType(pChangedOutMediaType);

      hr = pChangedOutMediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_IYUV);
      CHECK_HR(hr, "Failed to set media sub type.");

      hr = pTransform->SetOutputType(0, pChangedOutMediaType, 0);
      CHECK_HR(hr, "Failed to set new output media type on MFT.");

      hr = pTransform->ProcessMessage(MFT_MESSAGE_COMMAND_FLUSH, NULL);
      CHECK_HR(hr, "Failed to process FLUSH command on MFT.");

      *transformFlushed = TRUE;
    }
    else {
      CLogServices::logMessage(CString(OSSTR("MFT stream changed but didn't have the data format change flag set. Don't know what to do.")));
      hr = E_NOTIMPL;
    }

    SAFE_RELEASE(pOutSample);
    *pOutSample = NULL;
  }
  else if (mftProcessOutput == MF_E_TRANSFORM_NEED_MORE_INPUT) {
    // More input is not an error condition but it means the allocated output sample is empty.
    SAFE_RELEASE(pOutSample);
    *pOutSample = NULL;
    hr = MF_E_TRANSFORM_NEED_MORE_INPUT;
  }
  else {
    printf("MFT ProcessOutput error result %.2X, MFT status %.2X.\n", mftProcessOutput, processOutputStatus);
    hr = mftProcessOutput;
    SAFE_RELEASE(pOutSample);
    *pOutSample = NULL;
  }

done:

  SAFE_RELEASE(pChangedOutMediaType);

  return hr;
}
#endif
