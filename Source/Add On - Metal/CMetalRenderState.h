//----------------------------------------------------------------------------------------------------------------------
//	CMetalRenderState.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#import <Metal/Metal.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: SGPURenderStateCommitInfo

struct SGPURenderStateCommitInfo {
	// Lifecycle methods
	SGPURenderStateCommitInfo(id<MTLDevice> device, id<MTLLibrary> shaderLibrary,
			id<MTLRenderCommandEncoder> renderCommandEncoder, MTLRenderPipelineDescriptor* renderPipelineDescriptor,
			NSMutableDictionary<NSString*, id<MTLFunction>>* metalShadersMap) :
		mDevice(device), mShaderLibrary(shaderLibrary), mRenderCommandEncoder(renderCommandEncoder),
				mRenderPipelineDescriptor(renderPipelineDescriptor), mMetalShadersMap(metalShadersMap)
		{}

	// Properties
	id<MTLDevice>										mDevice;
	id<MTLLibrary>										mShaderLibrary;
	id<MTLRenderCommandEncoder>							mRenderCommandEncoder;
	MTLRenderPipelineDescriptor*						mRenderPipelineDescriptor;
	NSMutableDictionary<NSString*, id<MTLFunction>>*	mMetalShadersMap;
};
