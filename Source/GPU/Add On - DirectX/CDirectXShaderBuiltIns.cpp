//----------------------------------------------------------------------------------------------------------------------
//	CDirectXShaderBuiltIns.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CDirectXShader.h"

#include "CDirectXGPU.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local proc declarations

static	CGPUFragmentShader&	sYCbCrFragmentShader(Float32 opacity);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDirectXVertexShaderBasic

class CDirectXVertexShaderBasic : public CDirectXVertexShader {
	private:
		struct SConstants {
			XMFLOAT4X4	mProjectionMatrix;
			XMFLOAT4X4	mViewMatrix;
			XMFLOAT4X4	mModelMatrix;
		};

	public:
				CDirectXVertexShaderBasic() :
					CDirectXVertexShader(CFilesystemPath(CString(OSSTR("VertexShaderBasic.cso")))),
							mInputLayout(NULL), mConstantBuffer(NULL)
					{}
				~CDirectXVertexShaderBasic()
					{
						// Cleanup
						if (mInputLayout != NULL)
							mInputLayout->Release();
						if (mConstantBuffer != NULL)
							mConstantBuffer->Release();
					}

		UInt32	getPerVertexByteCount() const
					{ return sizeof(SVertex2DMultitexture); }

		void	setup(ID3D11Device& d3dDevice, ID3D11DeviceContext3& d3dDeviceContext,
						const XMFLOAT4X4& projectionMatrix, const XMFLOAT4X4& viewMatrix)
					{
						// Do super
						CDirectXVertexShader::setup(d3dDevice, d3dDeviceContext, projectionMatrix, viewMatrix);

						// Set input layout
						d3dDeviceContext.IASetInputLayout(mInputLayout);

						// Update constants
						SConstants	constants;
						constants.mProjectionMatrix = projectionMatrix;
						constants.mViewMatrix = viewMatrix;
						constants.mModelMatrix = getModelMatrix();

						d3dDeviceContext.UpdateSubresource1(mConstantBuffer, 0, NULL, &constants, 0, 0, 0);
						d3dDeviceContext.VSSetConstantBuffers1(0, 1, &mConstantBuffer, NULL, NULL);
					}

		void	createResources(ID3D11Device& d3dDevice, const CData& shaderData)
					{
						// Create input layout
						D3D11_INPUT_ELEMENT_DESC	vertexDesc [] =
															{
																{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0,
																		D3D11_INPUT_PER_VERTEX_DATA, 0},
																{"TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 8,
																		D3D11_INPUT_PER_VERTEX_DATA, 0},
															};
						HRESULT	result =
										d3dDevice.CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc),
												shaderData.getBytePtr(), (SIZE_T) shaderData.getByteCount(),
												&mInputLayout);
						AssertFailIf(FAILED(result));

						// Create constant buffer
						CD3D11_BUFFER_DESC	constantBufferDesc(sizeof(SConstants), D3D11_BIND_CONSTANT_BUFFER);
						AssertFailIf(FAILED(d3dDevice.CreateBuffer(&constantBufferDesc, NULL, &mConstantBuffer)));
					}

		ID3D11InputLayout*	mInputLayout;
		ID3D11Buffer*		mConstantBuffer;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDirectXVertexShaderClip

class CDirectXVertexShaderClip : public CDirectXVertexShader {
	private:
		struct SConstants {
			XMFLOAT4X4	mProjectionMatrix;
			XMFLOAT4X4	mViewMatrix;
			XMFLOAT4X4	mModelMatrix;
			XMFLOAT4	mClipPlane;
		};

	public:
				CDirectXVertexShaderClip() :
					CDirectXVertexShader(CFilesystemPath(CString(OSSTR("VertexShaderClip.cso")))),
							mInputLayout(NULL), mConstantBuffer(NULL)
					{}
				~CDirectXVertexShaderClip()
					{
						// Cleanup
						if (mInputLayout != NULL)
							mInputLayout->Release();
						if (mConstantBuffer != NULL)
							mConstantBuffer->Release();
					}

		UInt32	getPerVertexByteCount() const
					{ return sizeof(SVertex2DMultitexture); }

		void	setup(ID3D11Device& d3dDevice, ID3D11DeviceContext3& d3dDeviceContext,
						const XMFLOAT4X4& projectionMatrix, const XMFLOAT4X4& viewMatrix)
					{
						// Do super
						CDirectXVertexShader::setup(d3dDevice, d3dDeviceContext, projectionMatrix, viewMatrix);

						// Set input layout
						d3dDeviceContext.IASetInputLayout(mInputLayout);

						// Update constants
						mConstants.mProjectionMatrix = projectionMatrix;
						mConstants.mViewMatrix = viewMatrix;
						mConstants.mModelMatrix = getModelMatrix();

						d3dDeviceContext.UpdateSubresource1(mConstantBuffer, 0, NULL, &mConstants, 0, 0, 0);
						d3dDeviceContext.VSSetConstantBuffers1(0, 1, &mConstantBuffer, NULL, NULL);
					}

