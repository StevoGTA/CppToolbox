//----------------------------------------------------------------------------------------------------------------------
//	CCoreMediaVideoCodec.cpp			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CCoreMediaVideoCodec.h"

#include "SError-Apple.h"

#include <VideoToolbox/VideoToolbox.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CCoreMediaDecodeVideoCodec::Internals

class CCoreMediaDecodeVideoCodec::Internals {
	public:
						Internals(OSType codecID,
								const I<CMediaPacketSource>& mediaPacketSource, UInt32 timeScale,
								const TNumberArray<UInt32>& keyframeIndexes) :
							mCodecID(codecID), mMediaPacketSource(mediaPacketSource), mTimeScale(timeScale),
									mKeyframeIndexes(keyframeIndexes),
									mFormatDescriptionRef(nil), mDecompressionSessionRef(nil)
							{}
						~Internals()
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

						// Class methods
		static	void	decompressionOutputCallback(void* decompressionOutputUserData, void* sourceFrameUserData,
								OSStatus status, VTDecodeInfoFlags decodeInfoFlags, CVImageBufferRef imageBufferRef,
								CMTime presentationTime, CMTime presentationDuration)
							{ *((CVImageBufferRef*) sourceFrameUserData) = (CVBufferRef) ::CFRetain(imageBufferRef); }

		OSType						mCodecID;
		I<CMediaPacketSource>		mMediaPacketSource;
		UInt32						mTimeScale;
		TNumberArray<UInt32>		mKeyframeIndexes;

		OV<SVideoProcessingFormat>	mVideoProcessingFormat;

		CMFormatDescriptionRef		mFormatDescriptionRef;
		VTDecompressionSessionRef	mDecompressionSessionRef;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CCoreMediaDecodeVideoCodec

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CCoreMediaDecodeVideoCodec::CCoreMediaDecodeVideoCodec(OSType codecID, const I<CMediaPacketSource>& mediaPacketSource,
		UInt32 timeScale, const TNumberArray<UInt32>& keyframeIndexes)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(codecID, mediaPacketSource, timeScale, keyframeIndexes);
}

//----------------------------------------------------------------------------------------------------------------------
CCoreMediaDecodeVideoCodec::~CCoreMediaDecodeVideoCodec()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CDecodeVideoCodec methods

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CCoreMediaDecodeVideoCodec::setup(const SVideoProcessingFormat& videoProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Compose format description
	TVResult<CMFormatDescriptionRef>	formatDescription = composeFormatDescription();
	ReturnErrorIfResultError(formatDescription);
	mInternals->mFormatDescriptionRef = *formatDescription;

	// Setup Decompression Session
	VTDecompressionOutputCallbackRecord	decompressionOutputCallbackRecord;
	decompressionOutputCallbackRecord.decompressionOutputCallback = Internals::decompressionOutputCallback;
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

	OSStatus	status =
						::VTDecompressionSessionCreate(kCFAllocatorDefault, mInternals->mFormatDescriptionRef,
								videoDecoderSpecification, destinationImageBufferAttributes,
								&decompressionOutputCallbackRecord, &mInternals->mDecompressionSessionRef);
	::CFRelease(destinationImageBufferAttributes);
	::CFRelease(videoDecoderSpecification);
	ReturnErrorIfFailed(status, OSSTR("VTDecompressionSessionCreate"));

	// Finish setup
	mInternals->mVideoProcessingFormat = OV<SVideoProcessingFormat>(videoProcessingFormat);

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
void CCoreMediaDecodeVideoCodec::seek(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Reset
	::VTDecompressionSessionWaitForAsynchronousFrames(mInternals->mDecompressionSessionRef);

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
TIResult<CVideoFrame> CCoreMediaDecodeVideoCodec::decode()
//----------------------------------------------------------------------------------------------------------------------
{
	// Get next packet
	TVResult<CMediaPacketSource::DataInfo>	dataInfo = mInternals->mMediaPacketSource->readNext();
	ReturnValueIfResultError(dataInfo, TIResult<CVideoFrame>(dataInfo.getError()));

	// Compose sample timing info
	TVResult<CMSampleTimingInfo>	sampleTimingInfo = composeSampleTimingInfo(*dataInfo, mInternals->mTimeScale);
	ReturnValueIfResultError(sampleTimingInfo, TIResult<CVideoFrame>(sampleTimingInfo.getError()));

	// Setup sample buffer
	const	CData&				data = dataInfo->getData();
			CMBlockBufferRef	blockBufferRef;
			OSStatus			status =
										::CMBlockBufferCreateWithMemoryBlock(kCFAllocatorDefault,
												(void*) data.getBytePtr(), data.getByteCount(), kCFAllocatorNull, nil,
												0, data.getByteCount(), 0, &blockBufferRef);
	LogOSStatusIfFailedAndReturnValue(status, OSSTR("CMBlockBufferCreateWithMemoryBlock"),
			TIResult<CVideoFrame>(SErrorFromOSStatus(status)));

	size_t				sampleSize = data.getByteCount();
	CMSampleBufferRef	sampleBufferRef;
	status =
			::CMSampleBufferCreate(kCFAllocatorDefault, blockBufferRef, true, nil, nil,
					mInternals->mFormatDescriptionRef, 1, 1, &*sampleTimingInfo, 1, &sampleSize, &sampleBufferRef);
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
									CVideoFrame(::CMTimeGetSeconds((*sampleTimingInfo).presentationTimeStamp),
											imageBufferRef));
	::CFRelease(imageBufferRef);

	return result;}
