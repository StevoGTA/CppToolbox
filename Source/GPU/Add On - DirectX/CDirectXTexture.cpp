//----------------------------------------------------------------------------------------------------------------------
//	CDirectXTexture.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CDirectXTexture.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CDirectXTextureInternals

class CDirectXTextureInternals : public TReferenceCountable<CDirectXTextureInternals> {
	public:
		CDirectXTextureInternals(ID3D11Device& device, ID3D11DeviceContext& deviceContext, const CData& data,
				EGPUTextureDataFormat gpuTextureDataFormat, const S2DSizeU16& size) :
			mGPUTextureDataFormat(gpuTextureDataFormat), mSize(size)
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
				switch (mGPUTextureDataFormat) {
					case kGPUTextureDataFormatRGBA8888:
						// RGBA8888
						texture2DDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
						bytesPerRow = 4 * size.mWidth;
						break;
				}

				// Create texture
				HRESULT	result = device.CreateTexture2D(&texture2DDesc, NULL, &mTexture2D);
				AssertFailIf(result != S_OK);

				// Upload data
				D3D11_BOX	rect = {0, 0, 0, size.mWidth, size.mHeight, 1};
				deviceContext.UpdateSubresource(mTexture2D, 0, &rect, data.getBytePtr(), bytesPerRow, data.getSize());
			}
		~CDirectXTextureInternals()
			{
				mTexture2D->Release();
			}

		EGPUTextureDataFormat	mGPUTextureDataFormat;
		S2DSizeU16				mSize;

		ID3D11Texture2D*		mTexture2D;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDirectXTexture

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CDirectXTexture::CDirectXTexture(ID3D11Device& device, ID3D11DeviceContext& deviceContext, const CData& data,
			EGPUTextureDataFormat gpuTextureDataFormat, const S2DSizeU16& size)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CDirectXTextureInternals(device, deviceContext, data, gpuTextureDataFormat, size);
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
bool CDirectXTexture::hasTransparency() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check format
	switch (mInternals->mGPUTextureDataFormat) {
		case kGPUTextureDataFormatRGBA8888:	return true;
	}
}
