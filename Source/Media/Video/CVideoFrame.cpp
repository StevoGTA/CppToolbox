//----------------------------------------------------------------------------------------------------------------------
//	CVideoFrame.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CVideoFrame.h"

#include "SError.h"

#if defined(TARGET_OS_WINDOWS)
	#include <mfapi.h>
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: CVideoFrame::Internals

class CVideoFrame::Internals : public TCopyOnWriteReferenceCountable<Internals> {
	public:
#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS)
		Internals(UniversalTimeInterval presentationTimeInterval, const S2DSizeU16& frameSize,
				CVideoFrame::DataFormat dataFormat, CVImageBufferRef imageBufferRef) :
			mPresentationTimeInterval(presentationTimeInterval), mFrameSize(frameSize),
					mViewRect(S2DPointU16(), frameSize), mDataFormat(dataFormat),
					mImageBufferRef((CVImageBufferRef) ::CFRetain(imageBufferRef))
			{}
#elif defined(TARGET_OS_WINDOWS)
		Internals(UniversalTimeInterval presentationTimeInterval, CVideoFrame::DataFormat dataFormat,
				const S2DSizeU16& frameSize, const S2DRectU16& viewRect, IMFSample* sample) :
			mPresentationTimeInterval(presentationTimeInterval), mDataFormat(dataFormat), mFrameSize(frameSize),
					mViewRect(viewRect), mSample(sample)
			{
				// Keep around
				mSample->AddRef();
			}
#endif
		~Internals()
			{
#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS)
				// Cleanup
				::CFRelease(mImageBufferRef);
#elif defined(TARGET_OS_WINDOWS)
				// Cleanup
				mSample->Release();
#endif
			}

		UniversalTimeInterval	mPresentationTimeInterval;
		CVideoFrame::DataFormat	mDataFormat;
		S2DSizeU16				mFrameSize;
		S2DRectU16				mViewRect;

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS)
		CVImageBufferRef		mImageBufferRef;
#elif defined(TARGET_OS_WINDOWS)
		IMFSample*				mSample;
#endif
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CVideoFrame

// MARK: Lifecycle methods

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS)
#if defined(TARGET_OS_MACOS)
	#define kCVPixelFormatType_Lossless_420YpCbCr8BiPlanarVideoRange '&8v0'
#endif
//----------------------------------------------------------------------------------------------------------------------
CVideoFrame::CVideoFrame(UniversalTimeInterval presentationTimeInterval, CVImageBufferRef imageBufferRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CGSize	frameSize = ::CVImageBufferGetDisplaySize(imageBufferRef);

	DataFormat	dataFormat;
	switch (::CVPixelBufferGetPixelFormatType(imageBufferRef)) {
		case kCVPixelFormatType_32ARGB:
		case kCVPixelFormatType_32BGRA:
			// RGB
			dataFormat = kDataFormatRGB;
			break;

		case kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange:
		case kCVPixelFormatType_Lossless_420YpCbCr8BiPlanarVideoRange:
			// YCbCr
			dataFormat = kDataFormatYCbCr;
			break;

		default:
			// Other
			AssertFailUnimplemented();
			dataFormat = kDataFormatRGB;
			break;
	}

	mInternals =
			new Internals(presentationTimeInterval, S2DSizeU16(frameSize.width, frameSize.height), dataFormat,
					imageBufferRef);
}
#elif defined(TARGET_OS_WINDOWS)
//----------------------------------------------------------------------------------------------------------------------
CVideoFrame::CVideoFrame(UniversalTimeInterval presentationTimeInterval, IMFSample* sample, const GUID& dataFormatGUID,
		const S2DSizeU16& frameSize, const S2DRectU16& viewRect)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	DataFormat	dataFormat;
	if (dataFormatGUID == MFVideoFormat_NV12)
		// YCbCr
		dataFormat = kDataFormatYCbCr;
	else {
		// Unknown
		AssertFailUnimplemented();
		dataFormat = kDataFormatYCbCr;
	}

	mInternals = new Internals(presentationTimeInterval, dataFormat, frameSize, viewRect, sample);
}
#endif

//----------------------------------------------------------------------------------------------------------------------
CVideoFrame::CVideoFrame(const CVideoFrame& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CVideoFrame::~CVideoFrame()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
UniversalTimeInterval CVideoFrame::getPresentationTimeInterval() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mPresentationTimeInterval;
}

//----------------------------------------------------------------------------------------------------------------------
CVideoFrame::DataFormat CVideoFrame::getDataFormat() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mDataFormat;
}

//----------------------------------------------------------------------------------------------------------------------
const S2DSizeU16& CVideoFrame::getFrameSize() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mFrameSize;
}

//----------------------------------------------------------------------------------------------------------------------
const S2DRectU16& CVideoFrame::getViewRect() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mViewRect;
}

