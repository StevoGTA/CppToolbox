//----------------------------------------------------------------------------------------------------------------------
//	CH264VideoCodec-Apple.cpp			©2021 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CH264VideoCodec.h"

#include "CBitReader.h"
#include "CLogServices-Apple.h"

#include <CoreMedia/CoreMedia.h>
#include <VideoToolbox/VideoToolbox.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CH264VideoCodecInternals

class CH264VideoCodecInternals {
	public:
						CH264VideoCodecInternals() :
							mTimeScale(0),
									mFormatDescriptionRef(nil), mDecompressionSessionRef(nil),
									mCurrentFrameNumberBitCount(0), mCurrentPicOrderCountLSBBitCount(0),
									mPicOrderCountMSBChangeThreshold(0),
									mPicOrderCountMSB(0), mPreviousPicOrderCountLSB(0), mLastIDRFrameTime(0),
									mNextFrameTime(0)
							{}
						~CH264VideoCodecInternals()
							{
								// Cleanup
								if (mFormatDescriptionRef != nil)
									// Release
									::CFRelease(mFormatDescriptionRef);

								if (mDecompressionSessionRef != nil) {
									// Invalidate and release
									::VTDecompressionSessionWaitForAsynchronousFrames(mDecompressionSessionRef);
									::VTDecompressionSessionInvalidate(mDecompressionSessionRef);
									::CFRelease(mDecompressionSessionRef);
								}
							}

		static	void	decompressionOutputCallback(void* decompressionOutputUserData, void* sourceFrameUserData,
								OSStatus status, VTDecodeInfoFlags decodeInfoFlags, CVImageBufferRef imageBufferRef,
								CMTime presentationTime, CMTime presentationDuration)
							{
								*((CVImageBufferRef*) sourceFrameUserData) = (CVBufferRef) ::CFRetain(imageBufferRef);
							}

		OR<CPacketMediaReader>		mPacketMediaReader;
		UInt32						mTimeScale;

		CMFormatDescriptionRef		mFormatDescriptionRef;
		VTDecompressionSessionRef	mDecompressionSessionRef;
		UInt8						mCurrentFrameNumberBitCount;
		UInt8						mCurrentPicOrderCountLSBBitCount;
		UInt8						mPicOrderCountMSBChangeThreshold;

