//----------------------------------------------------------------------------------------------------------------------
//	CGPUTexture.h			©2019 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "C2DGeometry.h"
#include "CData.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPUTexture

class CGPUTexture {
	// Enums
	public:
		enum DataFormat {
			// 16 bpp raw formats
		//	kDataFormatRGB565	= 0x0001,
		//	kDataFormatRGBA4444	= 0x0002,
		//	kDataFormatRGBA5551	= 0x0003,

			// 32 bpp raw formats
			kDataFormatRGBA8888	= 0x0010,
		};

	// Methods
	public:
										// Lifecycle methods
		virtual							~CGPUTexture() {}

										// Instance methods
		virtual			CGPUTexture*	copy() const = 0;

		virtual	const	S2DSizeU16&		getSize() const = 0;

										// Temporary methods - will be removed in the future
		virtual	const	S2DSizeU16&		getUsedSize() const = 0;
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
