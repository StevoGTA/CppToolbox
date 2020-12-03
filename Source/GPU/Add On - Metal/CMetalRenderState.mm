//----------------------------------------------------------------------------------------------------------------------
//	CMetalRenderState.mm			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#import "CGPURenderState.h"

#import "CMetalRenderState.h"
#import "CMetalShader.h"
#import "CMetalTexture.h"
#import "MetalShaderTypes.h"

#import <simd/simd.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPURenderStateInternals

class CGPURenderStateInternals {
	public:
		CGPURenderStateInternals(EGPURenderMode renderMode, CGPUVertexShader& vertexShader,
				CGPUFragmentShader& fragmentShader) :
			mRenderMode(renderMode), mVertexShader(vertexShader), mFragmentShader(fragmentShader)
			{}

		EGPURenderMode							mRenderMode;
		CGPUVertexShader&						mVertexShader;
		CGPUFragmentShader&						mFragmentShader;

		SMatrix4x4_32							mModelMatrix;

		OR<const SGPUVertexBuffer>				mVertexBuffer;
		OR<const SGPUBuffer>					mIndexBuffer;
		OR<const TArray<const CGPUTexture> >	mTextures;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPURenderState

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CGPURenderState::CGPURenderState(EGPURenderMode renderMode, CGPUVertexShader& vertexShader,
		CGPUFragmentShader& fragmentShader)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new CGPURenderStateInternals(renderMode, vertexShader, fragmentShader);
}

//----------------------------------------------------------------------------------------------------------------------
CGPURenderState::~CGPURenderState()
//----------------------------------------------------------------------------------------------------------------------
{
	// Cleanup
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderState::setModelMatrix(const SMatrix4x4_32& modelMatrix)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mModelMatrix = modelMatrix;
}

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderState::setVertexBuffer(const SGPUVertexBuffer& gpuVertexBuffer)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mVertexBuffer = OR<const SGPUVertexBuffer>(gpuVertexBuffer);
}

//----------------------------------------------------------------------------------------------------------------------
const OR<const SGPUBuffer>& CGPURenderState::getIndexBuffer() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mIndexBuffer;
}

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderState::setIndexBuffer(const SGPUBuffer& gpuIndexBuffer)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mIndexBuffer = OR<const SGPUBuffer>(gpuIndexBuffer);
}

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderState::setTextures(const TArray<const CGPUTexture>& gpuTextures)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mTextures = OR<const TArray<const CGPUTexture> >(gpuTextures);
}