		void	createResources(ID3D11Device& d3dDevice, const CData& shaderData)
					{
						// Create input layout
						D3D11_INPUT_ELEMENT_DESC	vertexDesc [] =
															{
																{"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0,
																		D3D11_INPUT_PER_VERTEX_DATA, 0},
																{"TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 8,
																		D3D11_INPUT_PER_VERTEX_DATA, 0},
															};
						HRESULT	result =
										d3dDevice.CreateInputLayout(vertexDesc, ARRAYSIZE(vertexDesc),
												shaderData.getBytePtr(), (SIZE_T) shaderData.getByteCount(),
												&mInputLayout);
						AssertFailIf(FAILED(result));

						// Create constant buffer
						CD3D11_BUFFER_DESC	constantBufferDesc(sizeof(SConstants), D3D11_BIND_CONSTANT_BUFFER);
						AssertFailIf(FAILED(d3dDevice.CreateBuffer(&constantBufferDesc, NULL, &mConstantBuffer)));
					}

		void	setClipPlane(const SMatrix4x1_32& clipPlane)
					{ mConstants.mClipPlane = *((XMFLOAT4*) &clipPlane); }

		ID3D11InputLayout*	mInputLayout;
		ID3D11Buffer*		mConstantBuffer;

		SConstants			mConstants;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUVertexShader

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CGPUVertexShader& CGPUVertexShader::getBasic2DMultiTexture()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	CDirectXVertexShaderBasic* sVertexShader = NULL;

	// Check if have shader
	if (sVertexShader == NULL)
		// Create shader
		sVertexShader = new CDirectXVertexShaderBasic();

	return *sVertexShader;
}

//----------------------------------------------------------------------------------------------------------------------
CGPUVertexShader& CGPUVertexShader::getClip2DMultiTexture(const SMatrix4x1_32& clipPlane)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	CDirectXVertexShaderClip* sVertexShader = NULL;

	// Check if have shader
	if (sVertexShader == NULL)
		// Create shader
		sVertexShader = new CDirectXVertexShaderClip();

	// Setup
	sVertexShader->setClipPlane(clipPlane);

	return *sVertexShader;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDirectXPixelShaderBasic

class CDirectXPixelShaderBasic : public CDirectXPixelShader {
	public:
				CDirectXPixelShaderBasic() :
					CDirectXPixelShader(CFilesystemPath(CString(OSSTR("PixelShaderBasic.cso"))))
					{}

		void	setup(ID3D11Device& d3dDevice, ID3D11DeviceContext3& d3dDeviceContext)
					{
						// Do super
						CDirectXPixelShader::setup(d3dDevice, d3dDeviceContext);
					}
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDirectXPixelShaderRGBAMultiTexture

class CDirectXPixelShaderRGBAMultiTexture : public CDirectXPixelShader {
	private:
		struct SConstants {
			// Lifecycle methods
			SConstants(Float32 opacity) : mOpacity(opacity) {}

			// Values
			Float32		mOpacity;
			XMFLOAT3	mPadding;
		};

	public:
				CDirectXPixelShaderRGBAMultiTexture() :
					CDirectXPixelShader(CFilesystemPath(CString(OSSTR("PixelShaderRGBAMultiTexture.cso")))),
							mConstantBuffer(NULL), mConstants(1.0)
					{}
				~CDirectXPixelShaderRGBAMultiTexture()
					{
						// Cleanup
						if (mConstantBuffer != NULL)
							mConstantBuffer->Release();
					}

		void	setup(ID3D11Device& d3dDevice, ID3D11DeviceContext3& d3dDeviceContext)
					{
						// Do super
						CDirectXPixelShader::setup(d3dDevice, d3dDeviceContext);

						// Update constants
						d3dDeviceContext.UpdateSubresource1(mConstantBuffer, 0, NULL, &mConstants, 0, 0, 0);
						d3dDeviceContext.PSSetConstantBuffers1(0, 1, &mConstantBuffer, NULL, NULL);
					}

		void	createResources(ID3D11Device& d3dDevice, const CData& shaderData)
					{
						// Create constant buffer
						CD3D11_BUFFER_DESC	constantBufferDesc(sizeof(SConstants), D3D11_BIND_CONSTANT_BUFFER);
						AssertFailIf(FAILED(d3dDevice.CreateBuffer(&constantBufferDesc, NULL, &mConstantBuffer)));
					}

		void	setOpacity(Float32 opacity)
					{ mConstants.mOpacity = opacity; }

