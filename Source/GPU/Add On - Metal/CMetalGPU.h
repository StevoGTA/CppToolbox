//----------------------------------------------------------------------------------------------------------------------
//	CMetalGPU.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CGPU.h"

#include <Metal/Metal.h>
#include <QuartzCore/CAMetalLayer.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: SGPUProcsInfo

struct SGPUProcsInfo {
	// Procs
	typedef	id<MTLDevice>				(*GetDeviceProc)(void* userData);
	typedef	id<CAMetalDrawable>			(*GetCurrentDrawableProc)(void* userData);
	typedef	MTLPixelFormat				(*GetPixelFormatProc)(void* userData);
	typedef	NSUInteger					(*GetSampleCountProc)(void* userData);
	typedef	MTLRenderPassDescriptor*	(*GetCurrentRenderPassDescriptor)(void* userData);

									// Lifecycle methods
									SGPUProcsInfo(GetDeviceProc getDeviceProc,
											GetCurrentDrawableProc getCurrentDrawableProc,
											GetPixelFormatProc getPixelFormatProc,
											GetSampleCountProc getSampleCountProc,
											GetCurrentRenderPassDescriptor getCurrentRenderPassDescriptorProc,
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
		GetDeviceProc					mGetDeviceProc;
		GetCurrentDrawableProc			mGetCurrentDrawableProc;
		GetPixelFormatProc				mGetPixelFormatProc;
		GetSampleCountProc				mGetSampleCountProc;
		GetCurrentRenderPassDescriptor	mGetCurrentRenderPassDescriptorProc;
		void*							mUserData;
};
