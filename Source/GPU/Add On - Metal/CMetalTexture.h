//----------------------------------------------------------------------------------------------------------------------
//	CMetalTexture.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CGPUTexture.h"

#import <Metal/Metal.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMetalTexture

class CMetalTextureInternals;
class CMetalTexture : public CGPUTexture {
	// Methods
	public:
								// Lifecycle methods
								CMetalTexture(id<MTLDevice> device, const CData& data,
										EGPUTextureDataFormat gpuTextureDataFormat, const S2DSizeU16& size);
								CMetalTexture(const CMetalTexture& other);
								~CMetalTexture();

								// CGPUTexture methods
				CGPUTexture*	copy() const
									{ return new CMetalTexture(*this); }
		const	S2DSizeU16&		getSize() const;

								// Temporary methods - will be removed in the future
		const	S2DSizeU16&		getUsedSize() const;

								// Instance methods
				id<MTLTexture>	getMetalTexture() const;
				bool			hasTransparency() const;

	// Properties
	private:
		CMetalTextureInternals*	mInternals;
};
