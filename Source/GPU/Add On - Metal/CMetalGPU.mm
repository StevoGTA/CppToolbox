//----------------------------------------------------------------------------------------------------------------------
//	CMetalGPU.mm			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#import "CMetalGPU.h"

#import "CLogServices.h"
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
		CGPUInternals(const SGPUProcsInfo& procs) :
			mProcs(procs),
					mCommandQueue([procs.getDevice() newCommandQueue]),
					mShaderLibrary([procs.getDevice() newDefaultLibrary]),
					mFunctionsCache([[NSMutableDictionary alloc] init]),
					mRenderPipelineDescriptor([[MTLRenderPipelineDescriptor alloc] init]),
					mRenderPipelineStateCache([[NSMutableDictionary alloc] init]),
					mSharedResourceBuffers(kBufferCount), mMetalBufferCacheIndex(0),
					mCurrentCommandBuffer(nil), mCurrentRenderCommandEncoder(nil)
			{
				// Finish setup
				MTLSamplerDescriptor*	samplerDescriptor = [[MTLSamplerDescriptor alloc] init];
				mSamplerState = [procs.getDevice() newSamplerStateWithDescriptor:samplerDescriptor];

				MTLDepthStencilDescriptor*	depthStencilDescriptor = [[MTLDepthStencilDescriptor alloc] init];
				depthStencilDescriptor.depthCompareFunction = MTLCompareFunctionLessEqual;
				depthStencilDescriptor.depthWriteEnabled = NO;
				mDepthStencilState2D = [procs.getDevice() newDepthStencilStateWithDescriptor:depthStencilDescriptor];

				depthStencilDescriptor.depthWriteEnabled = YES;
				mDepthStencilState3D = [procs.getDevice() newDepthStencilStateWithDescriptor:depthStencilDescriptor];

				mMetalBufferCaches = [NSMutableArray arrayWithCapacity:kBufferCount];
				for (NSUInteger i = 0; i < kBufferCount; i++)
					[mMetalBufferCaches addObject:[[MetalBufferCache alloc] initWithDevice:mProcs.getDevice()]];

 				mRenderPipelineDescriptor.sampleCount = procs.getSampleCount();
				mRenderPipelineDescriptor.colorAttachments[0].pixelFormat = procs.getPixelFormat();
			}
		~CGPUInternals()
			{
				// Cleanup
				if (mMetalTextureCache.hasInstance())
					// Release
					::CFRelease(*mMetalTextureCache);
			}

	SGPUProcsInfo												mProcs;

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
	OI<CVMetalTextureCacheRef>									mMetalTextureCache;

	id<MTLCommandBuffer>										mCurrentCommandBuffer;
	id<MTLRenderCommandEncoder>									mCurrentRenderCommandEncoder;
	TNArray<const I<CGPUTexture> >								mPreviousTextures;
	TNArray<const I<CGPUTexture> >								mCurrentTextures;

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
CGPU::CGPU(const SGPUProcsInfo& procs)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CGPUInternals(procs);
}

//----------------------------------------------------------------------------------------------------------------------
CGPU::~CGPU()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CGPU methods

//----------------------------------------------------------------------------------------------------------------------
CVideoFrame::Compatibility CGPU::getVideoFrameCompatibility() const
//----------------------------------------------------------------------------------------------------------------------
{
	return CVideoFrame::kCompatibilityMetal;
}

//----------------------------------------------------------------------------------------------------------------------
I<CGPUTexture> CGPU::registerTexture(const CData& data, CGPUTexture::DataFormat dataFormat, const S2DSizeU16& size)
//----------------------------------------------------------------------------------------------------------------------
{
	return I<CGPUTexture>(new CMetalTexture(mInternals->mProcs.getDevice(), data, dataFormat, size));
}

