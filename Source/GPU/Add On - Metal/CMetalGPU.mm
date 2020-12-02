//----------------------------------------------------------------------------------------------------------------------
//	CMetalGPU.mm			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#import "CMetalGPU.h"

#import "CMetalRenderState.h"
#import "CMetalTexture.h"
#import "ConcurrencyPrimitives.h"
#import "SError.h"

#import "MetalShaderTypes.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	const	UInt32	kBufferCount = 3;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUInternals

class CGPUInternals {
	public:
		CGPUInternals(const SGPUProcsInfo& procsInfo) :
			mProcsInfo(procsInfo),
					mCommandQueue([procsInfo.getDevice() newCommandQueue]),
					mShaderLibrary([procsInfo.getDevice() newDefaultLibrary]),
					mFunctionsCache([[NSMutableDictionary alloc] init]),
					mRenderPipelineDescriptor([[MTLRenderPipelineDescriptor alloc] init]),
					mRenderPipelineStateCache([[NSMutableDictionary alloc] init]),
					mSharedResourceBuffers(kBufferCount), mMetalBufferCacheIndex(0),
					mCurrentCommandBuffer(nil), mCurrentRenderCommandEncoder(nil)
			{
				// Finish setup
				MTLSamplerDescriptor*	samplerDescriptor = [[MTLSamplerDescriptor alloc] init];
				mSamplerState = [procsInfo.getDevice() newSamplerStateWithDescriptor:samplerDescriptor];

				MTLDepthStencilDescriptor*	depthStencilDescriptor = [[MTLDepthStencilDescriptor alloc] init];
				depthStencilDescriptor.depthCompareFunction = MTLCompareFunctionLessEqual;
				depthStencilDescriptor.depthWriteEnabled = NO;
				mDepthStencilState2D =
						[procsInfo.getDevice() newDepthStencilStateWithDescriptor:depthStencilDescriptor];

				depthStencilDescriptor.depthWriteEnabled = YES;
				mDepthStencilState3D =
						[procsInfo.getDevice() newDepthStencilStateWithDescriptor:depthStencilDescriptor];

				mMetalBufferCaches = [NSMutableArray arrayWithCapacity:kBufferCount];
				for (NSUInteger i = 0; i < kBufferCount; i++)
					[mMetalBufferCaches addObject:[[MetalBufferCache alloc] initWithDevice:mProcsInfo.getDevice()]];

 				mRenderPipelineDescriptor.sampleCount = procsInfo.getSampleCount();
				mRenderPipelineDescriptor.colorAttachments[0].pixelFormat = procsInfo.getPixelFormat();
			}

	SGPUProcsInfo												mProcsInfo;

	id<MTLCommandQueue>											mCommandQueue;
	id<MTLLibrary>												mShaderLibrary;
	id<MTLSamplerState>											mSamplerState;
	id<MTLDepthStencilState>									mDepthStencilState2D;
	id<MTLDepthStencilState>									mDepthStencilState3D;
	NSMutableDictionary<NSString*, id<MTLFunction>>*			mFunctionsCache;
	MTLRenderPipelineDescriptor*								mRenderPipelineDescriptor;
	NSMutableDictionary<NSString*, id<MTLRenderPipelineState>>*	mRenderPipelineStateCache;

	CSharedResource												mSharedResourceBuffers;
	NSMutableArray<MetalBufferCache*>*							mMetalBufferCaches;
	UInt32														mMetalBufferCacheIndex;

	id<MTLCommandBuffer>										mCurrentCommandBuffer;
	id<MTLRenderCommandEncoder>									mCurrentRenderCommandEncoder;

	SMatrix4x4_32												mViewMatrix2D;
	SMatrix4x4_32												mProjectionMatrix2D;
	
