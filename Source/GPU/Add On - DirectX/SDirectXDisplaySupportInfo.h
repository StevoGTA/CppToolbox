//----------------------------------------------------------------------------------------------------------------------
//	SDirectXDisplaySupportInfo.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "C2DGeometry.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SDirectXDisplaySupportInfo

struct SDirectXDisplaySupportInfo {
					// Lifecycle methods
					SDirectXDisplaySupportInfo(bool alwaysSupportHighResolutions = false,
							Float32 highResolutionDPIThreshold = 192.0,
							S2DSizeF32 highResolutionSizeThreshold = S2DSizeF32(1920.0, 1080.0)) :
						mAlwaysSupportHighResolutions(alwaysSupportHighResolutions),
								mHighResolutionDPIThreshold(highResolutionDPIThreshold),
								mHighResolutionSizeThreshold(highResolutionSizeThreshold)
						{}

					// Instance methods
			Float32	getRenderDPI(Float32 screenDPI, S2DSizeF32 targetRenderSize)
						{
							// To improve battery life on high resolution devices, render to a smaller render target and
							//	allow the GPU to scale the output when it is presented.
							if (!mAlwaysSupportHighResolutions && (screenDPI > mHighResolutionDPIThreshold)) {
								// Setup
								Float32	width = pixelsFromDIPS(targetRenderSize.mWidth, screenDPI);
								Float32	height = pixelsFromDIPS(targetRenderSize.mHeight, screenDPI);

								// When the device is in portrait orientation, height > width. Compare the
								// larger dimension against the width threshold and the smaller dimension
								// against the height threshold.
								if ((std::max<Float32>(width, height) > mHighResolutionSizeThreshold.mWidth) &&
										(std::min<Float32>(width, height) > mHighResolutionSizeThreshold.mHeight))
									// To scale the app we change the effective DPI. Logical size does not change.
									return screenDPI / 2.0f;
								else
									// No scaling needed
									return screenDPI;
							} else
								// No scaling needed
								return screenDPI;
						}

					// Class methods
	static	Float32	pixelsFromDIPS(Float32 DIPs, Float32 DPI)
						{
							// Converts a length in device-independent pixels (DIPs) to a length in physical pixels.
							const	Float32	sDIPsPerInch = 96.0f;

							return floorf(DIPs * DPI / sDIPsPerInch + 0.5f); // Round to nearest integer.
						}

	// Properties
	// High resolution displays can require a lot of GPU and battery power to render.  High resolution phones, for
	//	example, may suffer from poor battery life if games attempt to render at 60 frames per second at full fidelity.
	//	The decision to render at full fidelity across all platforms and form factors should be deliberate.
	bool		mAlwaysSupportHighResolutions;

	// The default thresholds that define a "high resolution" display. If the thresholds are exceeded and
	//	mAlwaysSupportHighResolutions is false, the dimensions will be scaled by 50%.
	float		mHighResolutionDPIThreshold;
	S2DSizeF32	mHighResolutionSizeThreshold;
};

