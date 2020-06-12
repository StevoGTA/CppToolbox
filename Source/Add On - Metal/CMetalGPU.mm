//----------------------------------------------------------------------------------------------------------------------
//	CMetalGPU.mm			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#import "CMetalGPU.h"

#import "CMetalRenderState.h"
#import "CMetalTexture.h"

#import "MetalShaderTypes.h"

/*
	TODOs:
		Move buffer creation out of shader setup into somewhere else so happens less often.
 */

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPUInternals

class CGPUInternals {
	public:
		CGPUInternals(const CGPUProcsInfo& procsInfo) :
			mProcsInfo(procsInfo),
					mCommandQueue([procsInfo.getDevice() newCommandQueue]),
					mShaderLibrary([procsInfo.getDevice() newDefaultLibrary]),
					mRenderPipelineDescriptor([[MTLRenderPipelineDescriptor alloc] init]),
					mFunctionsCache([[NSMutableDictionary alloc] init]),
					mVertexDescriptorCache([[NSMutableDictionary alloc] init]),
					mRenderPipelineStateCache([[NSMutableDictionary alloc] init]),
					mCurrentCommandBuffer(nil), mCurrentRenderCommandEncoder(nil)
			{
				// Finish setup
				MTLSamplerDescriptor*	samplerDescriptor = [[MTLSamplerDescriptor alloc] init];
				mSamplerState = [procsInfo.getDevice() newSamplerStateWithDescriptor:samplerDescriptor];

 				mRenderPipelineDescriptor.sampleCount = procsInfo.getSampleCount();
				mRenderPipelineDescriptor.colorAttachments[0].pixelFormat = procsInfo.getPixelFormat();
			}

	CGPUProcsInfo												mProcsInfo;

	id<MTLCommandQueue>											mCommandQueue;
	id<MTLLibrary>												mShaderLibrary;
	id<MTLSamplerState>											mSamplerState;
	MTLRenderPipelineDescriptor*								mRenderPipelineDescriptor;
	NSMutableDictionary<NSString*, id<MTLFunction>>*			mFunctionsCache;
	NSMutableDictionary<NSString*, MTLVertexDescriptor*>*		mVertexDescriptorCache;
	NSMutableDictionary<NSString*, id<MTLRenderPipelineState>>*	mRenderPipelineStateCache;

	SMatrix4x4_32												mViewMatrix;

	id<MTLCommandBuffer>										mCurrentCommandBuffer;
	id<MTLRenderCommandEncoder>									mCurrentRenderCommandEncoder;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPU

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CGPU::CGPU(const CGPUProcsInfo& procsInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CGPUInternals(procsInfo);
}

//----------------------------------------------------------------------------------------------------------------------
CGPU::~CGPU()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CGPU methods

//----------------------------------------------------------------------------------------------------------------------
void CGPU::setViewMatrix(const SMatrix4x4_32& viewMatrix)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mViewMatrix = viewMatrix;
}

//----------------------------------------------------------------------------------------------------------------------
const SMatrix4x4_32& CGPU::getViewMatrix() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mViewMatrix;
}

//----------------------------------------------------------------------------------------------------------------------
SGPUTextureReference CGPU::registerTexture(const CData& data, EGPUTextureDataFormat gpuTextureDataFormat,
		const S2DSizeU16& size)
//----------------------------------------------------------------------------------------------------------------------
{
	// Register texture
	CGPUTexture*	gpuTexture =
							new CMetalTexture(mInternals->mProcsInfo.getDevice(), data, gpuTextureDataFormat, size);

	return SGPUTextureReference(*gpuTexture);
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::unregisterTexture(SGPUTextureReference& gpuTexture)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CMetalTexture*	metalTexture = (CMetalTexture*) gpuTexture.mGPUTexture;

	// Cleanup
	Delete(metalTexture);
}

