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
		CGPURenderStateInternals(EGPURenderMode renderMode, CGPUVertexShader& vertexShader,
				CGPUFragmentShader& pixelShader) :
			mRenderMode(renderMode), mVertexShader(vertexShader), mPixelShader(pixelShader)
			{}

		EGPURenderMode						mRenderMode;
		CGPUVertexShader&					mVertexShader;
		CGPUFragmentShader&					mPixelShader;

		SMatrix4x4_32						mModelMatrix;

		OR<const SGPUVertexBuffer>			mVertexBuffer;
		OR<const SGPUBuffer>				mIndexBuffer;
		OR<const TArray<const CGPUTexture>>	mTextures;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPURenderState

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CGPURenderState::CGPURenderState(EGPURenderMode renderMode, CGPUVertexShader& vertexShader,
		CGPUFragmentShader& pixelShader)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new CGPURenderStateInternals(renderMode, vertexShader, pixelShader);
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
	mInternals->mIndexBuffer = OR<const SGPUBuffer>(gpuIndexBuffer);
}

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderState::setTextures(const TArray<const CGPUTexture>& gpuTextures)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mTextures = OR<const TArray<const CGPUTexture>>(gpuTextures);
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
	// Setup buffers
	if (mInternals->mVertexBuffer.hasReference()) {
		// Setup vertex buffer
		ID3D11Buffer*	buffer = (ID3D11Buffer*) mInternals->mVertexBuffer->mPlatformReference;
		UINT			stride = mInternals->mVertexBuffer->mPerVertexByteCount;
		UINT			offset = 0;
		renderStateCommitInfo.mD3DDeviceContext.IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
	}

	if (mInternals->mIndexBuffer.hasReference()) {
		// Setup index buffer
		ID3D11Buffer*	buffer = (ID3D11Buffer*) mInternals->mIndexBuffer->mPlatformReference;
		renderStateCommitInfo.mD3DDeviceContext.IASetIndexBuffer(buffer, DXGI_FORMAT_R16_UINT, 0);
	}

	// Check for textures
	if (mInternals->mTextures.hasReference()) {
		// Setup textures
				bool						needBlend = false;
		const	TArray<const CGPUTexture>&	gpuTextures = mInternals->mTextures.getReference();
		for (CArray::ItemIndex i = 0; i < gpuTextures.getCount(); i++) {
			// Setup
			const	CDirectXTexture&	texture = (const CDirectXTexture&) gpuTextures[i];

			// Setup this texture
			ID3D11ShaderResourceView*	shaderResourceView = texture.getShaderResourceView();
			renderStateCommitInfo.mD3DDeviceContext.PSSetShaderResources(i, 1, &shaderResourceView);
			needBlend |= texture.hasTransparency();
		}

		// Check if need blend
		float	blendFactor[] = {0.0, 0.0, 0.0, 0.0};
		if (needBlend)
			// Setup blend state
			renderStateCommitInfo.mD3DDeviceContext.OMSetBlendState(&renderStateCommitInfo.mD3DBlendState, blendFactor,
					0xFFFFFFFF);
		else
			// No blend
			renderStateCommitInfo.mD3DDeviceContext.OMSetBlendState(NULL, blendFactor, 0xFFFFFFFF);
	}

	// Setup shaders
	((CDirectXVertexShader&) mInternals->mVertexShader).setModelMatrix(mInternals->mModelMatrix);
	((CDirectXVertexShader&) mInternals->mVertexShader).setup(renderStateCommitInfo.mD3DDevice,
			renderStateCommitInfo.mD3DDeviceContext, renderStateCommitInfo.mProjectionMatrix,
			renderStateCommitInfo.mViewMatrix);

	((CDirectXPixelShader&) mInternals->mPixelShader).setup(renderStateCommitInfo.mD3DDevice,
			renderStateCommitInfo.mD3DDeviceContext);
}
