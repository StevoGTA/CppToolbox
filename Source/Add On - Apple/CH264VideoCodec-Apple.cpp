//----------------------------------------------------------------------------------------------------------------------
//	CH264VideoCodec-Apple.cpp			©2021 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CH264VideoCodec.h"

#include "CBitReader.h"
#include "CLogServices.h"
#include "SError-Apple.h"

#include <CoreMedia/CoreMedia.h>
#include <VideoToolbox/VideoToolbox.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CH264DecodeVideoCodec

class CH264DecodeVideoCodec : public CDecodeVideoCodec {
	public:
										// Lifecycle methods
										CH264DecodeVideoCodec(const I<CMediaPacketSource>& mediaPacketSource,
												const CData& configurationData, UInt32 timeScale,
												const TNumericArray<UInt32>& keyframeIndexes) :
											mDecodeInfo(mediaPacketSource, configurationData, timeScale,
															keyframeIndexes),
													mFormatDescriptionRef(nil), mDecompressionSessionRef(nil)
											{}
										~CH264DecodeVideoCodec()
											{
												// Cleanup
												if (mFormatDescriptionRef != nil)
													// Release
													::CFRelease(mFormatDescriptionRef);

												if (mDecompressionSessionRef != nil) {
													// Invalidate and release
													::VTDecompressionSessionWaitForAsynchronousFrames(
															mDecompressionSessionRef);
													::VTDecompressionSessionInvalidate(mDecompressionSessionRef);
													::CFRelease(mDecompressionSessionRef);
												}
											}

										// CVideoCodec methods
				OI<SError>				setup(const SVideoProcessingFormat& videoProcessingFormat);
				void					seek(UniversalTimeInterval timeInterval);
				TIResult<CVideoFrame>	decode();

										// Class methods
		static	void					decompressionOutputCallback(void* decompressionOutputUserData,
												void* sourceFrameUserData, OSStatus status,
												VTDecodeInfoFlags decodeInfoFlags, CVImageBufferRef imageBufferRef,
												CMTime presentationTime, CMTime presentationDuration)
											{ *((CVImageBufferRef*) sourceFrameUserData) =
													(CVBufferRef) ::CFRetain(imageBufferRef); }

	private:
		CH264VideoCodec::DecodeInfo			mDecodeInfo;

		OV<UInt32>							mTimeScale;
		OI<SVideoProcessingFormat>			mVideoProcessingFormat;

		CMFormatDescriptionRef				mFormatDescriptionRef;
		VTDecompressionSessionRef			mDecompressionSessionRef;
		OI<CH264VideoCodec::FrameTiming>	mFrameTiming;
};