		ID3D11Buffer*	mConstantBuffer;
		SConstants		mConstants;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDirectXFragmentShaderYCbCr

class CDirectXFragmentShaderYCbCr : public CDirectXPixelShader {
	private:
		struct SConstants {
			// Lifecycle methods
			SConstants(Float32 opacity) : mOpacity(opacity) {}

			// Values
			XMFLOAT3X4	mColorConversionMatrix;
			Float32		mOpacity;
			XMFLOAT3	mPadding;
		};

	public:
				CDirectXFragmentShaderYCbCr() :
					CDirectXPixelShader(CFilesystemPath(CString(OSSTR("PixelShaderYCbCr.cso")))),
							mConstantBuffer(NULL), mConstants(1.0)
					{}
				~CDirectXFragmentShaderYCbCr()
					{
						// Cleanup
						if (mConstantBuffer != NULL)
							mConstantBuffer->Release();
					}

		void	setup(ID3D11Device& d3dDevice, ID3D11DeviceContext3& d3dDeviceContext)
					{
						// Do super
						CDirectXPixelShader::setup(d3dDevice, d3dDeviceContext);

						// Update constants
						d3dDeviceContext.UpdateSubresource1(mConstantBuffer, 0, NULL, &mConstants, 0, 0, 0);
						d3dDeviceContext.PSSetConstantBuffers1(0, 1, &mConstantBuffer, NULL, NULL);
					}

		void	createResources(ID3D11Device& d3dDevice, const CData& shaderData)
					{
						// Create constant buffer
						CD3D11_BUFFER_DESC	constantBufferDesc(sizeof(SConstants), D3D11_BIND_CONSTANT_BUFFER);
						AssertFailIf(FAILED(d3dDevice.CreateBuffer(&constantBufferDesc, NULL, &mConstantBuffer)));
					}

		void	setColorConversionMatrix(const SMatrix3x3_32& colorConversionMatrix)
					{
						// Store
						mConstants.mColorConversionMatrix._11 = colorConversionMatrix.m1_1;
						mConstants.mColorConversionMatrix._12 = colorConversionMatrix.m1_2;
						mConstants.mColorConversionMatrix._13 = colorConversionMatrix.m1_3;

						mConstants.mColorConversionMatrix._21 = colorConversionMatrix.m2_1;
						mConstants.mColorConversionMatrix._22 = colorConversionMatrix.m2_2;
						mConstants.mColorConversionMatrix._23 = colorConversionMatrix.m2_3;

						mConstants.mColorConversionMatrix._31 = colorConversionMatrix.m3_1;
						mConstants.mColorConversionMatrix._32 = colorConversionMatrix.m3_2;
						mConstants.mColorConversionMatrix._33 = colorConversionMatrix.m3_3;
					}
		void	setOpacity(Float32 opacity)
					{ mConstants.mOpacity = opacity; }

		ID3D11Buffer*	mConstantBuffer;
		SConstants		mConstants;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUPixelShader

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CGPUFragmentShader& CGPUFragmentShader::getRGBAMultiTexture(Float32 opacity)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check opacity
	if (opacity == 1.0) {
		// No opacity
		static	CDirectXPixelShaderBasic*	sPixelShaderBasic = NULL;
		if (sPixelShaderBasic == NULL)
			// Create shader
			sPixelShaderBasic = new CDirectXPixelShaderBasic();

		return *sPixelShaderBasic;
	} else {
		// Have opacity
		static	CDirectXPixelShaderRGBAMultiTexture*	sPixelShaderRGBAMultiTexture = NULL;
		if (sPixelShaderRGBAMultiTexture == NULL)
			// Create shader
			sPixelShaderRGBAMultiTexture = new CDirectXPixelShaderRGBAMultiTexture();

		// Setup
		sPixelShaderRGBAMultiTexture->setOpacity(opacity);

		return *sPixelShaderRGBAMultiTexture;
	}
}

//----------------------------------------------------------------------------------------------------------------------
CGPUFragmentShader::Proc CGPUFragmentShader::getProc(CColor::Primaries primaries,
		CColor::YCbCrConversionMatrix yCbCrConversionMatrix, CColor::TransferFunction transferFunction)
//----------------------------------------------------------------------------------------------------------------------
{
	return sYCbCrFragmentShader;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions

//----------------------------------------------------------------------------------------------------------------------
CGPUFragmentShader&	sYCbCrFragmentShader(Float32 opacity)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	CDirectXFragmentShaderYCbCr*	sFragmentShader = nil;

	// Check if have shader
	if (sFragmentShader == nil)
		// Create shader
		sFragmentShader = new CDirectXFragmentShaderYCbCr();

	// Setup
	sFragmentShader->setColorConversionMatrix(CColor::mYCbCrConverstionMatrixRec601VideoRange);
	sFragmentShader->setOpacity(opacity);

	return *sFragmentShader;
}
