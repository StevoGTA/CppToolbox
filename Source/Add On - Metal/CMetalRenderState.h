//----------------------------------------------------------------------------------------------------------------------
//	CMetalRenderState.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#import <Metal/Metal.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: SMetalVertexBufferInfo

struct SMetalVertexBufferInfo {
	// Lifecycle methods
	SMetalVertexBufferInfo(NSString* vertexDescriptorUUID, MTLVertexDescriptor* vertexDescriptor,
			id<MTLBuffer> vertexBuffer) :
		mVertexDescriptorUUID(vertexDescriptorUUID), mVertexDescriptor(vertexDescriptor), mVertexBuffer(vertexBuffer)
		{}

	// Properties
	NSString*				mVertexDescriptorUUID;
	MTLVertexDescriptor*	mVertexDescriptor;
	id<MTLBuffer>			mVertexBuffer;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SGPURenderStateCommitInfo

struct SGPURenderStateCommitInfo {
	// Lifecycle methods
	SGPURenderStateCommitInfo(id<MTLDevice> device, id<MTLLibrary> shaderLibrary,
			id<MTLRenderCommandEncoder> renderCommandEncoder, MTLRenderPipelineDescriptor* renderPipelineDescriptor,
			NSMutableDictionary<NSString*, id<MTLFunction>>* functionsCache,
			NSMutableDictionary<NSString*, id<MTLRenderPipelineState>>* renderPipelineStateCache) :
		mDevice(device), mShaderLibrary(shaderLibrary), mRenderCommandEncoder(renderCommandEncoder),
				mRenderPipelineDescriptor(renderPipelineDescriptor), mFunctionsCache(functionsCache),
				mRenderPipelineStateCache(renderPipelineStateCache)
		{}

	// Properties
	id<MTLDevice>												mDevice;
	id<MTLLibrary>												mShaderLibrary;
	id<MTLRenderCommandEncoder>									mRenderCommandEncoder;
	MTLRenderPipelineDescriptor*								mRenderPipelineDescriptor;
	NSMutableDictionary<NSString*, id<MTLFunction>>*			mFunctionsCache;
	NSMutableDictionary<NSString*, id<MTLRenderPipelineState>>*	mRenderPipelineStateCache;
};
