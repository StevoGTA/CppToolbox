//----------------------------------------------------------------------------------------------------------------------
//	CDirectXTexture.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CDirectXTexture.h"

#include "CReferenceCountable.h"
#include "SError.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CDirectXTexture::Internals

class CDirectXTexture::Internals : public TReferenceCountableAutoDelete<Internals> {
	public:
		Internals(ID3D11Device& device, ID3D11DeviceContext& deviceContext, const CBitmap& bitmap, DXGI_FORMAT format) :
			TReferenceCountableAutoDelete(),
					mFormat(format), mDimensions(S2DSizeU16(bitmap.getSize().mWidth, bitmap.getSize().mHeight))
			{
				// Setup
				D3D11_TEXTURE2D_DESC	texture2DDesc = {0};
				texture2DDesc.Format = mFormat;
				texture2DDesc.Width = mDimensions.mWidth;
				texture2DDesc.Height = mDimensions.mHeight;
				texture2DDesc.MipLevels = 1;
				texture2DDesc.ArraySize = 1;
				texture2DDesc.SampleDesc.Count = 1;
				texture2DDesc.SampleDesc.Quality = 0;
				texture2DDesc.Usage = D3D11_USAGE_IMMUTABLE;
				texture2DDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
				texture2DDesc.CPUAccessFlags = 0;
				texture2DDesc.MiscFlags = 0;

				switch (mFormat) {
					case DXGI_FORMAT_R8G8B8A8_UNORM:
						// RGBA8888
						mHasTransparency = true;
						break;

					case DXGI_FORMAT_NV12:
						// NV12
						mHasTransparency = false;
						break;

					default:
						// ???
						AssertFailUnimplemented();
						break;
				}

				// Setup subresource data
				D3D11_SUBRESOURCE_DATA	subresourceData = {0};
				subresourceData.pSysMem = bitmap.getPixelData().getBytePtr();
				subresourceData.SysMemPitch = bitmap.getBytesPerRow();
				subresourceData.SysMemSlicePitch = 0;

				// Create texture
				ID3D11Texture2D*	texture2D;
				HRESULT	result = device.CreateTexture2D(&texture2DDesc, &subresourceData, &texture2D);
				AssertFailIf(result != S_OK);
				mTexture2D = OCI<ID3D11Texture2D>(texture2D);

				// Create resource view(s)
				D3D11_SHADER_RESOURCE_VIEW_DESC		shaderResourceViewDesc;
				ID3D11ShaderResourceView*			shaderResourceView;
				switch (mFormat) {
					case DXGI_FORMAT_R8G8B8A8_UNORM:
						// RGBA8888
						shaderResourceViewDesc =
								CD3D11_SHADER_RESOURCE_VIEW_DESC(*mTexture2D, D3D_SRV_DIMENSION_TEXTURE2D,
										texture2DDesc.Format);
						result =
								device.CreateShaderResourceView(*mTexture2D, &shaderResourceViewDesc,
										&shaderResourceView);
						AssertFailIf(result != S_OK);
						mShaderResourceViews += CI<ID3D11ShaderResourceView>(shaderResourceView);
						break;

					case DXGI_FORMAT_NV12:
						// NV12
						shaderResourceViewDesc =
								CD3D11_SHADER_RESOURCE_VIEW_DESC(*mTexture2D, D3D11_SRV_DIMENSION_TEXTURE2D,
										DXGI_FORMAT_R8_UNORM);
						result =
								device.CreateShaderResourceView(*mTexture2D, &shaderResourceViewDesc,
										&shaderResourceView);
						AssertFailIf(result != S_OK);
						mShaderResourceViews += CI<ID3D11ShaderResourceView>(shaderResourceView);

						shaderResourceViewDesc =
								CD3D11_SHADER_RESOURCE_VIEW_DESC(*mTexture2D, D3D11_SRV_DIMENSION_TEXTURE2D,
										DXGI_FORMAT_R8G8_UNORM);
						result =
								device.CreateShaderResourceView(*mTexture2D, &shaderResourceViewDesc,
										&shaderResourceView);
						AssertFailIf(result != S_OK);
						mShaderResourceViews += CI<ID3D11ShaderResourceView>(shaderResourceView);
						break;
				}
			}
		Internals(ID3D11Device& device, ID3D11DeviceContext& deviceContext, const CData& data, DXGI_FORMAT format,
				const S2DSizeU16& dimensions) :
			TReferenceCountableAutoDelete(),
					mFormat(format), mDimensions(dimensions)
			{
				// Setup
				D3D11_TEXTURE2D_DESC	texture2DDesc = {0};
				texture2DDesc.Format = mFormat;
				texture2DDesc.Width = mDimensions.mWidth;
				texture2DDesc.Height = mDimensions.mHeight;
				texture2DDesc.MipLevels = 1;
				texture2DDesc.ArraySize = 1;
				texture2DDesc.SampleDesc.Count = 1;
				texture2DDesc.SampleDesc.Quality = 0;
				texture2DDesc.Usage = D3D11_USAGE_IMMUTABLE;
				texture2DDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
				texture2DDesc.CPUAccessFlags = 0;
				texture2DDesc.MiscFlags = 0;

				UINT bytesPerRow;
				switch (mFormat) {
					case DXGI_FORMAT_R8G8B8A8_UNORM:
						// RGBA8888
						mHasTransparency = true;
						bytesPerRow = 4 * mDimensions.mWidth;
						break;

					case DXGI_FORMAT_NV12:
						// NV12
						mHasTransparency = false;
						bytesPerRow = mDimensions.mWidth;
						break;

					default:
						// ???
						AssertFailUnimplemented();
						break;
				}

				// Setup subresource data
				D3D11_SUBRESOURCE_DATA	subresourceData = {0};
				subresourceData.pSysMem = data.getBytePtr();
				subresourceData.SysMemPitch = bytesPerRow;
				subresourceData.SysMemSlicePitch = 0;

				// Create texture
				ID3D11Texture2D*	texture2D;
				HRESULT	result = device.CreateTexture2D(&texture2DDesc, &subresourceData, &texture2D);
				AssertFailIf(result != S_OK);
				mTexture2D = OCI<ID3D11Texture2D>(texture2D);

				// Create resource view(s)
				D3D11_SHADER_RESOURCE_VIEW_DESC		shaderResourceViewDesc;
				ID3D11ShaderResourceView*			shaderResourceView;
				switch (mFormat) {
					case DXGI_FORMAT_R8G8B8A8_UNORM:
						// RGBA8888
						shaderResourceViewDesc =
								CD3D11_SHADER_RESOURCE_VIEW_DESC(*mTexture2D, D3D_SRV_DIMENSION_TEXTURE2D,
										texture2DDesc.Format);
						result =
								device.CreateShaderResourceView(*mTexture2D, &shaderResourceViewDesc,
										&shaderResourceView);
						AssertFailIf(result != S_OK);
						mShaderResourceViews += CI<ID3D11ShaderResourceView>(shaderResourceView);
						break;

					case DXGI_FORMAT_NV12:
						// NV12
						shaderResourceViewDesc =
								CD3D11_SHADER_RESOURCE_VIEW_DESC(*mTexture2D, D3D11_SRV_DIMENSION_TEXTURE2D,
										DXGI_FORMAT_R8_UNORM);
						result =
								device.CreateShaderResourceView(*mTexture2D, &shaderResourceViewDesc,
										&shaderResourceView);
						AssertFailIf(result != S_OK);
						mShaderResourceViews += CI<ID3D11ShaderResourceView>(shaderResourceView);

						shaderResourceViewDesc =
								CD3D11_SHADER_RESOURCE_VIEW_DESC(*mTexture2D, D3D11_SRV_DIMENSION_TEXTURE2D,
										DXGI_FORMAT_R8G8_UNORM);
						result =
								device.CreateShaderResourceView(*mTexture2D, &shaderResourceViewDesc,
										&shaderResourceView);
						AssertFailIf(result != S_OK);
						mShaderResourceViews += CI<ID3D11ShaderResourceView>(shaderResourceView);
						break;
				}
			}

