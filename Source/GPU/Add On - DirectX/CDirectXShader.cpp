//----------------------------------------------------------------------------------------------------------------------
//	CDirectXShader.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CDirectXShader.h"

#include "CFileReader.h"

#include <DirectXMath.h>

#define Delete(x)	{ delete x; x = nil; }

using namespace DirectX;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: CDirectXVertexShaderInternals

class CDirectXVertexShaderInternals {
	public:
		CDirectXVertexShaderInternals(const CFilesystemPath& filesystemPath) :
			mFilesystemPath(filesystemPath), mShader(NULL)
			{}
		~CDirectXVertexShaderInternals()
			{
				// Cleanup
				if (mShader != NULL)
					mShader->Release();
			}

		CFilesystemPath		mFilesystemPath;

		ID3D11VertexShader*	mShader;

		SMatrix4x4_32		mModelMatrix;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDirectXVertexShader

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CDirectXVertexShader::CDirectXVertexShader(const CFilesystemPath& filesystemPath) : CGPUVertexShader()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CDirectXVertexShaderInternals(filesystemPath);
}

//----------------------------------------------------------------------------------------------------------------------
CDirectXVertexShader::~CDirectXVertexShader()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CDirectXVertexShader::setup(ID3D11Device& d3dDevice, ID3D11DeviceContext3& d3dDeviceContext,
		const XMFLOAT4X4& projectionMatrix, const XMFLOAT4X4& viewMatrix)
//----------------------------------------------------------------------------------------------------------------------
{
	// Finish setup if needed
	if (mInternals->mShader == NULL) {
		// Load data
		UError	error;
		CData	data = CFileReader::readData(CFile(mInternals->mFilesystemPath), error);
		AssertFailIf(error != kNoError);

		// Create Vertex Shader
		HRESULT	result;	
		result = d3dDevice.CreateVertexShader(data.getBytePtr(), data.getSize(), NULL, &mInternals->mShader);
		AssertFailIf(FAILED(result));

		// Setup input layout
		setupInputLayout(d3dDevice, data);
	}

	// Make current
	d3dDeviceContext.VSSetShader(mInternals->mShader, NULL, 0);
	makeInputLayoutCurrent(d3dDeviceContext);
}

//----------------------------------------------------------------------------------------------------------------------
void CDirectXVertexShader::setModelMatrix(const SMatrix4x4_32& modelMatrix)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mModelMatrix = modelMatrix;
}

//----------------------------------------------------------------------------------------------------------------------
const SMatrix4x4_32& CDirectXVertexShader::getModelMatrix() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mModelMatrix;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDirectXPixelShaderInternals

class CDirectXPixelShaderInternals {
	public:
		CDirectXPixelShaderInternals(const CFilesystemPath& filesystemPath) :
			mFilesystemPath(filesystemPath), mShader(NULL)
			{}
		~CDirectXPixelShaderInternals()
			{
				// Cleanup
				if (mShader != NULL)
					mShader->Release();
			}

		CFilesystemPath		mFilesystemPath;

		ID3D11PixelShader*	mShader;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDirectXPixelShader

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CDirectXPixelShader::CDirectXPixelShader(const CFilesystemPath& filesystemPath) : CGPUFragmentShader()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CDirectXPixelShaderInternals(filesystemPath);
}

//----------------------------------------------------------------------------------------------------------------------
CDirectXPixelShader::~CDirectXPixelShader()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CDirectXPixelShader::setup(ID3D11Device& d3dDevice, ID3D11DeviceContext& d3dDeviceContext)
//----------------------------------------------------------------------------------------------------------------------
{
	// Finish setup if needed
	if (mInternals->mShader == NULL) {
		// Load data
		UError	error;
		CData	data = CFileReader::readData(CFile(mInternals->mFilesystemPath), error);
		AssertFailIf(error != kNoError);

		// Create Pixel Shader
		HRESULT	result = d3dDevice.CreatePixelShader(data.getBytePtr(), data.getSize(), NULL, &mInternals->mShader);
		AssertFailIf(FAILED(result));
	}

	// Make current
	d3dDeviceContext.PSSetShader(mInternals->mShader, NULL, 0);
}