	SMatrix4x4_32												mViewMatrix3D;
	SMatrix4x4_32												mProjectionMatrix3D;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPU

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CGPU::CGPU(const SGPUProcsInfo& procsInfo)
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
SGPUBuffer CGPU::allocateIndexBuffer(const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	// Create buffer
	id<MTLBuffer>	mtlBuffer =
							[mInternals->mProcsInfo.getDevice() newBufferWithBytes:data.getBytePtr()
									length:data.getSize() options:MTLResourceStorageModeShared];

	return SGPUBuffer((void*) ::CFBridgingRetain(mtlBuffer));
}

//----------------------------------------------------------------------------------------------------------------------
SGPUVertexBuffer CGPU::allocateVertexBuffer(UInt32 perVertexByteCount, const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	// Create buffer
	id<MTLBuffer>	mtlBuffer =
							[mInternals->mProcsInfo.getDevice() newBufferWithBytes:data.getBytePtr()
									length:data.getSize() options:MTLResourceStorageModeShared];

	return SGPUVertexBuffer(perVertexByteCount, (void*) ::CFBridgingRetain(mtlBuffer));
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::disposeBuffer(const SGPUBuffer& buffer)
//----------------------------------------------------------------------------------------------------------------------
{
	// Cleanup
	::CFBridgingRelease(buffer.mPlatformReference);
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::renderStart(const S2DSizeF32& size2D, Float32 fieldOfViewAngle3D, Float32 aspectRatio3D, Float32 nearZ3D,
		Float32 farZ3D, const S3DPointF32& camera3D, const S3DPointF32& target3D, const S3DVectorF32& up3D) const
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

	// Projection 2D
	mInternals->mProjectionMatrix2D =
			SMatrix4x4_32(
							2.0 / size2D.mWidth, 0.0, 0.0, 0.0,
							0.0, 2.0 / -size2D.mHeight, 0.0, 0.0,
							0.0, 0.0, -0.5, 0.0,
							-1.0, 1.0, 0.5, 1.0);

	// View 3D
	S3DVectorF32	camera3DVector = S3DVectorF32(camera3D.mX, camera3D.mY, camera3D.mZ);
	S3DVectorF32	target3DVector = S3DVectorF32(target3D.mX, target3D.mY, target3D.mZ);
	S3DVectorF32	f = (target3DVector - camera3DVector).normalized();
	S3DVectorF32	s = f.crossed(up3D).normalized();
	S3DVectorF32	t = s.crossed(f);
	mInternals->mViewMatrix3D =
			SMatrix4x4_32(
					s.mDX, t.mDX, -f.mDX, 0.0,
					s.mDY, t.mDY, -f.mDY, 0.0,
					s.mDZ, t.mDZ, -f.mDZ, 0.0,
					0.0, 0.0, 0.0, 1.0)
			.translated(S3DOffsetF32(-camera3D.mX, -camera3D.mY, -camera3D.mZ));

	// Projection 3D
	Float32	ys = 1.0 / tanf(fieldOfViewAngle3D * 0.5);
	Float32	xs = ys / aspectRatio3D;
	Float32	zs = farZ3D / (nearZ3D - farZ3D);
	mInternals->mProjectionMatrix3D =
			SMatrix4x4_32(
					xs,		0.0,	0.0,			0.0,
					0.0,	ys,		0.0,			0.0,
					0.0,	0.0,	zs,				-1.0,
					0.0,	0.0,	nearZ3D * zs,	0.0);

	// Setup Metal Buffer Cache
	mInternals->mSharedResourceBuffers.consume();
	[mInternals->mMetalBufferCaches[mInternals->mMetalBufferCacheIndex] reset];
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::render(CGPURenderState& renderState, EGPURenderType type, UInt32 count, UInt32 offset)
//----------------------------------------------------------------------------------------------------------------------
{
	// Finalize render state
	switch (renderState.getRenderMode()) {
		case kGPURenderMode2D:
			// 2D
			[mInternals->mCurrentRenderCommandEncoder setDepthStencilState:mInternals->mDepthStencilState2D];
			renderState.commit(
					SGPURenderStateCommitInfo(mInternals->mProcsInfo.getDevice(), mInternals->mShaderLibrary,
							mInternals->mCurrentRenderCommandEncoder,
							mInternals->mMetalBufferCaches[mInternals->mMetalBufferCacheIndex],
							mInternals->mFunctionsCache, mInternals->mRenderPipelineDescriptor,
							mInternals->mRenderPipelineStateCache, mInternals->mViewMatrix2D,
							mInternals->mProjectionMatrix2D));
			break;

		case kGPURenderMode3D:
			// 3D
			[mInternals->mCurrentRenderCommandEncoder setDepthStencilState:mInternals->mDepthStencilState3D];
			renderState.commit(
					SGPURenderStateCommitInfo(mInternals->mProcsInfo.getDevice(), mInternals->mShaderLibrary,
							mInternals->mCurrentRenderCommandEncoder,
							mInternals->mMetalBufferCaches[mInternals->mMetalBufferCacheIndex],
							mInternals->mFunctionsCache, mInternals->mRenderPipelineDescriptor,
							mInternals->mRenderPipelineStateCache, mInternals->mViewMatrix3D,
							mInternals->mProjectionMatrix3D));
			break;
	}

	// Check type
	switch (type) {
		case kGPURenderTypeTriangleList:
			// Triangle list
			[mInternals->mCurrentRenderCommandEncoder pushDebugGroup:@"Triangle Strip"];
			AssertFailUnimplemented();
			[mInternals->mCurrentRenderCommandEncoder popDebugGroup];
			break;

		case kGPURenderTypeTriangleStrip:
			// Triangle strip
			[mInternals->mCurrentRenderCommandEncoder pushDebugGroup:@"Triangle Strip"];
			[mInternals->mCurrentRenderCommandEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip
					vertexStart:offset vertexCount:count];
			[mInternals->mCurrentRenderCommandEncoder popDebugGroup];
			break;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::renderIndexed(CGPURenderState& renderState, EGPURenderType type, UInt32 count, UInt32 offset)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	OR<const SGPUBuffer>&	indexBuffer = renderState.getIndexBuffer();
			id<MTLBuffer>			mtlBuffer = (__bridge id<MTLBuffer>) indexBuffer->mPlatformReference;

	// Finalize render state
	switch (renderState.getRenderMode()) {
		case kGPURenderMode2D:
			// 2D
			[mInternals->mCurrentRenderCommandEncoder setDepthStencilState:mInternals->mDepthStencilState2D];
			renderState.commit(
					SGPURenderStateCommitInfo(mInternals->mProcsInfo.getDevice(), mInternals->mShaderLibrary,
							mInternals->mCurrentRenderCommandEncoder,
							mInternals->mMetalBufferCaches[mInternals->mMetalBufferCacheIndex],
							mInternals->mFunctionsCache, mInternals->mRenderPipelineDescriptor,
							mInternals->mRenderPipelineStateCache, mInternals->mViewMatrix2D,
							mInternals->mProjectionMatrix2D));
			break;

		case kGPURenderMode3D:
			// 3D
			[mInternals->mCurrentRenderCommandEncoder setDepthStencilState:mInternals->mDepthStencilState3D];
			renderState.commit(
					SGPURenderStateCommitInfo(mInternals->mProcsInfo.getDevice(), mInternals->mShaderLibrary,
							mInternals->mCurrentRenderCommandEncoder,
							mInternals->mMetalBufferCaches[mInternals->mMetalBufferCacheIndex],
							mInternals->mFunctionsCache, mInternals->mRenderPipelineDescriptor,
							mInternals->mRenderPipelineStateCache, mInternals->mViewMatrix3D,
							mInternals->mProjectionMatrix3D));
			break;
	}

	// Check type
	switch (type) {
		case kGPURenderTypeTriangleList:
			// Triangle list
			[mInternals->mCurrentRenderCommandEncoder pushDebugGroup:@"Triangle List Indexed"];
			[mInternals->mCurrentRenderCommandEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:count
					indexType:MTLIndexTypeUInt16 indexBuffer:mtlBuffer indexBufferOffset:0];
			[mInternals->mCurrentRenderCommandEncoder popDebugGroup];
			break;

		case kGPURenderTypeTriangleStrip:
			// Triangle strip
			[mInternals->mCurrentRenderCommandEncoder pushDebugGroup:@"Triangle Strip Indexed"];
			AssertFailUnimplemented();
			[mInternals->mCurrentRenderCommandEncoder popDebugGroup];
			break;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::renderEnd() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Next Metal Buffer Cache
	mInternals->mMetalBufferCacheIndex = (mInternals->mMetalBufferCacheIndex + 1) % kBufferCount;

	// Setup for command completion
	[mInternals->mCurrentCommandBuffer addCompletedHandler:^(id<MTLCommandBuffer> commandBuffer) {
		// We're done
		mInternals->mSharedResourceBuffers.release();
	}];

	// Cleanup
	[mInternals->mCurrentRenderCommandEncoder endEncoding];
	mInternals->mCurrentRenderCommandEncoder = nil;

	[mInternals->mCurrentCommandBuffer presentDrawable:mInternals->mProcsInfo.getCurrentDrawable()];
	[mInternals->mCurrentCommandBuffer commit];
	mInternals->mCurrentCommandBuffer = nil;
}