		DXGI_FORMAT								mFormat;
		S2DSizeU16								mDimensions;
		bool									mHasTransparency;

		OCI<ID3D11Texture2D>					mTexture2D;
		TNArray<CI<ID3D11ShaderResourceView> >	mShaderResourceViews;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDirectXTexture

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CDirectXTexture::CDirectXTexture(ID3D11Device& device, ID3D11DeviceContext& deviceContext, const CBitmap& bitmap,
		DXGI_FORMAT format)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(device, deviceContext, bitmap, format);
}

//----------------------------------------------------------------------------------------------------------------------
CDirectXTexture::CDirectXTexture(ID3D11Device& device, ID3D11DeviceContext& deviceContext, const CData& data,
		DXGI_FORMAT format, const S2DSizeU16& dimensions)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(device, deviceContext, data, format, dimensions);
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
	return mInternals->mDimensions;
}

// MARK: Temporary methods - will be removed in the future

//----------------------------------------------------------------------------------------------------------------------
const S2DSizeU16& CDirectXTexture::getUsedSize() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mDimensions;
}

//----------------------------------------------------------------------------------------------------------------------
const TArray<CI<ID3D11ShaderResourceView> >& CDirectXTexture::getShaderResourceViews() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mShaderResourceViews;
}

//----------------------------------------------------------------------------------------------------------------------
bool CDirectXTexture::hasTransparency() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mHasTransparency;
}