//----------------------------------------------------------------------------------------------------------------------
TArray<I<CGPUTexture> > CGPU::registerTextures(const CVideoFrame& videoFrame)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup Metal Texture Cache
	if (!mInternals->mMetalTextureCache.hasInstance()) {
		// Create
		CVMetalTextureCacheRef	metalTextureCacheRef;
		CVReturn				result =
										::CVMetalTextureCacheCreate(kCFAllocatorDefault, nil,
												mInternals->mProcs.getDevice(), nil, &metalTextureCacheRef);
		if (result != kCVReturnSuccess) {
			// Error
			CLogServices::logError(CString(OSSTR("CMetalGPU - error when creating texture cache: ")) + CString(result));

			return TNArray<I<CGPUTexture> >();
		}

		// Store
		mInternals->mMetalTextureCache = OI<CVMetalTextureCacheRef>(metalTextureCacheRef);
	} else
		// Flush
		::CVMetalTextureCacheFlush(*mInternals->mMetalTextureCache, 0);

	// Load textures
	CVImageBufferRef			imageBufferRef = videoFrame.getImageBufferRef();
	UInt32						planeCount =
										::CVPixelBufferIsPlanar(imageBufferRef) ?
												(UInt32) ::CVPixelBufferGetPlaneCount(imageBufferRef) : 1;
	TNArray<I<CGPUTexture> >	textures;
	for (UInt32 i = 0; i < planeCount; i++)
		// Add texture
		textures += I<CGPUTexture>(new CMetalTexture(*mInternals->mMetalTextureCache, imageBufferRef, i));

	return textures;
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::unregisterTexture(I<CGPUTexture>& gpuTexture)
//----------------------------------------------------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------------------------------------------------
SGPUBuffer CGPU::allocateIndexBuffer(const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	// Create buffer
	id<MTLBuffer>	mtlBuffer =
							[mInternals->mProcs.getDevice() newBufferWithBytes:data.getBytePtr()
									length:data.getSize() options:MTLResourceStorageModeShared];

	return SGPUBuffer((void*) ::CFBridgingRetain(mtlBuffer));
}

