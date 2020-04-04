//----------------------------------------------------------------------------------------------------------------------
//	CGPUTexture.h			©2019 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "C2DGeometry.h"
#include "CData.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Types

typedef	T2DSize<UInt16>		SGPUTextureSize;

//----------------------------------------------------------------------------------------------------------------------
// MARK: - EGPUTextureFormat

enum EGPUTextureFormat {
	// 16 bpp raw formats
	kGPUTextureFormatRGB565		= 0x0001,
	kGPUTextureFormatRGBA4444	= 0x0002,
	kGPUTextureFormatRGBA5551	= 0x0003,

	// 32 bpp raw formats
	kGPUTextureFormatRGBA8888	= 0x0010,
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SGPUTextureInfo

struct SGPUTextureInfo {
			// Lifecycle methods
			SGPUTextureInfo(const SGPUTextureSize& gpuTextureSize, Float32 maxU, Float32 maxV,
					void* internalReference) :
				mGPUTextureSize(gpuTextureSize), mMaxU(maxU), mMaxV(maxV), mInternalReference(internalReference)
				{}
			SGPUTextureInfo(const SGPUTextureInfo& other) :
				mGPUTextureSize(other.mGPUTextureSize), mMaxU(other.mMaxU), mMaxV(other.mMaxV),
						mInternalReference(other.mInternalReference)
				{}
			SGPUTextureInfo() : mGPUTextureSize(), mMaxU(0.0), mMaxV(0.0), mInternalReference(nil) {}

			// Instance methods
	bool	isValid() const
				{ return mMaxU != 0.0; }

	// Properties
	SGPUTextureSize	mGPUTextureSize;
	Float32			mMaxU;
	Float32			mMaxV;
	void*			mInternalReference;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUTexture

class CGPUTextureInternals;
class CGPUTexture {
	// Methods
	public:
									// Lifecycle methods
									CGPUTexture(const CData& pixelData, EGPUTextureFormat gpuTextureFormat,
											SGPUTextureSize gpuTextureSize);
									~CGPUTexture();

									// Instance methods
		const	CData&				getPixelData() const;
				EGPUTextureFormat	getGPUTextureFormat() const;
				SGPUTextureSize		getGPUTextureSize() const;

	// Properties
	private:
		CGPUTextureInternals*	mInternals;
};