// MARK: CVideoCodec methods

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CH264DecodeVideoCodec::setup(const SVideoProcessingFormat& videoProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
			CH264VideoCodec::DecodeInfo::SPSPPSInfo	spsppsInfo = mDecodeInfo.getSPSPPSInfo();
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
					parameterSetPointers, parameterSetSizes, mDecodeInfo.getNALUHeaderLengthSize(),
					&mFormatDescriptionRef);
	ReturnErrorIfFailed(status, OSSTR("CMVideoFormatDescriptionCreateFromH264ParameterSets"));

	// Setup Decompression Session
	VTDecompressionOutputCallbackRecord	decompressionOutputCallbackRecord;
	decompressionOutputCallbackRecord.decompressionOutputCallback = decompressionOutputCallback;
	decompressionOutputCallbackRecord.decompressionOutputRefCon = this;

	CFMutableDictionaryRef	destinationImageBufferAttributes =
									::CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,
											&kCFTypeDictionaryValueCallBacks);

	switch (videoProcessingFormat.getCompatibility()) {
		case CVideoFrame::kCompatibilityCGImage:
			// CGImage
			::CFDictionarySetValue(destinationImageBufferAttributes, kCVPixelBufferCGImageCompatibilityKey,
					kCFBooleanTrue);
			break;

		case CVideoFrame::kCompatibilityMetal:
			// Metal
			::CFDictionarySetValue(destinationImageBufferAttributes, kCVPixelBufferMetalCompatibilityKey,
					kCFBooleanTrue);
			break;

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS)
		case CVideoFrame::kCompatibilityOpenGLES:
			// OpenGLES
			::CFDictionarySetValue(destinationImageBufferAttributes, kCVPixelBufferOpenGLESCompatibilityKey,
					kCFBooleanTrue);
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
			::VTDecompressionSessionCreate(kCFAllocatorDefault, mFormatDescriptionRef, videoDecoderSpecification,
					destinationImageBufferAttributes, &decompressionOutputCallbackRecord, &mDecompressionSessionRef);
	::CFRelease(destinationImageBufferAttributes);
	::CFRelease(videoDecoderSpecification);
	ReturnErrorIfFailed(status, OSSTR("VTDecompressionSessionCreate"));

	// Finish setup
	mTimeScale = OV<UInt32>(mDecodeInfo.getTimeScale());
	mVideoProcessingFormat = OI<SVideoProcessingFormat>(videoProcessingFormat);

	CH264VideoCodec::SequenceParameterSetPayload	spsPayload(
															CData(parameterSetPointers[0],
																	(CData::ByteCount) parameterSetSizes[0], false));
	mFrameTiming = OI<CH264VideoCodec::FrameTiming>(new CH264VideoCodec::FrameTiming(spsPayload));

	return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
void CH264DecodeVideoCodec::seek(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Reset
	::VTDecompressionSessionWaitForAsynchronousFrames(mDecompressionSessionRef);

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
	// Get next packet
	TIResult<CMediaPacketSource::DataInfo>	dataInfo = mDecodeInfo.getMediaPacketSource()->readNext();
	ReturnValueIfResultError(dataInfo, TIResult<CVideoFrame>(dataInfo.getError()));

	// Update frame timing
	TIResult<CH264VideoCodec::FrameTiming::Times>	times = mFrameTiming->updateFrom(*dataInfo);
	ReturnValueIfResultError(dataInfo, TIResult<CVideoFrame>(times.getError()));

	// Setup sample buffer
	const	CData&				data = dataInfo->getData();
			CMBlockBufferRef	blockBufferRef;
			OSStatus			status =
										::CMBlockBufferCreateWithMemoryBlock(kCFAllocatorDefault,
												(void*) data.getBytePtr(), data.getByteCount(), kCFAllocatorNull, nil,
												0, data.getByteCount(), 0, &blockBufferRef);
	LogOSStatusIfFailedAndReturnValue(status, OSSTR("CMBlockBufferCreateWithMemoryBlock"),
			TIResult<CVideoFrame>(SErrorFromOSStatus(status)));

	CMSampleTimingInfo	sampleTimingInfo;
	sampleTimingInfo.duration = ::CMTimeMake(dataInfo->getDuration(), *mTimeScale);
	sampleTimingInfo.decodeTimeStamp = ::CMTimeMake(times->mDecodeTime, *mTimeScale);
	sampleTimingInfo.presentationTimeStamp = ::CMTimeMake(times->mPresentationTime, *mTimeScale);

	size_t				sampleSize = data.getByteCount();
	CMSampleBufferRef	sampleBufferRef;
	status =
			::CMSampleBufferCreate(kCFAllocatorDefault, blockBufferRef, true, nil, nil, mFormatDescriptionRef, 1, 1,
					&sampleTimingInfo, 1, &sampleSize, &sampleBufferRef);
	::CFRelease(blockBufferRef);
	LogOSStatusIfFailedAndReturnValue(status, OSSTR("CMSampleBufferCreate"),
			TIResult<CVideoFrame>(SErrorFromOSStatus(status)));

	// Decode frame
	CVImageBufferRef	imageBufferRef;
	VTDecodeInfoFlags	decodeInfoFlags;
	status =
			::VTDecompressionSessionDecodeFrame(mDecompressionSessionRef, sampleBufferRef, 0, &imageBufferRef,
					&decodeInfoFlags);
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
