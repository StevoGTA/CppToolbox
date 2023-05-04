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
// MARK: CGPURenderState::Internals

class CGPURenderState::Internals {
	public:
		Internals(CGPURenderState::Mode mode, CGPUVertexShader& vertexShader, CGPUFragmentShader& fragmentShader) :
			mMode(mode), mVertexShader(vertexShader), mFragmentShader(fragmentShader)
			{}

		CGPURenderState::Mode						mMode;
		CGPUVertexShader&							mVertexShader;
		CGPUFragmentShader&							mFragmentShader;

		SMatrix4x4_32								mModelMatrix;

		OR<const SGPUVertexBuffer>					mVertexBuffer;
		OR<const SGPUBuffer>						mIndexBuffer;
		OR<const TArray<const I<CGPUTexture> > >	mTextures;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPURenderState

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CGPURenderState::CGPURenderState(Mode mode, CGPUVertexShader& vertexShader, CGPUFragmentShader& fragmentShader)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new Internals(mode, vertexShader, fragmentShader);
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
void CGPURenderState::setTextures(const TArray<const I<CGPUTexture> >& gpuTextures)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mTextures = OR<const TArray<const I<CGPUTexture> > >(gpuTextures);
}

//----------------------------------------------------------------------------------------------------------------------
const OR<const TArray<const I<CGPUTexture> > > CGPURenderState::getTextures() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mTextures;
}

//----------------------------------------------------------------------------------------------------------------------
CGPURenderState::Mode CGPURenderState::getMode() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mMode;
}

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderState::commit(const CommitInfo& commitInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CMetalVertexShader&		vertexShader = (CMetalVertexShader&) mInternals->mVertexShader;
	CMetalFragmentShader&	fragmentShader = (CMetalFragmentShader&) mInternals->mFragmentShader;

	// Setup globals
	GlobalUniforms	globalUniforms;
	::memcpy(&globalUniforms.mModelMatrix, &mInternals->mModelMatrix, sizeof(matrix_float4x4));
	::memcpy(&globalUniforms.mViewMatrix, &commitInfo.mViewMatrix, sizeof(matrix_float4x4));
	::memcpy(&globalUniforms.mProjectionMatrix, &commitInfo.mProjectionMatrix, sizeof(matrix_float4x4));
	[commitInfo.mRenderCommandEncoder setVertexBytes:&globalUniforms length:sizeof(GlobalUniforms)
			atIndex:kBufferIndexGlobalUniforms];

	// Check for textures
	bool	needBlend = false;
	if (mInternals->mTextures.hasReference()) {
		// Setup textures
		const	TArray<const I<CGPUTexture> >&	gpuTextures = mInternals->mTextures.getReference();
		for (CArray::ItemIndex i = 0; i < gpuTextures.getCount(); i++) {
			// Setup
			const	CMetalTexture&	metalTexture = (const CMetalTexture&) *gpuTextures[i];

			// Setup this texture
			[commitInfo.mRenderCommandEncoder setFragmentTexture:metalTexture.getMetalTexture() atIndex:i];
			needBlend |= metalTexture.hasTransparency();
		}
	}

	// Setup functions
	NSString*		vertexShaderName = (__bridge NSString*) vertexShader.getName().getOSString();
	id<MTLFunction>	vertexFunction = commitInfo.mFunctionsCache[vertexShaderName];
	if (vertexFunction == nil) {
		// Create and store
		vertexFunction = [commitInfo.mShaderLibrary newFunctionWithName:vertexShaderName];
		commitInfo.mFunctionsCache[vertexShaderName] = vertexFunction;
	}

	NSString*		fragmentShaderName = (__bridge NSString*) fragmentShader.getName().getOSString();
	id<MTLFunction>	fragmentFunction = commitInfo.mFunctionsCache[fragmentShaderName];
	if (fragmentFunction == nil) {
		// Create and store
		fragmentFunction = [commitInfo.mShaderLibrary newFunctionWithName:fragmentShaderName];
		commitInfo.mFunctionsCache[fragmentShaderName] = fragmentFunction;
	}

	// Setup render pipeline descriptor
	bool		requiresDepthTest = vertexShader.requiresDepthTest();
	UInt32		options =
						(needBlend ?			1 << 0 : 0) +
						(requiresDepthTest ?	1 << 1 : 0);
	NSString*	key = [NSString stringWithFormat:@"%@/%@/%u", vertexFunction.name, fragmentFunction.name, options];
	id<MTLRenderPipelineState>	renderPipelineState = commitInfo.mRenderPipelineStateCache[key];
	if (renderPipelineState == nil) {
		// Create render pipeline state
		commitInfo.mRenderPipelineDescriptor.label = @"Pipeline";
		commitInfo.mRenderPipelineDescriptor.vertexFunction = vertexFunction;
		commitInfo.mRenderPipelineDescriptor.fragmentFunction = fragmentFunction;
		commitInfo.mRenderPipelineDescriptor.vertexDescriptor = vertexShader.getVertexDescriptor();
		commitInfo.mRenderPipelineDescriptor.depthAttachmentPixelFormat = MTLPixelFormatDepth32Float;

		commitInfo.mRenderPipelineDescriptor.colorAttachments[0].blendingEnabled = needBlend;
		commitInfo.mRenderPipelineDescriptor.colorAttachments[0].sourceRGBBlendFactor = MTLBlendFactorSourceAlpha;
		commitInfo.mRenderPipelineDescriptor.colorAttachments[0].destinationRGBBlendFactor =
				MTLBlendFactorOneMinusSourceAlpha;

		NSError*	error;
		renderPipelineState =
				[commitInfo.mDevice newRenderPipelineStateWithDescriptor:commitInfo.mRenderPipelineDescriptor
						error:&error];
		if (renderPipelineState == nil)
			// Error
			NSLog(@"Failed to create pipeline state with error %@", error);

		// Store
		commitInfo.mRenderPipelineStateCache[key] = renderPipelineState;
	}

	// Setup render command encoder
	id<MTLBuffer>	mtlBuffer = (__bridge id<MTLBuffer>) mInternals->mVertexBuffer->mPlatformReference;
	[commitInfo.mRenderCommandEncoder setRenderPipelineState:renderPipelineState];

	vertexShader.setModelMatrix(mInternals->mModelMatrix);
	vertexShader.setup(commitInfo.mRenderCommandEncoder, mtlBuffer, commitInfo.mMetalBufferCache);

	fragmentShader.setup(commitInfo.mRenderCommandEncoder, commitInfo.mMetalBufferCache);
}
