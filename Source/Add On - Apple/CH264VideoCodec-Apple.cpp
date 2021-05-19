//----------------------------------------------------------------------------------------------------------------------
//	CH264VideoCodec-Apple.cpp			©2021 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CH264VideoCodec.h"

#include "CBitParceller.h"
#include "CLogServices-Apple.h"

#include <CoreMedia/CoreMedia.h>
#include <VideoToolbox/VideoToolbox.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CH264VideoCodecInternals

class CH264VideoCodecInternals {
	public:
						CH264VideoCodecInternals(const I<CDataSource>& dataSource,
								const CH264VideoCodec::DecodeInfo& decodeInfo,
								const CVideoCodec::DecodeFrameInfo& decodeFrameInfo) :
							mBitParceller(dataSource, true), mTimeScale(decodeInfo.getTimeScale()),
									mPacketAndLocations(decodeInfo.getPacketAndLocations()),
									mDecodeFrameInfo(decodeFrameInfo),
									mFormatDescriptionRef(nil), mDecompressionSessionRef(nil), mNextFrameIndex(0),
									mNextFrameTime(0)
							{
								// Setup
								const	CData&							configurationData =
																				decodeInfo.getConfigurationData();
								const	CH264VideoCodec::Configuration&	configuration =
																				*((CH264VideoCodec::Configuration*)
																						configurationData.getBytePtr());
										OSStatus						status;

								// Setup format description
										UInt32		spsCount = configuration.getSPSCount();
										UInt32		ppsCount = configuration.getPPSCount();
								const	uint8_t*	parameterSetPointers[spsCount + ppsCount];
										size_t		parameterSetSizes[spsCount + ppsCount];
								for (UInt32 i = 0; i < spsCount; i++) {
									// Store
									parameterSetPointers[i] = configuration.getSPSPayload(i);
									parameterSetSizes[i] = configuration.getSPSSize(i);
								}
								for (UInt32 i = 0; i < ppsCount; i++) {
									// Store
									parameterSetPointers[spsCount + i] = configuration.getPPSPayload(i);
									parameterSetSizes[spsCount + i] = configuration.getPPSSize(i);
								}

								status =
										::CMVideoFormatDescriptionCreateFromH264ParameterSets(kCFAllocatorDefault,
												spsCount + ppsCount, parameterSetPointers, parameterSetSizes,
												configuration.getNALUHeaderLengthSize(), &mFormatDescriptionRef);
								LogOSStatusIfFailed(status,
										OSSTR("CMVideoFormatDescriptionCreateFromH264ParameterSets"));

								// Setup Decompression Session
								VTDecompressionOutputCallbackRecord	decompressionOutputCallbackRecord;
								decompressionOutputCallbackRecord.decompressionOutputCallback =
										CH264VideoCodecInternals::decompressionOutputCallback;
								decompressionOutputCallbackRecord.decompressionOutputRefCon = this;

								CFMutableDictionaryRef	destinationImageBufferAttributes =
																::CFDictionaryCreateMutable(kCFAllocatorDefault, 0,
																		&kCFTypeDictionaryKeyCallBacks,
																		&kCFTypeDictionaryValueCallBacks);

								switch (mDecodeFrameInfo.getCompatibility()) {
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
										::VTDecompressionSessionCreate(kCFAllocatorDefault, mFormatDescriptionRef,
												videoDecoderSpecification, destinationImageBufferAttributes,
												&decompressionOutputCallbackRecord, &mDecompressionSessionRef);
								::CFRelease(destinationImageBufferAttributes);
								::CFRelease(videoDecoderSpecification);
								LogOSStatusIfFailed(status, OSSTR("VTDecompressionSessionCreate"));

								// Finish setup
								CH264VideoCodec::SequenceParameterSetPayload	spsPayload(
																						CData(parameterSetPointers[0],
																								(CData::Size)
																										parameterSetSizes[0],
																								false));
								mCurrentFrameNumberBitCount = spsPayload.mFrameNumberBitCount;
								mCurrentPicOrderCountLSBBitCount = spsPayload.mPicOrderCountLSBBitCount;
								mPicOrderCountMSBChangeThreshold = 1 << (mCurrentPicOrderCountLSBBitCount - 1);
							}
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
									internals.mDecodeFrameInfo.frameReady(
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
									internals.mDecodeFrameInfo.error(error);
								}
							}

		CBitParceller						mBitParceller;
		UInt32								mTimeScale;
		TArray<CCodec::PacketAndLocation>	mPacketAndLocations;
		CVideoCodec::DecodeFrameInfo		mDecodeFrameInfo;

		CMFormatDescriptionRef				mFormatDescriptionRef;
		VTDecompressionSessionRef			mDecompressionSessionRef;
		UInt8								mCurrentFrameNumberBitCount;
		UInt8								mCurrentPicOrderCountLSBBitCount;
		UInt8								mPicOrderCountMSBChangeThreshold;

		UInt32								mNextFrameIndex;
		UInt64								mNextFrameTime;

		UInt64								mPicOrderCountMSB;
		UInt64								mPreviousPicOrderCountLSB;
		UInt64								mLastIDRFrameTime;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CH264VideoCodec

//----------------------------------------------------------------------------------------------------------------------
CH264VideoCodec::CH264VideoCodec(const I<CDataSource>& dataSource, const I<CCodec::DecodeInfo>& decodeInfo,
		const DecodeFrameInfo& decodeFrameInfo) : CDecodeOnlyVideoCodec()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CH264VideoCodecInternals(dataSource, *((DecodeInfo*) &*decodeInfo), decodeFrameInfo);
}

