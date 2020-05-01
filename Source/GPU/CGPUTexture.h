//----------------------------------------------------------------------------------------------------------------------
//	CGPUTexture.h			©2019 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "C2DGeometry.h"
#include "CData.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: EGPUTextureDataFormat

enum EGPUTextureDataFormat {
	// 16 bpp raw formats
	kGPUTextureDataFormatRGB565		= 0x0001,
	kGPUTextureDataFormatRGBA4444	= 0x0002,
	kGPUTextureDataFormatRGBA5551	= 0x0003,

	// 32 bpp raw formats
	kGPUTextureDataFormatRGBA8888	= 0x0010,
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUTexture

class CGPUTexture {
	// Methods
	public:
									// Lifecycle methods
		virtual						~CGPUTexture() {}

									// Instance methods
		virtual	const	S2DSizeU16&	getSize() const = 0;

									// Temporary methods - will be removed in the future
		virtual	const	S2DSizeU16&	getUsedSize() const = 0;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SGPUTextureReference

struct SGPUTextureReference {
			// Lifecycle methods
			SGPUTextureReference() : mGPUTexture(nil) {}
			SGPUTextureReference(const CGPUTexture& gpuTexture) : mGPUTexture(&gpuTexture) {}

			// Instance methods
	bool	hasGPUTexture() const
				{ return mGPUTexture != nil; }
	void	reset()
				{ mGPUTexture = nil; }

	// Properties
	const	CGPUTexture*	mGPUTexture;
};
