//----------------------------------------------------------------------------------------------------------------------
//	CMetalTexture.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CBitmap.h"
#include "CGPUTexture.h"

#include <CoreVideo/CoreVideo.h>
#include <Metal/Metal.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMetalTexture

class CMetalTexture : public CGPUTexture {
	// Classes
	private:
		class Internals;

	// Methods
	public:
								// Lifecycle methods
								CMetalTexture(id<MTLDevice> device, const CBitmap& bitmap,
										CGPUTexture::DataFormat gpuTextureDataFormat);
								CMetalTexture(id<MTLDevice> device, const CData& data, const S2DSizeU16& dimensions,
										CGPUTexture::DataFormat gpuTextureDataFormat);
								CMetalTexture(CVMetalTextureCacheRef metalTextureCacheRef,
										CVImageBufferRef imageBufferRef, UInt32 planeIndex);
								CMetalTexture(const CMetalTexture& other);
								~CMetalTexture();

								// CGPUTexture methods
		const	S2DSizeU16&		getSize() const;

								// Temporary methods - will be removed in the future
		const	S2DSizeU16&		getUsedSize() const;

								// Instance methods
				id<MTLTexture>	getMetalTexture() const;
				bool			hasTransparency() const;

	// Properties
	private:
		Internals*	mInternals;
};