//----------------------------------------------------------------------------------------------------------------------
CColor::Primaries CVideoFrame::getColorPrimaries() const
//----------------------------------------------------------------------------------------------------------------------
{
#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS)
	CFStringRef	stringRef =
						(CFStringRef) ::CVBufferGetAttachment(mInternals->mImageBufferRef,
								kCVImageBufferColorPrimariesKey, nil);
	if (::CFStringCompare(stringRef, kCVImageBufferColorPrimaries_SMPTE_C, 0) == 0)
		// Rec 601 / SMPTE C
		return CColor::kPrimariesRec601;
	else if (::CFStringCompare(stringRef, kCVImageBufferColorPrimaries_ITU_R_709_2, 0) == 0)
		// Rec 709
		return CColor::kPrimariesRec709;
	else if (::CFStringCompare(stringRef, kCVImageBufferColorPrimaries_ITU_R_2020, 0) == 0)
		// Rec 2020
		return CColor::kPrimariesRec2020;
	else if (::CFStringCompare(stringRef, kCVImageBufferColorPrimaries_EBU_3213, 0) == 0)
		// EBU 3213
		return CColor::kPrimariesEBU3213;
	else if (::CFStringCompare(stringRef, kCVImageBufferColorPrimaries_DCI_P3, 0) == 0)
		// DCI P3
		return CColor::kPrimariesDCIP3;
	else if (::CFStringCompare(stringRef, kCVImageBufferColorPrimaries_P3_D65, 0) == 0)
		// P3 D65
		return CColor::kPrimariesP3D65;
	else if (::CFStringCompare(stringRef, kCVImageBufferColorPrimaries_P22, 0) == 0)
		// P22
		return CColor::kPrimariesP22;
	else {
		// Not yet supported
		AssertFailUnimplemented();
		return CColor::kPrimariesRec601;
	}
#else
	return CColor::kPrimariesRec601;
#endif
}

//----------------------------------------------------------------------------------------------------------------------
CColor::YCbCrConversionMatrix CVideoFrame::getYCbCrConversionMatrix() const
//----------------------------------------------------------------------------------------------------------------------
{
#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS)
	CFStringRef	stringRef =
						(CFStringRef) ::CVBufferGetAttachment(mInternals->mImageBufferRef, kCVImageBufferYCbCrMatrixKey,
								nil);
	if (::CFStringCompare(stringRef, kCVImageBufferYCbCrMatrix_ITU_R_601_4, 0) == 0)
		// Rec 601
		return CColor::kYCbCrConversionMatrixRec601;
	else if (::CFStringCompare(stringRef, kCVImageBufferYCbCrMatrix_ITU_R_709_2, 0) == 0)
		// Rec 709
		return CColor::kYCbCrConversionMatrixRec709;
	else if (::CFStringCompare(stringRef, kCVImageBufferYCbCrMatrix_ITU_R_2020, 0) == 0)
		// Rec 2020
		return CColor::kYCbCrConversionMatrixRec2020;
	else {
		// Not yet supported
		AssertFailUnimplemented();
		return CColor::kYCbCrConversionMatrixRec601;
	}
#else
	return CColor::kYCbCrConversionMatrixRec601;
#endif
}

//----------------------------------------------------------------------------------------------------------------------
CColor::TransferFunction CVideoFrame::getColorTransferFunction() const
//----------------------------------------------------------------------------------------------------------------------
{
#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS)
	CFStringRef	stringRef =
						(CFStringRef) ::CVBufferGetAttachment(mInternals->mImageBufferRef,
								kCVImageBufferTransferFunctionKey, nil);
	if (::CFStringCompare(stringRef, kCVImageBufferTransferFunction_ITU_R_709_2, 0) == 0)
		// Rec 601/Rec709
		return CColor::kTransferFunctionRec709;
	else if (::CFStringCompare(stringRef, kCVImageBufferTransferFunction_ITU_R_2020, 0) == 0)
		// Rec 2020
		return CColor::kTransferFunctionRec2020;
	else if (::CFStringCompare(stringRef, kCVImageBufferTransferFunction_sRGB, 0) == 0)
		// sRGB
		return CColor::kTransferFunctionSRGB;
	else {
		// Not yet supported
		AssertFailUnimplemented();
		return CColor::kTransferFunctionRec709;
	}
#else
	return CColor::kTransferFunctionRec709;
#endif
}

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS)
//----------------------------------------------------------------------------------------------------------------------
CVImageBufferRef CVideoFrame::getImageBufferRef() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mImageBufferRef;
}
#elif defined(TARGET_OS_WINDOWS)
//----------------------------------------------------------------------------------------------------------------------
IMFSample* CVideoFrame::getSample() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mSample;
}
#endif

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
bool CVideoFrame::comparePresentationTimeInterval(const CVideoFrame& videoFrame1, const CVideoFrame& videoFrame2,
		void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	return eCompare(videoFrame1.getPresentationTimeInterval(), videoFrame2.getPresentationTimeInterval());
}
