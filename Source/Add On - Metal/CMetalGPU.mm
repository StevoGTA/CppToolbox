//----------------------------------------------------------------------------------------------------------------------
//	CMetalGPU.mm			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#import "CMetalGPU.h"

#import "CMetalRenderState.h"
#import "CMetalTexture.h"

#import "MetalShaderTypes.h"

/*
	TODOs:
		Frame delta time in views
		Clip Vertex Shader
		Opacity Fragment Shader
		Auto-scale matrix to aspect fit
 */

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPUInternals

class CGPUInternals {
	public:
		CGPUInternals(const CGPUProcsInfo& procsInfo) :
			mProcsInfo(procsInfo),
					mCommandQueue([procsInfo.getDevice() newCommandQueue]),
					mShaderLibrary([procsInfo.getDevice() newDefaultLibrary]),
					mGlobalUniformsBuffer(
							[procsInfo.getDevice() newBufferWithLength:sizeof(GlobalUniforms)
									options:MTLResourceStorageModeShared]),
					mRenderPipelineDescriptor([[MTLRenderPipelineDescriptor alloc] init]),
					mMetalShadersMap([[NSMutableDictionary alloc] init]),
					mCurrentCommandBuffer(nil), mCurrentRenderCommandEncoder(nil)
			{
				// Finish setup
				mGlobalUniformsBuffer.label = @"Global Uniforms";

				GlobalUniforms*	globalUniforms = (GlobalUniforms*) mGlobalUniformsBuffer.contents;
				globalUniforms->mViewMatrix =
						(matrix_float4x4)
								{{
									{1.0, 0.0, 0.0, 0.0},
									{0.0, 1.0, 0.0, 0.0},
									{0.0, 0.0, 1.0, 0.0},
									{0.0, 0.0, 0.0, 1.0},
								}};

				mRenderPipelineDescriptor.sampleCount = procsInfo.getSampleCount();
				mRenderPipelineDescriptor.colorAttachments[0].pixelFormat = procsInfo.getPixelFormat();
			}

	CGPUProcsInfo										mProcsInfo;

	id<MTLCommandQueue>									mCommandQueue;
	id<MTLLibrary>										mShaderLibrary;
	id<MTLBuffer>										mGlobalUniformsBuffer;
	MTLRenderPipelineDescriptor*						mRenderPipelineDescriptor;
	NSMutableDictionary<NSString*, id<MTLFunction>>*	mMetalShadersMap;

	id<MTLCommandBuffer>								mCurrentCommandBuffer;
	id<MTLRenderCommandEncoder>							mCurrentRenderCommandEncoder;
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
SGPUVertexBuffer CGPU::allocateVertexBuffer(const SGPUVertexBufferInfo& gpuVertexBufferInfo, UInt32 vertexCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup Metal Vertex Descriptor
	MTLVertexDescriptor*	vertexDescriptor = [[MTLVertexDescriptor alloc] init];

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
	vertexDescriptor.attributes[kVertexAttributeTextureCoordinate].bufferIndex = kBufferIndexVertexTextureCoordinate;
	vertexDescriptor.layouts[kBufferIndexVertexTextureCoordinate].stride = gpuVertexBufferInfo.mTotalSize;
	vertexDescriptor.layouts[kBufferIndexVertexTextureCoordinate].stepRate = 1;
	vertexDescriptor.layouts[kBufferIndexVertexTextureCoordinate].stepFunction = MTLVertexStepFunctionPerVertex;

	vertexDescriptor.attributes[kVertexAttributeTextureIndex].format = MTLVertexFormatFloat;
	vertexDescriptor.attributes[kVertexAttributeTextureIndex].offset = gpuVertexBufferInfo.mTextureIndexOffset;
	vertexDescriptor.attributes[kVertexAttributeTextureIndex].bufferIndex = kBufferIndexVertexTextureIndex;
	vertexDescriptor.layouts[kBufferIndexVertexTextureIndex].stride = gpuVertexBufferInfo.mTotalSize;
	vertexDescriptor.layouts[kBufferIndexVertexTextureIndex].stepRate = 1;
	vertexDescriptor.layouts[kBufferIndexVertexTextureIndex].stepFunction = MTLVertexStepFunctionPerVertex;

	return SGPUVertexBuffer(gpuVertexBufferInfo, CData(gpuVertexBufferInfo.mTotalSize * vertexCount),
			(void*) CFBridgingRetain(vertexDescriptor));
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::disposeBuffer(const SGPUBuffer& buffer)
//----------------------------------------------------------------------------------------------------------------------
{
	// Cleanup
	CFBridgingRelease(buffer.mInternalReference);
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::renderStart() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup uniforms
	CGSize			size = mInternals->mProcsInfo.getCurrentDrawable().layer.bounds.size;
	GlobalUniforms*	globalUniforms = (GlobalUniforms*) mInternals->mGlobalUniformsBuffer.contents;
	globalUniforms->mProjectionMatrix =
			(matrix_float4x4)
					{{
						{2.0 / size.width, 0.0, 0.0, 0.0},
						{0.0, 2.0 / -size.height, 0.0, 0.0},
						{0.0, 0.0, -0.5, 0.0},
						{-1.0, 1.0, 0.5, 1.0},
					}};

	// Setup current command buffer
	mInternals->mCurrentCommandBuffer = [mInternals->mCommandQueue commandBuffer];
	mInternals->mCurrentCommandBuffer.label = @"Current Command Buffer";

	// Setup current render command encoder
	mInternals->mCurrentRenderCommandEncoder =
			[mInternals->mCurrentCommandBuffer
					renderCommandEncoderWithDescriptor:mInternals->mProcsInfo.getCurrentRenderPassDescriptor()];
	mInternals->mCurrentRenderCommandEncoder.label = @"Render Command Encoder";
	[mInternals->mCurrentRenderCommandEncoder setVertexBuffer:mInternals->mGlobalUniformsBuffer offset:0
			atIndex:kBufferIndexGlobalUniforms];
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::setViewMatrix(const SMatrix4x4_32& viewMatrix)
//----------------------------------------------------------------------------------------------------------------------
{
	((GlobalUniforms*) mInternals->mGlobalUniformsBuffer.contents)->mViewMatrix = *((matrix_float4x4*) &viewMatrix);
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
					mInternals->mMetalShadersMap));

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