//----------------------------------------------------------------------------------------------------------------------
CH264VideoCodec::~CH264VideoCodec()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CVideoCodec methods - Decoding

//----------------------------------------------------------------------------------------------------------------------
bool CH264VideoCodec::triggerDecode()
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	if (mInternals->mNextFrameIndex >= mInternals->mPacketAndLocations.getCount())
		return false;

	// Setup
	CCodec::PacketAndLocation&	packetAndLocation = mInternals->mPacketAndLocations.getAt(mInternals->mNextFrameIndex);
	OSStatus					status;

	// Read packet
	OI<SError>	error = mInternals->mBitParceller.setPos(CDataSource::kPositionFromBeginning, packetAndLocation.mPos);
	LogIfErrorAndReturnValue(error, OSSTR("setting position for video frame packet"), false);

	OI<CData>	data = mInternals->mBitParceller.readData(packetAndLocation.mPacket.mByteCount, error);
	LogIfErrorAndReturnValue(error, OSSTR("reading video frame packet data"), false);

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

error = mInternals->mBitParceller.setPos(CDataSource::kPositionFromBeginning, packetAndLocation.mPos);
while (true) {
	//
	OV<UInt32>	size = mInternals->mBitParceller.readUInt32(error);
	SInt64		pos = mInternals->mBitParceller.getPos();

	OV<UInt8>	forbidden_zero_bit = mInternals->mBitParceller.readUInt8(1, error);
	OV<UInt8>	nal_ref_idc = mInternals->mBitParceller.readUInt8(2, error);
	OV<UInt8>	nal_unit_type = mInternals->mBitParceller.readUInt8(5, error);
	NALUInfo::Type	naluType = (NALUInfo::Type) *nal_unit_type;

	if (naluType == NALUInfo::kTypeCodedSliceNonIDRPicture) {
		// Coded Slice Non-IDR Picture
		OV<UInt32>	first_mb_in_slice = mInternals->mBitParceller.readUEColumbusCode(error);
		OV<UInt32>	slice_type = mInternals->mBitParceller.readUEColumbusCode(error);
		OV<UInt32>	pic_parameter_set_id = mInternals->mBitParceller.readUEColumbusCode(error);
		OV<UInt8>	frame_num = mInternals->mBitParceller.readUInt8(mInternals->mCurrentFrameNumberBitCount, error);
		OV<UInt8>	pic_order_cnt_lsb = mInternals->mBitParceller.readUInt8(mInternals->mCurrentPicOrderCountLSBBitCount, error);
		OV<UInt32>	delta_pic_order_cnt_bottom = mInternals->mBitParceller.readUEColumbusCode(error);

		naluUnitType = *nal_unit_type;
		sliceType = *slice_type;
		frameNum = *frame_num;
		picOrderCntLSB = *pic_order_cnt_lsb;
		deltaPicOrderCntBottom = *delta_pic_order_cnt_bottom;
		break;
	} else if (naluType == NALUInfo::kTypeCodedSliceIDRPicture) {
		// Coded Slice IDR Picture
		OV<UInt32>	first_mb_in_slice = mInternals->mBitParceller.readUEColumbusCode(error);
		OV<UInt32>	slice_type = mInternals->mBitParceller.readUEColumbusCode(error);
		OV<UInt32>	pic_parameter_set_id = mInternals->mBitParceller.readUEColumbusCode(error);
		OV<UInt8>	frame_num = mInternals->mBitParceller.readUInt8(mInternals->mCurrentFrameNumberBitCount, error);
		OV<UInt32>	idr_pic_id = mInternals->mBitParceller.readUEColumbusCode(error);
		OV<UInt8>	pic_order_cnt_lsb = mInternals->mBitParceller.readUInt8(mInternals->mCurrentPicOrderCountLSBBitCount, error);
		OV<UInt32>	delta_pic_order_cnt_bottom = mInternals->mBitParceller.readUEColumbusCode(error);

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
	error = mInternals->mBitParceller.setPos(CDataSource::kPositionFromBeginning, pos + *size);
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
			::CMBlockBufferCreateWithMemoryBlock(kCFAllocatorDefault, (void*) data->getBytePtr(), data->getSize(),
					kCFAllocatorNull, nil, 0, data->getSize(), 0, &blockBufferRef);
	LogOSStatusIfFailedAndReturnValue(status, OSSTR("CMBlockBufferCreateWithMemoryBlock"), false);

	CMSampleTimingInfo	sampleTimingInfo;
	sampleTimingInfo.duration = ::CMTimeMake(packetAndLocation.mPacket.mDuration, mInternals->mTimeScale);

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
					(mInternals->mPicOrderCountMSB + picOrderCntLSB) / 2 * packetAndLocation.mPacket.mDuration;

	mInternals->mPreviousPicOrderCountLSB = picOrderCntLSB;
}

sampleTimingInfo.presentationTimeStamp = ::CMTimeMake(frameTime, mInternals->mTimeScale);



//else if (sliceType == 1)
//	sampleTimingInfo.presentationTimeStamp = ::CMTimeMake(mInternals->mNextFrameTime - packetAndLocation.mPacket.mDuration, mInternals->mTimeScale);
//else
//	sampleTimingInfo.presentationTimeStamp = ::CMTimeMake(mInternals->mNextFrameTime + packetAndLocation.mPacket.mDuration, mInternals->mTimeScale);

	sampleTimingInfo.decodeTimeStamp = ::CMTimeMake(mInternals->mNextFrameTime, mInternals->mTimeScale);

	size_t				sampleSize = data->getSize();
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
	mInternals->mNextFrameTime += packetAndLocation.mPacket.mDuration;

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