//----------------------------------------------------------------------------------------------------------------------
EGPURenderMode CGPURenderState::getRenderMode() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mRenderMode;
}

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderState::commit(const SGPURenderStateCommitInfo& renderStateCommitInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CMetalVertexShader&		vertexShader = (CMetalVertexShader&) mInternals->mVertexShader;
	CMetalFragmentShader&	fragmentShader = (CMetalFragmentShader&) mInternals->mFragmentShader;

	// Setup globals
	GlobalUniforms	globalUniforms;
	::memcpy(&globalUniforms.mModelMatrix, &mInternals->mModelMatrix, sizeof(matrix_float4x4));
	::memcpy(&globalUniforms.mViewMatrix, &renderStateCommitInfo.mViewMatrix, sizeof(matrix_float4x4));
	::memcpy(&globalUniforms.mProjectionMatrix, &renderStateCommitInfo.mProjectionMatrix, sizeof(matrix_float4x4));
	[renderStateCommitInfo.mRenderCommandEncoder setVertexBytes:&globalUniforms length:sizeof(GlobalUniforms)
			atIndex:kBufferIndexGlobalUniforms];

	// Check for textures
	bool	needBlend = false;
	if (mInternals->mTextures.hasReference()) {
		// Setup textures
		const	TArray<const CGPUTexture>&	gpuTextures = mInternals->mTextures.getReference();
		for (CArray::ItemIndex i = 0; i < gpuTextures.getCount(); i++) {
			// Setup
			const	CMetalTexture&	metalTexture = (const CMetalTexture&) gpuTextures[i];

			// Setup this texture
			[renderStateCommitInfo.mRenderCommandEncoder setFragmentTexture:metalTexture.getMetalTexture() atIndex:i];
			needBlend |= metalTexture.hasTransparency();
		}
	}

	// Setup functions
	NSString*		vertexShaderName = (__bridge NSString*) vertexShader.getName().getOSString();
	id<MTLFunction>	vertexFunction = renderStateCommitInfo.mFunctionsCache[vertexShaderName];
	if (vertexFunction == nil) {
		// Create and store
		vertexFunction = [renderStateCommitInfo.mShaderLibrary newFunctionWithName:vertexShaderName];
		renderStateCommitInfo.mFunctionsCache[vertexShaderName] = vertexFunction;
	}

	NSString*		fragmentShaderName = (__bridge NSString*) fragmentShader.getName().getOSString();
	id<MTLFunction>	fragmentFunction = renderStateCommitInfo.mFunctionsCache[fragmentShaderName];
	if (fragmentFunction == nil) {
		// Create and store
		fragmentFunction = [renderStateCommitInfo.mShaderLibrary newFunctionWithName:fragmentShaderName];
		renderStateCommitInfo.mFunctionsCache[fragmentShaderName] = fragmentFunction;
	}

	// Setup render pipeline descriptor
	bool		requiresDepthTest = vertexShader.requiresDepthTest();
	UInt32		options =
						(needBlend ?			1 << 0 : 0) +
						(requiresDepthTest ?	1 << 1 : 0);
	NSString*	key = [NSString stringWithFormat:@"%@/%@/%u", vertexFunction.name, fragmentFunction.name, options];
	id<MTLRenderPipelineState>	renderPipelineState = renderStateCommitInfo.mRenderPipelineStateCache[key];
	if (renderPipelineState == nil) {
		// Create render pipeline state
		renderStateCommitInfo.mRenderPipelineDescriptor.label = @"Pipeline";
		renderStateCommitInfo.mRenderPipelineDescriptor.vertexFunction = vertexFunction;
		renderStateCommitInfo.mRenderPipelineDescriptor.fragmentFunction = fragmentFunction;
		renderStateCommitInfo.mRenderPipelineDescriptor.vertexDescriptor = vertexShader.getVertexDescriptor();
		renderStateCommitInfo.mRenderPipelineDescriptor.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;

		renderStateCommitInfo.mRenderPipelineDescriptor.colorAttachments[0].blendingEnabled = needBlend;
		renderStateCommitInfo.mRenderPipelineDescriptor.colorAttachments[0].sourceRGBBlendFactor =
				MTLBlendFactorSourceAlpha;
		renderStateCommitInfo.mRenderPipelineDescriptor.colorAttachments[0].destinationRGBBlendFactor =
				MTLBlendFactorOneMinusSourceAlpha;

		NSError*	error;
		renderPipelineState =
				[renderStateCommitInfo.mDevice
						newRenderPipelineStateWithDescriptor:renderStateCommitInfo.mRenderPipelineDescriptor
						error:&error];
		if (renderPipelineState == nil)
			// Error
			NSLog(@"Failed to create pipeline state with error %@", error);

		// Store
		renderStateCommitInfo.mRenderPipelineStateCache[key] = renderPipelineState;
	}

	// Setup render command encoder
	id<MTLBuffer>	mtlBuffer = (__bridge id<MTLBuffer>) mInternals->mVertexBuffer->mPlatformReference;
	[renderStateCommitInfo.mRenderCommandEncoder setRenderPipelineState:renderPipelineState];

	vertexShader.setModelMatrix(mInternals->mModelMatrix);
	vertexShader.setup(renderStateCommitInfo.mRenderCommandEncoder, mtlBuffer, renderStateCommitInfo.mMetalBufferCache);

	fragmentShader.setup(renderStateCommitInfo.mRenderCommandEncoder, renderStateCommitInfo.mMetalBufferCache);
}