//----------------------------------------------------------------------------------------------------------------------
SGPUVertexBuffer CGPU::allocateVertexBuffer(const SGPUVertexBufferInfo& gpuVertexBufferInfo, const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	// Retrieve metal vertex descriptor
	NSString*				vertexDescriptorUUID =
									(__bridge NSString*) gpuVertexBufferInfo.mUUID.getBase64String().getOSString();
	MTLVertexDescriptor*	vertexDescriptor = mInternals->mVertexDescriptorCache[vertexDescriptorUUID];
	if (vertexDescriptor == nil) {
		// Setup metal vertex descriptor
		vertexDescriptor = [[MTLVertexDescriptor alloc] init];

		vertexDescriptor.attributes[kVertexAttributePosition].format =
				MTLVertexFormatFloat + (gpuVertexBufferInfo.mVertexCount - 1);
		vertexDescriptor.attributes[kVertexAttributePosition].offset = gpuVertexBufferInfo.mVertexOffset;
		vertexDescriptor.attributes[kVertexAttributePosition].bufferIndex = kBufferIndexVertexPosition;
		vertexDescriptor.layouts[kBufferIndexVertexPosition].stride = gpuVertexBufferInfo.mTotalSize;
		vertexDescriptor.layouts[kBufferIndexVertexPosition].stepRate = 1;
		vertexDescriptor.layouts[kBufferIndexVertexPosition].stepFunction = MTLVertexStepFunctionPerVertex;

		vertexDescriptor.attributes[kVertexAttributeTextureCoordinate].format =
				MTLVertexFormatFloat + (gpuVertexBufferInfo.mTextureCoordinateCount - 1);
		vertexDescriptor.attributes[kVertexAttributeTextureCoordinate].offset =
				gpuVertexBufferInfo.mTextureCoordinateOffset;
		vertexDescriptor.attributes[kVertexAttributeTextureCoordinate].bufferIndex =
				kBufferIndexVertexTextureCoordinate;
		vertexDescriptor.layouts[kBufferIndexVertexTextureCoordinate].stride = gpuVertexBufferInfo.mTotalSize;
		vertexDescriptor.layouts[kBufferIndexVertexTextureCoordinate].stepRate = 1;
		vertexDescriptor.layouts[kBufferIndexVertexTextureCoordinate].stepFunction = MTLVertexStepFunctionPerVertex;

		vertexDescriptor.attributes[kVertexAttributeTextureIndex].format = MTLVertexFormatFloat;
		vertexDescriptor.attributes[kVertexAttributeTextureIndex].offset = gpuVertexBufferInfo.mTextureIndexOffset;
		vertexDescriptor.attributes[kVertexAttributeTextureIndex].bufferIndex = kBufferIndexVertexTextureIndex;
		vertexDescriptor.layouts[kBufferIndexVertexTextureIndex].stride = gpuVertexBufferInfo.mTotalSize;
		vertexDescriptor.layouts[kBufferIndexVertexTextureIndex].stepRate = 1;
		vertexDescriptor.layouts[kBufferIndexVertexTextureIndex].stepFunction = MTLVertexStepFunctionPerVertex;

		// Store
		mInternals->mVertexDescriptorCache[vertexDescriptorUUID] = vertexDescriptor;
	}

	// Setup vertex buffer
	id<MTLBuffer>	vertexBuffer =
							[mInternals->mProcsInfo.getDevice() newBufferWithBytes:data.getBytePtr()
									length:data.getSize() options:MTLResourceStorageModeShared];

	return SGPUVertexBuffer(gpuVertexBufferInfo,
			new SMetalVertexBufferInfo(vertexDescriptorUUID, vertexDescriptor, vertexBuffer));
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::disposeBuffer(const SGPUBuffer& buffer)
//----------------------------------------------------------------------------------------------------------------------
{
	// Cleanup
	SMetalVertexBufferInfo*	metalVertexBufferInfo = (SMetalVertexBufferInfo*) buffer.mInternalReference;
	Delete(metalVertexBufferInfo);
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::renderStart() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup current command buffer
	mInternals->mCurrentCommandBuffer = [mInternals->mCommandQueue commandBuffer];
	mInternals->mCurrentCommandBuffer.label = @"Current Command Buffer";

	// Setup current render command encoder
	mInternals->mCurrentRenderCommandEncoder =
			[mInternals->mCurrentCommandBuffer
					renderCommandEncoderWithDescriptor:mInternals->mProcsInfo.getCurrentRenderPassDescriptor()];
	mInternals->mCurrentRenderCommandEncoder.label = @"Render Command Encoder";

	[mInternals->mCurrentRenderCommandEncoder setFragmentSamplerState:mInternals->mSamplerState atIndex:0];

	// Setup uniforms
	CGSize			size = mInternals->mProcsInfo.getCurrentDrawable().layer.bounds.size;
	SMatrix4x4_32	projectionViewMatrix =
							SMatrix4x4_32(
											2.0 / size.width, 0.0, 0.0, 0.0,
											0.0, 2.0 / -size.height, 0.0, 0.0,
											0.0, 0.0, -0.5, 0.0,
											-1.0, 1.0, 0.5, 1.0) *
									mInternals->mViewMatrix;

	GlobalUniforms	globalUniforms;
	globalUniforms.mProjectionViewMatrix = *((matrix_float4x4*) &projectionViewMatrix);
	[mInternals->mCurrentRenderCommandEncoder setVertexBytes:&globalUniforms length:sizeof(GlobalUniforms)
			atIndex:kBufferIndexGlobalUniforms];
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::renderTriangleStrip(CGPURenderState& renderState, const SMatrix4x4_32& modelMatrix, UInt32 triangleCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Push debug group
	[mInternals->mCurrentRenderCommandEncoder pushDebugGroup:@"Triangle Strip"];

	// Finalize render state
	renderState.setModelMatrix(modelMatrix);
	renderState.commit(
			SGPURenderStateCommitInfo(mInternals->mProcsInfo.getDevice(), mInternals->mShaderLibrary,
					mInternals->mCurrentRenderCommandEncoder, mInternals->mRenderPipelineDescriptor,
					mInternals->mFunctionsCache, mInternals->mRenderPipelineStateCache));

	// Draw
	[mInternals->mCurrentRenderCommandEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip
			vertexStart:renderState.getTriangleOffset() vertexCount:triangleCount + 2];

	// Pop debug group
	[mInternals->mCurrentRenderCommandEncoder popDebugGroup];
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::renderEnd() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Cleanup
	[mInternals->mCurrentRenderCommandEncoder endEncoding];
	mInternals->mCurrentRenderCommandEncoder = nil;

	[mInternals->mCurrentCommandBuffer presentDrawable:mInternals->mProcsInfo.getCurrentDrawable()];
	[mInternals->mCurrentCommandBuffer commit];
	mInternals->mCurrentCommandBuffer = nil;
}
