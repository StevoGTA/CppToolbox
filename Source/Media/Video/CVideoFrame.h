//----------------------------------------------------------------------------------------------------------------------
//	CVideoFrame.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "C2DGeometry.h"
#include "CColor.h"
#include "TimeAndDate.h"

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS)
	#include <CoreVideo/CoreVideo.h>
#elif defined(TARGET_OS_WINDOWS)
	#undef Delete
	#include <mftransform.h>
	#undef THIS
	#define Delete(x)	{ delete x; x = nil; }
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: CVideoFrame

/*
	Terms:
		"frame" is an individual image corresponding to a time slice.
			A frame may not be directly displayable, eg. compressed "frames" may be temporal difference frames.

		"packet" is opaque data comprising a certain number of frames compressed into a certain number of
				bytes.  (For video data, this is typically 1 frame)
*/

class CVideoFrame {
	// Compatibility
	public:
		enum Compatibility {
#if defined(TARGET_OS_IOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS)
			kCompatibilityCGImage,
			kCompatibilityMetal,
			kCompatibilityOpenGLES,
#elif defined(TARGET_OS_MACOS)
			kCompatibilityCGImage,
			kCompatibilityMetal,
			kCompatibilityOpenGL,
#else
			kCompatibilityNotApplicable,
#endif
		};

	// Data Format
	public:
		enum DataFormat {
			kDataFormatRGB,
			kDataFormatYCbCr,
		};

	// Classes
	private:
		class Internals;

	// Methods
	public:
														// Lifecycle methods
#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS)
														CVideoFrame(UniversalTimeInterval presentationTimeInterval,
																CVImageBufferRef imageBufferRef);
#elif defined(TARGET_OS_WINDOWS)
														CVideoFrame(UniversalTimeInterval presentationTimeInterval,
																IMFSample* sample, const GUID& dataFormatGUID,
																const S2DSizeU16& frameSize,
																const S2DRectU16& viewRect);
#endif
														CVideoFrame(const CVideoFrame& other);
														~CVideoFrame();

														// Instance methods
						UniversalTimeInterval			getPresentationTimeInterval() const;
						DataFormat						getDataFormat() const;
				const	S2DSizeU16&						getFrameSize() const;
				const	S2DRectU16&						getViewRect() const;
						CColor::Primaries				getColorPrimaries() const;
						CColor::YCbCrConversionMatrix	getYCbCrConversionMatrix() const;
						CColor::TransferFunction		getColorTransferFunction() const;

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS)
						CVImageBufferRef				getImageBufferRef() const;
#elif defined(TARGET_OS_WINDOWS)
						IMFSample*						getSample() const;
#endif

														// Class methods
		static			bool							comparePresentationTimeInterval(const CVideoFrame& videoFrame1,
																const CVideoFrame& videoFrame2, void* userData);

	// Properties
	private:
		Internals*	mInternals;
};
