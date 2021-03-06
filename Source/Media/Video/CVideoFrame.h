//----------------------------------------------------------------------------------------------------------------------
//	CVideoFrame.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "C2DGeometry.h"
#include "CColor.h"
#include "TimeAndDate.h"

#if TARGET_OS_IOS || TARGET_OS_MACOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
	#include <CoreVideo/CoreVideo.h>
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
				CVImageBufferRef						getImageBufferRef() const;
#endif

														// Class methods
		static	ECompareResult							comparePresentationTimeInterval(const CVideoFrame& videoFrame1,
																const CVideoFrame& videoFrame2, void* userData);

	// Properties
	private:
		CVideoFrameInternals*	mInternals;
};