//----------------------------------------------------------------------------------------------------------------------
SGPUVertexBuffer CGPU::allocateVertexBuffer(UInt32 perVertexByteCount, const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	// Create buffer
	id<MTLBuffer>	mtlBuffer =
							[mInternals->mProcs.getDevice() newBufferWithBytes:data.getBytePtr()
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
					renderCommandEncoderWithDescriptor:mInternals->mProcs.getCurrentRenderPassDescriptor()];
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

	// Manage textures
	//	Until unveiling video frame handling, this was unnecessary, but possibly because textures largely don't change
	//	frame to frame.  With video frame handling, it seems that we need to keep (video frame) textures around for an
	//	extra frame or the visuals on-screen seem to be affected by frame decoding going into textures not longer active
	//	but affecting what is seen on screen.  This is probably not the right solution, but I don't know anything better
	//	at this time.  (Stevo 4/30/2021).
	mInternals->mPreviousTextures = mInternals->mCurrentTextures;
	mInternals->mCurrentTextures.removeAll();
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::render(CGPURenderState& renderState, RenderType renderType, UInt32 count, UInt32 offset)
//----------------------------------------------------------------------------------------------------------------------
{
	// Finalize render state
	switch (renderState.getMode()) {
		case CGPURenderState::kMode2D:
			// 2D
			[mInternals->mCurrentRenderCommandEncoder setDepthStencilState:mInternals->mDepthStencilState2D];
			renderState.commit(
					SGPURenderStateCommitInfo(mInternals->mProcs.getDevice(), mInternals->mShaderLibrary,
							mInternals->mCurrentRenderCommandEncoder,
							mInternals->mMetalBufferCaches[mInternals->mMetalBufferCacheIndex],
							mInternals->mFunctionsCache, mInternals->mRenderPipelineDescriptor,
							mInternals->mRenderPipelineStateCache, mInternals->mViewMatrix2D,
							mInternals->mProjectionMatrix2D));
			break;

		case CGPURenderState::kMode3D:
			// 3D
			[mInternals->mCurrentRenderCommandEncoder setDepthStencilState:mInternals->mDepthStencilState3D];
			renderState.commit(
					SGPURenderStateCommitInfo(mInternals->mProcs.getDevice(), mInternals->mShaderLibrary,
							mInternals->mCurrentRenderCommandEncoder,
							mInternals->mMetalBufferCaches[mInternals->mMetalBufferCacheIndex],
							mInternals->mFunctionsCache, mInternals->mRenderPipelineDescriptor,
							mInternals->mRenderPipelineStateCache, mInternals->mViewMatrix3D,
							mInternals->mProjectionMatrix3D));
			break;
	}

	// Query textures
	const OR<const TArray<const I<CGPUTexture> > >	textures = renderState.getTextures();
	if (textures.hasReference())
		// Save a reference to the textures
		mInternals->mCurrentTextures += *textures;

	// Check type
	switch (renderType) {
		case kRenderTypeTriangleList:
			// Triangle list
			[mInternals->mCurrentRenderCommandEncoder pushDebugGroup:@"Triangle List"];
			AssertFailUnimplemented();
			[mInternals->mCurrentRenderCommandEncoder popDebugGroup];
			break;

		case kRenderTypeTriangleStrip:
			// Triangle strip
			[mInternals->mCurrentRenderCommandEncoder pushDebugGroup:@"Triangle Strip"];
			[mInternals->mCurrentRenderCommandEncoder drawPrimitives:MTLPrimitiveTypeTriangleStrip vertexStart:offset
					vertexCount:count];
			[mInternals->mCurrentRenderCommandEncoder popDebugGroup];
			break;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::renderIndexed(CGPURenderState& renderState, RenderType renderType, UInt32 count, UInt32 offset)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	const	OR<const SGPUBuffer>&	indexBuffer = renderState.getIndexBuffer();
			id<MTLBuffer>			mtlBuffer = (__bridge id<MTLBuffer>) indexBuffer->mPlatformReference;

	// Finalize render state
	switch (renderState.getMode()) {
		case CGPURenderState::kMode2D:
			// 2D
			[mInternals->mCurrentRenderCommandEncoder setDepthStencilState:mInternals->mDepthStencilState2D];
			renderState.commit(
					SGPURenderStateCommitInfo(mInternals->mProcs.getDevice(), mInternals->mShaderLibrary,
							mInternals->mCurrentRenderCommandEncoder,
							mInternals->mMetalBufferCaches[mInternals->mMetalBufferCacheIndex],
							mInternals->mFunctionsCache, mInternals->mRenderPipelineDescriptor,
							mInternals->mRenderPipelineStateCache, mInternals->mViewMatrix2D,
							mInternals->mProjectionMatrix2D));
			break;

		case CGPURenderState::kMode3D:
			// 3D
			[mInternals->mCurrentRenderCommandEncoder setDepthStencilState:mInternals->mDepthStencilState3D];
			renderState.commit(
					SGPURenderStateCommitInfo(mInternals->mProcs.getDevice(), mInternals->mShaderLibrary,
							mInternals->mCurrentRenderCommandEncoder,
							mInternals->mMetalBufferCaches[mInternals->mMetalBufferCacheIndex],
							mInternals->mFunctionsCache, mInternals->mRenderPipelineDescriptor,
							mInternals->mRenderPipelineStateCache, mInternals->mViewMatrix3D,
							mInternals->mProjectionMatrix3D));
			break;
	}

	// Query textures
	const OR<const TArray<const I<CGPUTexture> > >	textures = renderState.getTextures();
	if (textures.hasReference())
		// Save a reference to the textures
		mInternals->mCurrentTextures += *textures;

	// Check type
	switch (renderType) {
		case kRenderTypeTriangleList:
			// Triangle list
			[mInternals->mCurrentRenderCommandEncoder pushDebugGroup:@"Triangle List Indexed"];
			[mInternals->mCurrentRenderCommandEncoder drawIndexedPrimitives:MTLPrimitiveTypeTriangle indexCount:count
					indexType:MTLIndexTypeUInt16 indexBuffer:mtlBuffer indexBufferOffset:0];
			[mInternals->mCurrentRenderCommandEncoder popDebugGroup];
			break;

		case kRenderTypeTriangleStrip:
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
	// Setup for command completion
	[mInternals->mCurrentCommandBuffer addCompletedHandler:^(id<MTLCommandBuffer> commandBuffer) {
		// We're done
		mInternals->mSharedResourceBuffers.release();
	}];

	// Cleanup
	[mInternals->mCurrentRenderCommandEncoder endEncoding];
	mInternals->mCurrentRenderCommandEncoder = nil;

	[mInternals->mCurrentCommandBuffer presentDrawable:mInternals->mProcs.getCurrentDrawable()];
	[mInternals->mCurrentCommandBuffer commit];
	mInternals->mCurrentCommandBuffer = nil;

	// Next Metal Buffer Cache
	mInternals->mMetalBufferCacheIndex = (mInternals->mMetalBufferCacheIndex + 1) % kBufferCount;
}