		UInt64						mPicOrderCountMSB;
		UInt64						mPreviousPicOrderCountLSB;
		UInt64						mLastIDRFrameTime;
		UInt64						mNextFrameTime;
};

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
	const	DecodeInfo&								h264DecodeInfo = *((DecodeInfo*) &*decodeInfo);
			CH264VideoCodec::DecodeInfo::SPSPPSInfo	spsppsInfo = h264DecodeInfo.getSPSPPSInfo();
	const	TArray<CH264VideoCodec::NALUInfo>		spsNALUInfos = spsppsInfo.getSPSNALUInfos();
	const	TArray<CH264VideoCodec::NALUInfo>		ppsNALUInfos = spsppsInfo.getPPSNALUInfos();
			OSStatus								status;

	// Setup format description
			CArray::ItemCount	spsCount = spsNALUInfos.getCount();
			CArray::ItemCount	ppsCount = ppsNALUInfos.getCount();
	const	uint8_t*			parameterSetPointers[spsCount + ppsCount];
			size_t				parameterSetSizes[spsCount + ppsCount];
	for (CArray::ItemIndex i = 0; i < spsCount; i++) {
		// Store
		parameterSetPointers[i] = spsNALUInfos[i].getBytePtr();
		parameterSetSizes[i] = spsNALUInfos[i].getSize();
	}
	for (CArray::ItemIndex i = 0; i < ppsCount; i++) {
		// Store
		parameterSetPointers[spsCount + i] = ppsNALUInfos[i].getBytePtr();
		parameterSetSizes[spsCount + i] = ppsNALUInfos[i].getSize();
	}

	status =
			::CMVideoFormatDescriptionCreateFromH264ParameterSets(kCFAllocatorDefault, spsCount + ppsCount,
					parameterSetPointers, parameterSetSizes, h264DecodeInfo.getNALUHeaderLengthSize(),
					&mInternals->mFormatDescriptionRef);
	LogOSStatusIfFailed(status, OSSTR("CMVideoFormatDescriptionCreateFromH264ParameterSets"));

	// Setup Decompression Session
	VTDecompressionOutputCallbackRecord	decompressionOutputCallbackRecord;
	decompressionOutputCallbackRecord.decompressionOutputCallback =
			CH264VideoCodecInternals::decompressionOutputCallback;
	decompressionOutputCallbackRecord.decompressionOutputRefCon = mInternals;

	CFMutableDictionaryRef	destinationImageBufferAttributes =
									::CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,
											&kCFTypeDictionaryValueCallBacks);

	switch (videoProcessingFormat.getCompatibility()) {
		case CVideoFrame::kCompatibilityCGImage:
			// CGImage
			::CFDictionarySetValue(destinationImageBufferAttributes,
					kCVPixelBufferCGImageCompatibilityKey, kCFBooleanTrue);
			break;

		case CVideoFrame::kCompatibilityMetal:
			// Metal
			::CFDictionarySetValue(destinationImageBufferAttributes,
					kCVPixelBufferMetalCompatibilityKey, kCFBooleanTrue);
			break;

#if TARGET_OS_IOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
		case CVideoFrame::kCompatibilityOpenGLES:
			// OpenGLES
			::CFDictionarySetValue(destinationImageBufferAttributes,
					kCVPixelBufferOpenGLESCompatibilityKey, kCFBooleanTrue);
			break;

#else
		case CVideoFrame::kCompatibilityOpenGL: {
			// OpenGL
			::CFDictionarySetValue(destinationImageBufferAttributes,
					kCVPixelBufferOpenGLCompatibilityKey, kCFBooleanTrue);
			::CFDictionarySetValue(destinationImageBufferAttributes,
					kCVPixelBufferOpenGLTextureCacheCompatibilityKey, kCFBooleanTrue);

			OSType	pixelFormat = kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;
			::CFDictionarySetValue(destinationImageBufferAttributes,
					kCVPixelBufferPixelFormatTypeKey,
					::CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &pixelFormat));
			} break;
#endif
	}

	CFMutableDictionaryRef	videoDecoderSpecification =
									::CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
											&kCFTypeDictionaryKeyCallBacks,
											&kCFTypeDictionaryValueCallBacks);
	::CFDictionarySetValue(videoDecoderSpecification, kVTDecompressionPropertyKey_RealTime,
			kCFBooleanTrue);

	status =
			::VTDecompressionSessionCreate(kCFAllocatorDefault, mInternals->mFormatDescriptionRef,
					videoDecoderSpecification, destinationImageBufferAttributes,
					&decompressionOutputCallbackRecord, &mInternals->mDecompressionSessionRef);
	::CFRelease(destinationImageBufferAttributes);
	::CFRelease(videoDecoderSpecification);
	LogOSStatusIfFailed(status, OSSTR("VTDecompressionSessionCreate"));

	// Finish setup
	mInternals->mPacketMediaReader = OR<CPacketMediaReader>(*((CPacketMediaReader*) &*mediaReader));
	mInternals->mTimeScale = h264DecodeInfo.getTimeScale();

	CH264VideoCodec::SequenceParameterSetPayload	spsPayload(
															CData(parameterSetPointers[0],
																	(CData::Size)
																			parameterSetSizes[0],
																	false));
	mInternals->mCurrentFrameNumberBitCount = spsPayload.mFrameNumberBitCount;
	mInternals->mCurrentPicOrderCountLSBBitCount = spsPayload.mPicOrderCountLSBBitCount;
	mInternals->mPicOrderCountMSBChangeThreshold = 1 << (mInternals->mCurrentPicOrderCountLSBBitCount - 1);
}

