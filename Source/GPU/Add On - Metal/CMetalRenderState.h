//----------------------------------------------------------------------------------------------------------------------
//	CMetalRenderState.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#import "MetalBufferCache.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SGPURenderStateCommitInfo

struct SGPURenderStateCommitInfo {
	// Lifecycle methods
	SGPURenderStateCommitInfo(id<MTLDevice> device, id<MTLLibrary> shaderLibrary,
			id<MTLRenderCommandEncoder> renderCommandEncoder, MetalBufferCache* metalBufferCache,
			NSMutableDictionary<NSString*, id<MTLFunction>>* functionsCache,
			MTLRenderPipelineDescriptor* renderPipelineDescriptor,
			NSMutableDictionary<NSString*, id<MTLRenderPipelineState>>* renderPipelineStateCache,
			const SMatrix4x4_32& viewMatrix, const SMatrix4x4_32& projectionMatrix) :
		mDevice(device), mShaderLibrary(shaderLibrary), mRenderCommandEncoder(renderCommandEncoder),
				mMetalBufferCache(metalBufferCache), mFunctionsCache(functionsCache),
				mRenderPipelineDescriptor(renderPipelineDescriptor),
				mRenderPipelineStateCache(renderPipelineStateCache), mViewMatrix(viewMatrix),
				mProjectionMatrix(projectionMatrix)
		{}

	// Properties
			id<MTLDevice>												mDevice;
			id<MTLLibrary>												mShaderLibrary;
			id<MTLRenderCommandEncoder>									mRenderCommandEncoder;
			MetalBufferCache*											mMetalBufferCache;
			NSMutableDictionary<NSString*, id<MTLFunction>>*			mFunctionsCache;
			MTLRenderPipelineDescriptor*								mRenderPipelineDescriptor;
			NSMutableDictionary<NSString*, id<MTLRenderPipelineState>>*	mRenderPipelineStateCache;
	const	SMatrix4x4_32&												mViewMatrix;
	const	SMatrix4x4_32&												mProjectionMatrix;
};
