//----------------------------------------------------------------------------------------------------------------------
//	CDirectXRenderState.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CGPURenderState.h"

#include "CDirectXRenderState.h"
#include "CDirectXShader.h"
#include "CDirectXTexture.h"
#include "TBuffer.h"

#define Delete(x)	{ delete x; x = nil; }

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPURenderState::Internals

class CGPURenderState::Internals {
	public:
		Internals(CGPURenderState::Mode mode, CGPUVertexShader& vertexShader, CGPUFragmentShader& pixelShader) :
			mMode(mode), mVertexShader(vertexShader), mPixelShader(pixelShader)
			{}

		CGPURenderState::Mode						mMode;
		CGPUVertexShader&							mVertexShader;
		CGPUFragmentShader&							mPixelShader;

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
CGPURenderState::CGPURenderState(Mode mode, CGPUVertexShader& vertexShader, CGPUFragmentShader& pixelShader)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new Internals(mode, vertexShader, pixelShader);
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
void CGPURenderState::setTextures(const TArray<const I<CGPUTexture> >& gpuTextures)
//----------------------------------------------------------------------------------------------------------------------
{
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
	// Setup buffers
	if (mInternals->mVertexBuffer.hasReference()) {
		// Setup vertex buffer
		ID3D11Buffer*	buffer = (ID3D11Buffer*) mInternals->mVertexBuffer->mPlatformReference;
		UINT			stride = mInternals->mVertexBuffer->mPerVertexByteCount;
		UINT			offset = 0;
		commitInfo.mD3DDeviceContext.IASetVertexBuffers(0, 1, &buffer, &stride, &offset);
	}

	if (mInternals->mIndexBuffer.hasReference()) {
		// Setup index buffer
		ID3D11Buffer*	buffer = (ID3D11Buffer*) mInternals->mIndexBuffer->mPlatformReference;
		commitInfo.mD3DDeviceContext.IASetIndexBuffer(buffer, DXGI_FORMAT_R16_UINT, 0);
	}

	// Check for textures
	if (mInternals->mTextures.hasReference()) {
		// Setup textures
		const	TArray<const I<CGPUTexture> >&	gpuTextures = mInternals->mTextures.getReference();
				bool							needBlend = false;
				UINT							shaderResourceSlot = 0;
		for (CArray::ItemIndex i = 0; i < gpuTextures.getCount(); i++) {
			// Setup
			const	CDirectXTexture&						texture = (const CDirectXTexture&) *gpuTextures[i];
			const	TArray<CI<ID3D11ShaderResourceView> >&	shaderResourceViews = texture.getShaderResourceViews();

			// Setup this texture
			TBuffer<ID3D11ShaderResourceView*>	shaderResourceViewPtrs(shaderResourceViews.getCount());
			for (CArray::ItemIndex i = 0; i < shaderResourceViews.getCount(); i++)
				shaderResourceViewPtrs[i] = *(shaderResourceViews[i]);
			commitInfo.mD3DDeviceContext.PSSetShaderResources(shaderResourceSlot, shaderResourceViews.getCount(),
					*shaderResourceViewPtrs);

			// Update
			needBlend |= texture.hasTransparency();
			shaderResourceSlot += shaderResourceViews.getCount();
		}

		// Check if need blend
		float	blendFactor[] = {0.0, 0.0, 0.0, 0.0};
		if (needBlend)
			// Setup blend state
			commitInfo.mD3DDeviceContext.OMSetBlendState(&commitInfo.mD3DBlendState, blendFactor, 0xFFFFFFFF);
		else
			// No blend
			commitInfo.mD3DDeviceContext.OMSetBlendState(NULL, blendFactor, 0xFFFFFFFF);
	}

	// Setup shaders
	((CDirectXVertexShader&) mInternals->mVertexShader).setModelMatrix(mInternals->mModelMatrix);
	((CDirectXVertexShader&) mInternals->mVertexShader).setup(commitInfo.mD3DDevice, commitInfo.mD3DDeviceContext,
			commitInfo.mProjectionMatrix, commitInfo.mViewMatrix);

	((CDirectXPixelShader&) mInternals->mPixelShader).setup(commitInfo.mD3DDevice, commitInfo.mD3DDeviceContext);
}
