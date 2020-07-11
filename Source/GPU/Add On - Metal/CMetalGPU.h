//----------------------------------------------------------------------------------------------------------------------
//	CMetalGPU.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CGPU.h"

#include <Metal/Metal.h>
#include <QuartzCore/CAMetalLayer.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: SGPUProcsInfo

typedef	id<MTLDevice>				(*CMetalGPUGetDeviceProc)(void* userData);
typedef	id<CAMetalDrawable>			(*CMetalGPUGetCurrentDrawableProc)(void* userData);
typedef	MTLPixelFormat				(*CMetalGPUGetPixelFormatProc)(void* userData);
typedef	NSUInteger					(*CMetalGPUGetSampleCountProc)(void* userData);
typedef	MTLRenderPassDescriptor*	(*CMetalGPUGetCurrentRenderPassDescriptor)(void* userData);

struct SGPUProcsInfo {
									// Lifecycle methods
									SGPUProcsInfo(CMetalGPUGetDeviceProc getDeviceProc,
											CMetalGPUGetCurrentDrawableProc getCurrentDrawableProc,
											CMetalGPUGetPixelFormatProc getPixelFormatProc,
											CMetalGPUGetSampleCountProc getSampleCountProc,
											CMetalGPUGetCurrentRenderPassDescriptor getCurrentRenderPassDescriptorProc,
											void* userData) :
										mGetDeviceProc(getDeviceProc), mGetCurrentDrawableProc(getCurrentDrawableProc),
												mGetPixelFormatProc(getPixelFormatProc),
												mGetSampleCountProc(getSampleCountProc),
												mGetCurrentRenderPassDescriptorProc(getCurrentRenderPassDescriptorProc),
												mUserData(userData)
										{}
									SGPUProcsInfo(const SGPUProcsInfo& other) :
										mGetDeviceProc(other.mGetDeviceProc),
												mGetCurrentDrawableProc(other.mGetCurrentDrawableProc),
												mGetPixelFormatProc(other.mGetPixelFormatProc),
												mGetSampleCountProc(other.mGetSampleCountProc),
												mGetCurrentRenderPassDescriptorProc(
														other.mGetCurrentRenderPassDescriptorProc),
												mUserData(other.mUserData)
										{}

									// Instance methods
		id<MTLDevice>				getDevice() const
										{ return mGetDeviceProc(mUserData); }
		id<CAMetalDrawable>			getCurrentDrawable() const
										{ return mGetCurrentDrawableProc(mUserData); }
		MTLPixelFormat				getPixelFormat() const
										{ return mGetPixelFormatProc(mUserData); }
		NSUInteger					getSampleCount() const
										{ return mGetSampleCountProc(mUserData); }
		MTLRenderPassDescriptor*	getCurrentRenderPassDescriptor() const
										{ return mGetCurrentRenderPassDescriptorProc(mUserData); }

	// Properties
	private:
		CMetalGPUGetDeviceProc					mGetDeviceProc;
		CMetalGPUGetCurrentDrawableProc			mGetCurrentDrawableProc;
		CMetalGPUGetPixelFormatProc				mGetPixelFormatProc;
		CMetalGPUGetSampleCountProc				mGetSampleCountProc;
		CMetalGPUGetCurrentRenderPassDescriptor	mGetCurrentRenderPassDescriptorProc;
		void*									mUserData;
};
