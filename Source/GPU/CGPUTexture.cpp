//----------------------------------------------------------------------------------------------------------------------
//	CGPUTexture.cpp			©2019 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CGPUTexture.h"

#include "CData.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPUTextureInternals
class CGPUTextureInternals {
	public:
		CGPUTextureInternals(const CData& pixelData, EGPUTextureFormat gpuTextureFormat,
				SGPUTextureSize gpuTextureSize) :
			mPixelData(pixelData), mGPUTextureFormat(gpuTextureFormat), mGPUTextureSize(gpuTextureSize)
		 {}

		const	CData				mPixelData;
				EGPUTextureFormat	mGPUTextureFormat;
				SGPUTextureSize		mGPUTextureSize;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUTexture

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CGPUTexture::CGPUTexture(const CData& pixelData, EGPUTextureFormat gpuTextureFormat, SGPUTextureSize gpuTextureSize)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CGPUTextureInternals(pixelData, gpuTextureFormat, gpuTextureSize);
}

//----------------------------------------------------------------------------------------------------------------------
CGPUTexture::~CGPUTexture()
//----------------------------------------------------------------------------------------------------------------------
{
	DisposeOf(mInternals);
}

//----------------------------------------------------------------------------------------------------------------------
const CData& CGPUTexture::getPixelData() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mPixelData;
}

//----------------------------------------------------------------------------------------------------------------------
EGPUTextureFormat CGPUTexture::getGPUTextureFormat() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mGPUTextureFormat;
}

//----------------------------------------------------------------------------------------------------------------------
SGPUTextureSize CGPUTexture::getGPUTextureSize() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mGPUTextureSize;
}
