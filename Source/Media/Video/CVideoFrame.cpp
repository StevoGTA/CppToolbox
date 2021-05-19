//----------------------------------------------------------------------------------------------------------------------
//	CVideoFrame.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CVideoFrame.h"

#include "SError.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CVideoFrameInternals

class CVideoFrameInternals : public TCopyOnWriteReferenceCountable<CVideoFrameInternals> {
	public:
#if TARGET_OS_IOS || TARGET_OS_MACOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
		CVideoFrameInternals(UniversalTimeInterval presentationTimeInterval, const S2DSizeU16& frameSize,
				CVideoFrame::DataFormat dataFormat, CVImageBufferRef imageBufferRef) :
			mPresentationTimeInterval(presentationTimeInterval), mFrameSize(frameSize), mDataFormat(dataFormat),
					mImageBufferRef((CVImageBufferRef) ::CFRetain(imageBufferRef))
			{}
#endif
		~CVideoFrameInternals()
			{
#if TARGET_OS_IOS || TARGET_OS_MACOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
				// Cleanup
				::CFRelease(mImageBufferRef);
#endif
			}

		UniversalTimeInterval	mPresentationTimeInterval;
		S2DSizeU16				mFrameSize;
		CVideoFrame::DataFormat	mDataFormat;

#if TARGET_OS_IOS || TARGET_OS_MACOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
		CVImageBufferRef		mImageBufferRef;
#endif
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CVideoFrame

// MARK: Lifecycle methods

#if TARGET_OS_IOS || TARGET_OS_MACOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
//----------------------------------------------------------------------------------------------------------------------
CVideoFrame::CVideoFrame(UniversalTimeInterval presentationTimeInterval, CVImageBufferRef imageBufferRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CGSize	frameSize = ::CVImageBufferGetDisplaySize(imageBufferRef);

	DataFormat	dataFormat;
	switch (::CVPixelBufferGetPixelFormatType(imageBufferRef)) {
		case 'BGRA':
			// RGB
			dataFormat = kDataFormatRGB;
			break;

		case '420v':
			// YCbCr
			dataFormat = kDataFormatYCbCr;
			break;
	}

	mInternals =
			new CVideoFrameInternals(presentationTimeInterval, S2DSizeU16(frameSize.width, frameSize.height),
					dataFormat, imageBufferRef);
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
const S2DSizeU16& CVideoFrame::getFrameSize() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mFrameSize;
}

//----------------------------------------------------------------------------------------------------------------------
CVideoFrame::DataFormat CVideoFrame::getDataFormat() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mDataFormat;
}

//----------------------------------------------------------------------------------------------------------------------
CColor::Primaries CVideoFrame::getColorPrimaries() const
//----------------------------------------------------------------------------------------------------------------------
{
#if TARGET_OS_IOS || TARGET_OS_MACOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
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
#endif
}

//----------------------------------------------------------------------------------------------------------------------
CColor::YCbCrConversionMatrix CVideoFrame::getYCbCrConversionMatrix() const
//----------------------------------------------------------------------------------------------------------------------
{
#if TARGET_OS_IOS || TARGET_OS_MACOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
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
#endif
}

//----------------------------------------------------------------------------------------------------------------------
CColor::TransferFunction CVideoFrame::getColorTransferFunction() const
//----------------------------------------------------------------------------------------------------------------------
{
#if TARGET_OS_IOS || TARGET_OS_MACOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
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
#endif
}

#if TARGET_OS_IOS || TARGET_OS_MACOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
//----------------------------------------------------------------------------------------------------------------------
CVImageBufferRef CVideoFrame::getImageBufferRef() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mImageBufferRef;
}
#endif

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
ECompareResult CVideoFrame::comparePresentationTimeInterval(const CVideoFrame& videoFrame1,
		const CVideoFrame& videoFrame2, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	return eCompare(videoFrame1.getPresentationTimeInterval(), videoFrame2.getPresentationTimeInterval());
}