//----------------------------------------------------------------------------------------------------------------------
TIResult<CVideoFrame> CH264VideoCodec::decode()
//----------------------------------------------------------------------------------------------------------------------
{
	// Read next packet
	TIResult<CPacketMediaReader::MediaPacketDataInfo>	mediaPacketDataInfo =
																mInternals->mPacketMediaReader->
																		readNextMediaPacketDataInfo();
	ReturnValueIfResultError(mediaPacketDataInfo, TIResult<CVideoFrame>(mediaPacketDataInfo.getError()));

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
	OV<UInt32>	size = bitReader.readUInt32().getValue();
	UInt64		pos = bitReader.getPos();

	OV<UInt8>	forbidden_zero_bit = bitReader.readUInt8(1).getValue();
	OV<UInt8>	nal_ref_idc = bitReader.readUInt8(2).getValue();
	OV<UInt8>	nal_unit_type = bitReader.readUInt8(5).getValue();
	NALUInfo::Type	naluType = (NALUInfo::Type) *nal_unit_type;

	if (naluType == NALUInfo::kTypeCodedSliceNonIDRPicture) {
		// Coded Slice Non-IDR Picture
		OV<UInt32>	first_mb_in_slice = bitReader.readUEColumbusCode().getValue();
		OV<UInt32>	slice_type = bitReader.readUEColumbusCode().getValue();
		OV<UInt32>	pic_parameter_set_id = bitReader.readUEColumbusCode().getValue();
		OV<UInt8>	frame_num = bitReader.readUInt8(mInternals->mCurrentFrameNumberBitCount).getValue();
		OV<UInt8>	pic_order_cnt_lsb = bitReader.readUInt8(mInternals->mCurrentPicOrderCountLSBBitCount).getValue();
		OV<UInt32>	delta_pic_order_cnt_bottom = bitReader.readUEColumbusCode().getValue();

		naluUnitType = *nal_unit_type;
		sliceType = *slice_type;
		frameNum = *frame_num;
		picOrderCntLSB = *pic_order_cnt_lsb;
		deltaPicOrderCntBottom = *delta_pic_order_cnt_bottom;
		break;
	} else if (naluType == NALUInfo::kTypeCodedSliceIDRPicture) {
		// Coded Slice IDR Picture
		OV<UInt32>	first_mb_in_slice = bitReader.readUEColumbusCode().getValue();
		OV<UInt32>	slice_type = bitReader.readUEColumbusCode().getValue();
		OV<UInt32>	pic_parameter_set_id = bitReader.readUEColumbusCode().getValue();
		OV<UInt8>	frame_num = bitReader.readUInt8(mInternals->mCurrentFrameNumberBitCount).getValue();
		OV<UInt32>	idr_pic_id = bitReader.readUEColumbusCode().getValue();
		OV<UInt8>	pic_order_cnt_lsb = bitReader.readUInt8(mInternals->mCurrentPicOrderCountLSBBitCount).getValue();
		OV<UInt32>	delta_pic_order_cnt_bottom = bitReader.readUEColumbusCode().getValue();

		naluUnitType = *nal_unit_type;
		sliceType = *slice_type;
		frameNum = *frame_num;
		picOrderCntLSB = *pic_order_cnt_lsb;
		deltaPicOrderCntBottom = *delta_pic_order_cnt_bottom;
		break;
	} else if (naluType == NALUInfo::kTypeSupplementalEnhancementInformation) {
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

	// Setup sample buffer
	CMBlockBufferRef	blockBufferRef;
	OSStatus	status =
						::CMBlockBufferCreateWithMemoryBlock(kCFAllocatorDefault, (void*) data.getBytePtr(),
								data.getSize(), kCFAllocatorNull, nil, 0, data.getSize(), 0, &blockBufferRef);
	LogOSStatusIfFailedAndReturnValue(status, OSSTR("CMBlockBufferCreateWithMemoryBlock"),
			TIResult<CVideoFrame>(SErrorFromOSStatus(status)));

	CMSampleTimingInfo	sampleTimingInfo;
	sampleTimingInfo.duration = ::CMTimeMake(duration, mInternals->mTimeScale);

UInt64	frameTime;
if (sliceType == 2) {
	// IDR
	frameTime = mInternals->mNextFrameTime;
	mInternals->mPicOrderCountMSB = 0;
	mInternals->mPreviousPicOrderCountLSB = 0;
	mInternals->mLastIDRFrameTime = mInternals->mNextFrameTime;
} else {
	// Non-IDR
	if ((picOrderCntLSB > mInternals->mPreviousPicOrderCountLSB) &&
			((picOrderCntLSB - mInternals->mPreviousPicOrderCountLSB) > mInternals->mPicOrderCountMSBChangeThreshold))
		//
		mInternals->mPicOrderCountMSB -= 1 << mInternals->mCurrentPicOrderCountLSBBitCount;
	else if ((mInternals->mPreviousPicOrderCountLSB > picOrderCntLSB) &&
			((mInternals->mPreviousPicOrderCountLSB - picOrderCntLSB) > mInternals->mPicOrderCountMSBChangeThreshold))
		//
		mInternals->mPicOrderCountMSB += 1 << mInternals->mCurrentPicOrderCountLSBBitCount;

	frameTime =
			mInternals->mLastIDRFrameTime +
					(mInternals->mPicOrderCountMSB + picOrderCntLSB) / 2 * duration;

	mInternals->mPreviousPicOrderCountLSB = picOrderCntLSB;
}

sampleTimingInfo.presentationTimeStamp = ::CMTimeMake(frameTime, mInternals->mTimeScale);



//else if (sliceType == 1)
//	sampleTimingInfo.presentationTimeStamp = ::CMTimeMake(mInternals->mNextFrameTime - packetAndLocation.mPacket.mDuration, mInternals->mTimeScale);
//else
//	sampleTimingInfo.presentationTimeStamp = ::CMTimeMake(mInternals->mNextFrameTime + packetAndLocation.mPacket.mDuration, mInternals->mTimeScale);

	sampleTimingInfo.decodeTimeStamp = ::CMTimeMake(mInternals->mNextFrameTime, mInternals->mTimeScale);

	size_t				sampleSize = data.getSize();
	CMSampleBufferRef	sampleBufferRef;
	status =
			::CMSampleBufferCreate(kCFAllocatorDefault, blockBufferRef, true, nil, nil,
					mInternals->mFormatDescriptionRef, 1, 1, &sampleTimingInfo, 1, &sampleSize, &sampleBufferRef);
	::CFRelease(blockBufferRef);
	LogOSStatusIfFailedAndReturnValue(status, OSSTR("CMSampleBufferCreate"),
			TIResult<CVideoFrame>(SErrorFromOSStatus(status)));

	// Decode frame
	CVImageBufferRef	imageBufferRef;
	VTDecodeInfoFlags	decodeInfoFlags;
	status =
			::VTDecompressionSessionDecodeFrame(mInternals->mDecompressionSessionRef, sampleBufferRef, 0,
					&imageBufferRef, &decodeInfoFlags);
	::CFRelease(sampleBufferRef);
	LogOSStatusIfFailedAndReturnValue(status, OSSTR("VTDecompressionSessionDecodeFrame"),
			TIResult<CVideoFrame>(SErrorFromOSStatus(status)));

	// Update
	mInternals->mNextFrameTime += duration;

	// Prepare return info
	TIResult<CVideoFrame>	result(
									CVideoFrame(::CMTimeGetSeconds(sampleTimingInfo.presentationTimeStamp),
											imageBufferRef));
	::CFRelease(imageBufferRef);

	return result;
}

//----------------------------------------------------------------------------------------------------------------------
void CH264VideoCodec::decodeReset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Reset
	if (mInternals->mDecompressionSessionRef != nil) {
		// Invalidate and release
		::VTDecompressionSessionWaitForAsynchronousFrames(mInternals->mDecompressionSessionRef);
	}
	if (mInternals->mPacketMediaReader.hasReference())
		// Back to the begining
		mInternals->mPacketMediaReader->set(0);
}
