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
		CGPURenderStateInternals(CGPUVertexShader& vertexShader, CGPUFragmentShader& fragmentShader) :
			mVertexShader(vertexShader), mFragmentShader(fragmentShader), mTriangleOffset(0)
			{}

		CGPUVertexShader&						mVertexShader;
		CGPUFragmentShader&						mFragmentShader;

		SMatrix4x4_32							mModelMatrix;

		OR<const SGPUVertexBuffer>				mVertexBuffer;
		UInt32									mTriangleOffset;
		OR<const TArray<const CGPUTexture> >	mTextures;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPURenderState

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CGPURenderState::CGPURenderState(CGPUVertexShader& vertexShader, CGPUFragmentShader& fragmentShader)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new CGPURenderStateInternals(vertexShader, fragmentShader);
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
void CGPURenderState::setProjectionMatrix(const SMatrix4x4_32& projectionMatrix)
//----------------------------------------------------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderState::setViewMatrix(const SMatrix4x4_32& viewMatrix)
//----------------------------------------------------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderState::setModelMatrix(const SMatrix4x4_32& modelMatrix)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mModelMatrix = modelMatrix;
}

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderState::setVertexTextureInfo(const SGPUVertexBuffer& gpuVertexBuffer, UInt32 triangleOffset,
		const TArray<const CGPUTexture>& gpuTextures)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mVertexBuffer = OR<const SGPUVertexBuffer>(gpuVertexBuffer);
	mInternals->mTriangleOffset = triangleOffset;
	mInternals->mTextures = OR<const TArray<const CGPUTexture> >(gpuTextures);
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CGPURenderState::getTriangleOffset() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mTriangleOffset;
}

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderState::commit(const SGPURenderStateCommitInfo& renderStateCommitInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup textures
			bool						needBlend = false;
	const	TArray<const CGPUTexture>&	gpuTextures = mInternals->mTextures.getReference();
	for (CArrayItemIndex i = 0; i < gpuTextures.getCount(); i++) {
		// Setup
		const	CMetalTexture&	metalTexture = (const CMetalTexture&) gpuTextures[i];

		// Setup this texture
		[renderStateCommitInfo.mRenderCommandEncoder setFragmentTexture:metalTexture.getMetalTexture() atIndex:i];
		needBlend |= metalTexture.hasTransparency();
	}

	// Setup functions
	NSString*		vertexShaderName =
							(__bridge NSString*)
									((CMetalVertexShader&) mInternals->mVertexShader).getName().getOSString();
	id<MTLFunction>	vertexFunction = renderStateCommitInfo.mFunctionsCache[vertexShaderName];
	if (vertexFunction == nil) {
		// Create and store
		vertexFunction = [renderStateCommitInfo.mShaderLibrary newFunctionWithName:vertexShaderName];
		renderStateCommitInfo.mFunctionsCache[vertexShaderName] = vertexFunction;
	}

	NSString*		fragmentShaderName =
							(__bridge NSString*)
									((CMetalFragmentShader&) mInternals->mFragmentShader).getName().getOSString();
	id<MTLFunction>	fragmentFunction = renderStateCommitInfo.mFunctionsCache[fragmentShaderName];
	if (fragmentFunction == nil) {
		// Create and store
		fragmentFunction = [renderStateCommitInfo.mShaderLibrary newFunctionWithName:fragmentShaderName];
		renderStateCommitInfo.mFunctionsCache[fragmentShaderName] = fragmentFunction;
	}

	// Setup render pipeline descriptor
	SMetalVertexBufferInfo*	metalVertexBufferInfo =
									(SMetalVertexBufferInfo*) mInternals->mVertexBuffer->mInternalReference;

	NSString*	key =
						[NSString stringWithFormat:@"%@/%@/%@/%u",
								vertexFunction.name, fragmentFunction.name,
								metalVertexBufferInfo->mVertexDescriptorUUID, needBlend];
	id<MTLRenderPipelineState>	renderPipelineState = renderStateCommitInfo.mRenderPipelineStateCache[key];
	if (renderPipelineState == nil) {
		// Create render pipeline state
		renderStateCommitInfo.mRenderPipelineDescriptor.label = @"Pipeline";
		renderStateCommitInfo.mRenderPipelineDescriptor.vertexFunction = vertexFunction;
		renderStateCommitInfo.mRenderPipelineDescriptor.fragmentFunction = fragmentFunction;
		renderStateCommitInfo.mRenderPipelineDescriptor.vertexDescriptor = metalVertexBufferInfo->mVertexDescriptor;

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
	[renderStateCommitInfo.mRenderCommandEncoder setRenderPipelineState:renderPipelineState];
	[renderStateCommitInfo.mRenderCommandEncoder setVertexBuffer:metalVertexBufferInfo->mVertexBuffer offset:0
			atIndex:kBufferIndexVertexPosition];
	[renderStateCommitInfo.mRenderCommandEncoder setVertexBuffer:metalVertexBufferInfo->mVertexBuffer offset:0
			atIndex:kBufferIndexVertexTextureCoordinate];
	[renderStateCommitInfo.mRenderCommandEncoder setVertexBuffer:metalVertexBufferInfo->mVertexBuffer offset:0
			atIndex:kBufferIndexVertexTextureIndex];

	((CMetalVertexShader&) mInternals->mVertexShader).setModelMatrix(mInternals->mModelMatrix);
	((CMetalVertexShader&) mInternals->mVertexShader).setup(renderStateCommitInfo.mRenderCommandEncoder,
			renderStateCommitInfo.mMetalBufferCache);

	((CMetalFragmentShader&) mInternals->mFragmentShader).setup(renderStateCommitInfo.mRenderCommandEncoder,
			renderStateCommitInfo.mMetalBufferCache);
}
