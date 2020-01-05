//----------------------------------------------------------------------------------------------------------------------
//	CGPUTexture.h			©2019 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "C2DGeometry.h"

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
