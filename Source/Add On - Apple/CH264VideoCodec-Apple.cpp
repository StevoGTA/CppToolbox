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
									mNextFrameIndex(0), mNextFrameTime(0)
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
								// Setup
								CH264VideoCodecInternals&	internals =
																	*((CH264VideoCodecInternals*)
																			decompressionOutputUserData);

//CLogServices::logMessage(CString("Presentation time: ") + CString(CMTimeGetSeconds(presentationTime)));
								// Check status
								if (status == noErr)
									// Success
									internals.mDecodeFrameInfo->frameReady(
											CVideoFrame(
													CMTIME_IS_VALID(presentationTime) ?
															CMTimeGetSeconds(presentationTime) : 0.0,
													imageBufferRef));
								else {
									// Error
									SError	error = SErrorFromOSStatus(status);
									CLogServices::logError(
											CString(OSSTR("VTDecompressionSessionDecodeFrame returned ")) +
													error.getDescription());
									internals.mDecodeFrameInfo->error(error);
								}
							}

		OI<CBitReader>							mBitReader;
		UInt32									mTimeScale;
		OI<TArray<SMediaPacketAndLocation> >	mPacketAndLocations;
		OI<CVideoCodec::DecodeFrameInfo>		mDecodeFrameInfo;

		CMFormatDescriptionRef					mFormatDescriptionRef;
		VTDecompressionSessionRef				mDecompressionSessionRef;
		UInt8									mCurrentFrameNumberBitCount;
		UInt8									mCurrentPicOrderCountLSBBitCount;
		UInt8									mPicOrderCountMSBChangeThreshold;

		UInt64									mPicOrderCountMSB;
		UInt64									mPreviousPicOrderCountLSB;
		UInt64									mLastIDRFrameTime;
		UInt32									mNextFrameIndex;
		UInt64									mNextFrameTime;
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
void CH264VideoCodec::setupForDecode(const I<CSeekableDataSource>& seekableDataSource,
		const I<CCodec::DecodeInfo>& decodeInfo, const DecodeFrameInfo& decodeFrameInfo)
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

	switch (decodeFrameInfo.getCompatibility()) {
		case CVideoCodec::DecodeFrameInfo::kCompatibilityAppleMetal:
			// Metal
			::CFDictionarySetValue(destinationImageBufferAttributes,
					kCVPixelBufferMetalCompatibilityKey, kCFBooleanTrue);
			break;

#if TARGET_OS_IOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
		case CVideoCodec::DecodeFrameInfo::kCompatibilityAppleOpenGLES:
			// OpenGLES
			::CFDictionarySetValue(destinationImageBufferAttributes,
					kCVPixelBufferOpenGLESCompatibilityKey, kCFBooleanTrue);
			break;

#else
		case CVideoCodec::DecodeFrameInfo::kCompatibilityAppleOpenGL: {
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
	mInternals->mBitReader = OI<CBitReader>(CBitReader(seekableDataSource, true));
	mInternals->mTimeScale = h264DecodeInfo.getTimeScale();
	mInternals->mPacketAndLocations = OI<TArray<SMediaPacketAndLocation> >(h264DecodeInfo.getPacketAndLocations());
	mInternals->mDecodeFrameInfo = OI<CVideoCodec::DecodeFrameInfo>(decodeFrameInfo);

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
bool CH264VideoCodec::triggerDecode()
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	if (mInternals->mNextFrameIndex >= mInternals->mPacketAndLocations->getCount())
		return false;

	// Setup
	SMediaPacketAndLocation&	mediaPacketAndLocation =
										mInternals->mPacketAndLocations->getAt(mInternals->mNextFrameIndex);
	OSStatus					status;

	// Read packet
	OI<SError>	error = mInternals->mBitReader->setPos(CBitReader::kPositionFromBeginning, mediaPacketAndLocation.mPos);
	LogIfErrorAndReturnValue(error, OSSTR("setting position for video frame packet"), false);

	TIResult<CData>	dataResult = mInternals->mBitReader->readData(mediaPacketAndLocation.mMediaPacket.mByteCount);
	LogIfResultErrorAndReturnValue(dataResult, "reading video frame packet data", false);
	const	CData&	data = dataResult.getValue();

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

error = mInternals->mBitReader->setPos(CBitReader::kPositionFromBeginning, mediaPacketAndLocation.mPos);
while (true) {
	//
	OV<UInt32>	size = mInternals->mBitReader->readUInt32().getValue();
	UInt64		pos = mInternals->mBitReader->getPos();

	OV<UInt8>	forbidden_zero_bit = mInternals->mBitReader->readUInt8(1).getValue();
	OV<UInt8>	nal_ref_idc = mInternals->mBitReader->readUInt8(2).getValue();
	OV<UInt8>	nal_unit_type = mInternals->mBitReader->readUInt8(5).getValue();
	NALUInfo::Type	naluType = (NALUInfo::Type) *nal_unit_type;

	if (naluType == NALUInfo::kTypeCodedSliceNonIDRPicture) {
		// Coded Slice Non-IDR Picture
		OV<UInt32>	first_mb_in_slice = mInternals->mBitReader->readUEColumbusCode().getValue();
		OV<UInt32>	slice_type = mInternals->mBitReader->readUEColumbusCode().getValue();
		OV<UInt32>	pic_parameter_set_id = mInternals->mBitReader->readUEColumbusCode().getValue();
		OV<UInt8>	frame_num = mInternals->mBitReader->readUInt8(mInternals->mCurrentFrameNumberBitCount).getValue();
		OV<UInt8>	pic_order_cnt_lsb = mInternals->mBitReader->readUInt8(mInternals->mCurrentPicOrderCountLSBBitCount).getValue();
		OV<UInt32>	delta_pic_order_cnt_bottom = mInternals->mBitReader->readUEColumbusCode().getValue();

		naluUnitType = *nal_unit_type;
		sliceType = *slice_type;
		frameNum = *frame_num;
		picOrderCntLSB = *pic_order_cnt_lsb;
		deltaPicOrderCntBottom = *delta_pic_order_cnt_bottom;
		break;
	} else if (naluType == NALUInfo::kTypeCodedSliceIDRPicture) {
		// Coded Slice IDR Picture
		OV<UInt32>	first_mb_in_slice = mInternals->mBitReader->readUEColumbusCode().getValue();
		OV<UInt32>	slice_type = mInternals->mBitReader->readUEColumbusCode().getValue();
		OV<UInt32>	pic_parameter_set_id = mInternals->mBitReader->readUEColumbusCode().getValue();
		OV<UInt8>	frame_num = mInternals->mBitReader->readUInt8(mInternals->mCurrentFrameNumberBitCount).getValue();
		OV<UInt32>	idr_pic_id = mInternals->mBitReader->readUEColumbusCode().getValue();
		OV<UInt8>	pic_order_cnt_lsb = mInternals->mBitReader->readUInt8(mInternals->mCurrentPicOrderCountLSBBitCount).getValue();
		OV<UInt32>	delta_pic_order_cnt_bottom = mInternals->mBitReader->readUEColumbusCode().getValue();

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
	error = mInternals->mBitReader->setPos(CBitReader::kPositionFromBeginning, pos + *size);
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
	status =
			::CMBlockBufferCreateWithMemoryBlock(kCFAllocatorDefault, (void*) data.getBytePtr(), data.getSize(),
					kCFAllocatorNull, nil, 0, data.getSize(), 0, &blockBufferRef);
	LogOSStatusIfFailedAndReturnValue(status, OSSTR("CMBlockBufferCreateWithMemoryBlock"), false);

	CMSampleTimingInfo	sampleTimingInfo;
	sampleTimingInfo.duration = ::CMTimeMake(mediaPacketAndLocation.mMediaPacket.mDuration, mInternals->mTimeScale);

//	sampleTimingInfo.presentationTimeStamp = ::CMTimeMake(mInternals->mNextFrameTime, mInternals->mTimeScale);

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
					(mInternals->mPicOrderCountMSB + picOrderCntLSB) / 2 * mediaPacketAndLocation.mMediaPacket.mDuration;

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
	LogOSStatusIfFailedAndReturnValue(status, OSSTR("CMSampleBufferCreate"), false);

	// Decode frame
	VTDecodeInfoFlags	decodeInfoFlags;
	::VTDecompressionSessionDecodeFrame(mInternals->mDecompressionSessionRef, sampleBufferRef,
			kVTDecodeFrame_EnableAsynchronousDecompression | kVTDecodeFrame_EnableTemporalProcessing,
			nil, &decodeInfoFlags);
	::CFRelease(sampleBufferRef);

	// Update
	mInternals->mNextFrameIndex++;
	mInternals->mNextFrameTime += mediaPacketAndLocation.mMediaPacket.mDuration;

	return true;
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CH264VideoCodec::set(const SMediaPosition& mediaPosition)
//----------------------------------------------------------------------------------------------------------------------
{
	return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CH264VideoCodec::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Reset
	if (mInternals->mDecompressionSessionRef != nil) {
		// Invalidate and release
		::VTDecompressionSessionWaitForAsynchronousFrames(mInternals->mDecompressionSessionRef);
	}
	mInternals->mNextFrameIndex = 0;
	
	return OI<SError>();
}
