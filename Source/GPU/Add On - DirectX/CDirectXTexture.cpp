//----------------------------------------------------------------------------------------------------------------------
//	CDirectXTexture.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CDirectXTexture.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CDirectXTextureInternals

class CDirectXTextureInternals : public TReferenceCountable<CDirectXTextureInternals> {
	public:
		CDirectXTextureInternals(ID3D11Device& device, ID3D11DeviceContext& deviceContext, const CData& data,
				CGPUTexture::DataFormat dataFormat, const S2DSizeU16& size) :
			mDataFormat(dataFormat), mSize(size), mTexture2D(NULL), mShaderResourceView(NULL)
			{
				// Setup
				D3D11_TEXTURE2D_DESC	texture2DDesc;
				texture2DDesc.Width = mSize.mWidth;
				texture2DDesc.Height = mSize.mHeight;
				texture2DDesc.MipLevels = 1;
				texture2DDesc.ArraySize = 1;
				texture2DDesc.SampleDesc.Count = 1;
				texture2DDesc.SampleDesc.Quality = 0;
				texture2DDesc.Usage = D3D11_USAGE_DEFAULT;
				texture2DDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
				texture2DDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
				texture2DDesc.MiscFlags = 0;

				UINT	bytesPerRow;
				switch (mDataFormat) {
					case CGPUTexture::kDataFormatRGBA8888:
						// RGBA8888
						texture2DDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
						bytesPerRow = 4 * size.mWidth;
						break;
				}

				// Create texture
				HRESULT	result = device.CreateTexture2D(&texture2DDesc, NULL, &mTexture2D);
				AssertFailIf(result != S_OK);

				// Update subresource
				D3D11_BOX	rect = {0, 0, 0, size.mWidth, size.mHeight, 1};
				deviceContext.UpdateSubresource(mTexture2D, 0, &rect, data.getBytePtr(), bytesPerRow, data.getSize());

				// Create resource view
				CD3D11_SHADER_RESOURCE_VIEW_DESC	shaderResourceViewDesc(mTexture2D, D3D_SRV_DIMENSION_TEXTURE2D,
															texture2DDesc.Format);
				AssertFailIf(FAILED(
						device.CreateShaderResourceView(mTexture2D, &shaderResourceViewDesc, &mShaderResourceView)));
			}
		~CDirectXTextureInternals()
			{
				if (mTexture2D != NULL)
					mTexture2D->Release();
				if (mShaderResourceView != NULL)
					mShaderResourceView->Release();
			}

		CGPUTexture::DataFormat		mDataFormat;
		S2DSizeU16					mSize;

		ID3D11Texture2D*			mTexture2D;
		ID3D11ShaderResourceView*	mShaderResourceView;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDirectXTexture

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CDirectXTexture::CDirectXTexture(ID3D11Device& device, ID3D11DeviceContext& deviceContext, const CData& data,
		DataFormat dataFormat, const S2DSizeU16& size)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CDirectXTextureInternals(device, deviceContext, data, dataFormat, size);
}

//----------------------------------------------------------------------------------------------------------------------
CDirectXTexture::CDirectXTexture(const CDirectXTexture& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CDirectXTexture::~CDirectXTexture()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: CGPUTexture methods

//----------------------------------------------------------------------------------------------------------------------
const S2DSizeU16& CDirectXTexture::getSize() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mSize;
}

// MARK: Temporary methods - will be removed in the future

//----------------------------------------------------------------------------------------------------------------------
const S2DSizeU16& CDirectXTexture::getUsedSize() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mSize;
}

//----------------------------------------------------------------------------------------------------------------------
ID3D11ShaderResourceView* CDirectXTexture::getShaderResourceView() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mShaderResourceView;
}

//----------------------------------------------------------------------------------------------------------------------
bool CDirectXTexture::hasTransparency() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check format
	switch (mInternals->mDataFormat) {
		case kDataFormatRGBA8888:	return true;
		default:					return false;
	}
}
