//----------------------------------------------------------------------------------------------------------------------
//	CDirectXRenderState.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CGPURenderState.h"

#include "CDirectXRenderState.h"
#include "CDirectXShader.h"
#include "CDirectXTexture.h"

#define Delete(x)	{ delete x; x = nil; }

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPURenderStateInternals

class CGPURenderStateInternals {
	public:
		CGPURenderStateInternals(CGPUVertexShader& vertexShader, CGPUFragmentShader& pixelShader) :
			mVertexShader(vertexShader), mPixelShader(pixelShader), mTriangleOffset(0)
			{}

		CGPUVertexShader&					mVertexShader;
		CGPUFragmentShader&					mPixelShader;

		SMatrix4x4_32						mModelMatrix;

		OR<const SGPUVertexBuffer>			mVertexBuffer;
		UInt32								mTriangleOffset;
		OR<const TArray<const CGPUTexture>>	mTextures;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPURenderState

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CGPURenderState::CGPURenderState(CGPUVertexShader& vertexShader, CGPUFragmentShader& pixelShader)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new CGPURenderStateInternals(vertexShader, pixelShader);
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
void CGPURenderState::setVertexTextureInfo(const SGPUVertexBuffer& gpuVertexBuffer, UInt32 triangleOffset,
		const TArray<const CGPUTexture>& gpuTextures)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mVertexBuffer = OR<const SGPUVertexBuffer>(gpuVertexBuffer);
	mInternals->mTriangleOffset = triangleOffset;
	mInternals->mTextures = OR<const TArray<const CGPUTexture>>(gpuTextures);
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
	// Setup buffers
	ID3D11Buffer*	buffer = (ID3D11Buffer*) mInternals->mVertexBuffer->mInternalReference;
	UINT			stride = mInternals->mVertexBuffer->mGPUVertexBufferInfo.mTotalSize;
	UINT			offset = 0;
	renderStateCommitInfo.mD3DDeviceContext.IASetVertexBuffers(0, 1, &buffer, &stride, &offset);

	// Setup textures
			bool						needBlend = false;
	const	TArray<const CGPUTexture>&	gpuTextures = mInternals->mTextures.getReference();
	for (CArrayItemIndex i = 0; i < gpuTextures.getCount(); i++) {
		// Setup
		const	CDirectXTexture&	texture = (const CDirectXTexture&) gpuTextures[i];

		// Setup this texture
		ID3D11ShaderResourceView*	shaderResourceView = texture.getShaderResourceView();
		renderStateCommitInfo.mD3DDeviceContext.PSSetShaderResources(0, 1, &shaderResourceView);
		needBlend |= texture.hasTransparency();
	}

	// Setup shaders
	((CDirectXVertexShader&) mInternals->mVertexShader).setModelMatrix(mInternals->mModelMatrix);
	((CDirectXVertexShader&) mInternals->mVertexShader).setup(renderStateCommitInfo.mD3DDevice,
			renderStateCommitInfo.mD3DDeviceContext, renderStateCommitInfo.mProjectionMatrix,
			renderStateCommitInfo.mViewMatrix);

	((CDirectXPixelShader&) mInternals->mPixelShader).setup(renderStateCommitInfo.mD3DDevice,
			renderStateCommitInfo.mD3DDeviceContext);
}
