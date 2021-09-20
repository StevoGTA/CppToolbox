//----------------------------------------------------------------------------------------------------------------------
//	CVideoFrame.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "C2DGeometry.h"
#include "CColor.h"
#include "TimeAndDate.h"

#if TARGET_OS_IOS || TARGET_OS_MACOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
	#include <CoreVideo/CoreVideo.h>
#elif TARGET_OS_WINDOWS
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

class CVideoFrameInternals;
class CVideoFrame {
	// Compatibility
	public:
		enum Compatibility {
#if TARGET_OS_IOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
			kCompatibilityAppleMetal,
			kCompatibilityAppleOpenGLES,
#elif TARGET_OS_MACOS
			kCompatibilityAppleMetal,
			kCompatibilityAppleOpenGL,
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

	// Methods
	public:
														// Lifecycle methods
#if TARGET_OS_IOS || TARGET_OS_MACOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
														CVideoFrame(UniversalTimeInterval presentationTimeInterval,
																CVImageBufferRef imageBufferRef);
#elif TARGET_OS_WINDOWS
														CVideoFrame(UniversalTimeInterval presentationTimeInterval,
																const S2DSizeU16& frameSize, const GUID& dataFormatGUID,
																IMFSample* sample);
#endif
														CVideoFrame(const CVideoFrame& other);
														~CVideoFrame();

														// Instance methods
						UniversalTimeInterval			getPresentationTimeInterval() const;
				const	S2DSizeU16&						getFrameSize() const;
						DataFormat						getDataFormat() const;
						CColor::Primaries				getColorPrimaries() const;
						CColor::YCbCrConversionMatrix	getYCbCrConversionMatrix() const;
						CColor::TransferFunction		getColorTransferFunction() const;

#if TARGET_OS_IOS || TARGET_OS_MACOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
						CVImageBufferRef				getImageBufferRef() const;
#endif

														// Class methods
		static			bool							comparePresentationTimeInterval(const CVideoFrame& videoFrame1,
																const CVideoFrame& videoFrame2, void* userData);

	// Properties
	private:
		CVideoFrameInternals*	mInternals;
};
