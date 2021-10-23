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
						CH264VideoCodecInternals() : mFormatDescriptionRef(nil), mDecompressionSessionRef(nil) {}
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
							{ *((CVImageBufferRef*) sourceFrameUserData) = (CVBufferRef) ::CFRetain(imageBufferRef); }

		OV<UInt32>							mTimeScale;
		OI<SVideoProcessingFormat>			mVideoProcessingFormat;
		OI<I<CCodec::DecodeInfo> >			mDecodeInfo;

		CMFormatDescriptionRef				mFormatDescriptionRef;
		VTDecompressionSessionRef			mDecompressionSessionRef;
		OI<CH264VideoCodec::FrameTiming>	mFrameTiming;
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
void CH264VideoCodec::setupForDecode(const SVideoProcessingFormat& videoProcessingFormat,
		const I<CCodec::DecodeInfo>& decodeInfo)
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
		parameterSetSizes[i] = spsNALUInfos[i].getByteCount();
	}
	for (CArray::ItemIndex i = 0; i < ppsCount; i++) {
		// Store
		parameterSetPointers[spsCount + i] = ppsNALUInfos[i].getBytePtr();
		parameterSetSizes[spsCount + i] = ppsNALUInfos[i].getByteCount();
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

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS)
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

			OSType		pixelFormat = kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange;
			CFNumberRef	numberRef = ::CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &pixelFormat);
			::CFDictionarySetValue(destinationImageBufferAttributes,
					kCVPixelBufferPixelFormatTypeKey, numberRef);
			::CFRelease(numberRef);
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
	mInternals->mTimeScale = OV<UInt32>(h264DecodeInfo.getTimeScale());
	mInternals->mVideoProcessingFormat = OI<SVideoProcessingFormat>(videoProcessingFormat);
	mInternals->mDecodeInfo = OI<I<CCodec::DecodeInfo> >(decodeInfo);

	SequenceParameterSetPayload	spsPayload(
										CData(parameterSetPointers[0], (CData::ByteCount) parameterSetSizes[0], false));
	mInternals->mFrameTiming = OI<CH264VideoCodec::FrameTiming>(new CH264VideoCodec::FrameTiming(spsPayload));
}

//----------------------------------------------------------------------------------------------------------------------
void CH264VideoCodec::seek(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	DecodeInfo&	h264DecodeInfo = *((DecodeInfo*) &**mInternals->mDecodeInfo);

	// Reset
	::VTDecompressionSessionWaitForAsynchronousFrames(mInternals->mDecompressionSessionRef);

	// Seek
	UInt32	frameIndex =
					h264DecodeInfo.getMediaPacketSource()->seekToKeyframe(
							(UInt32) (timeInterval * mInternals->mVideoProcessingFormat->getFramerate() + 0.5),
							h264DecodeInfo.getKeyframeIndexes());
	mInternals->mFrameTiming->seek(
			(UInt64) ((UniversalTimeInterval) frameIndex / mInternals->mVideoProcessingFormat->getFramerate() *
					(UniversalTimeInterval) *mInternals->mTimeScale));
}

//----------------------------------------------------------------------------------------------------------------------
TIResult<CVideoFrame> CH264VideoCodec::decode()
//----------------------------------------------------------------------------------------------------------------------
{
	// Get next packet
	TIResult<CMediaPacketSource::DataInfo>	dataInfo = (*mInternals->mDecodeInfo)->getMediaPacketSource()->readNext();
	ReturnValueIfResultError(dataInfo, TIResult<CVideoFrame>(dataInfo.getError()));

	// Update frame timing
	TIResult<CH264VideoCodec::FrameTiming::Times>	times = mInternals->mFrameTiming->updateFrom(dataInfo.getValue());
	ReturnValueIfResultError(dataInfo, TIResult<CVideoFrame>(times.getError()));

	// Setup sample buffer
	const	CData&				data = dataInfo.getValue().getData();
			CMBlockBufferRef	blockBufferRef;
			OSStatus			status =
										::CMBlockBufferCreateWithMemoryBlock(kCFAllocatorDefault,
												(void*) data.getBytePtr(), data.getByteCount(), kCFAllocatorNull, nil,
												0, data.getByteCount(), 0, &blockBufferRef);
	LogOSStatusIfFailedAndReturnValue(status, OSSTR("CMBlockBufferCreateWithMemoryBlock"),
			TIResult<CVideoFrame>(SErrorFromOSStatus(status)));

	CMSampleTimingInfo	sampleTimingInfo;
	sampleTimingInfo.duration = ::CMTimeMake(dataInfo.getValue().getDuration(), *mInternals->mTimeScale);
	sampleTimingInfo.decodeTimeStamp = ::CMTimeMake(times.getValue().mDecodeTime, *mInternals->mTimeScale);
	sampleTimingInfo.presentationTimeStamp = ::CMTimeMake(times.getValue().mPresentationTime, *mInternals->mTimeScale);

	size_t				sampleSize = data.getByteCount();
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

	// Prepare return info
	TIResult<CVideoFrame>	result(
									CVideoFrame(::CMTimeGetSeconds(sampleTimingInfo.presentationTimeStamp),
											imageBufferRef));
	::CFRelease(imageBufferRef);

	return result;
}
