//----------------------------------------------------------------------------------------------------------------------
//	CDirectXTexture.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CGPUTexture.h"
#include "TWrappers-Windows.h"

#undef Delete
#include <d3d11_3.h>
#define Delete(x)	{ delete x; x = nil; }

//----------------------------------------------------------------------------------------------------------------------
// MARK: CDirectXTexture

class CDirectXTexture : public CGPUTexture {
	// Classes
	private:
		class Internals;

	// Methods
	public:
														// Lifecycle methods
														CDirectXTexture(ID3D11Device& device,
																ID3D11DeviceContext& deviceContext, const CData& data,
																DXGI_FORMAT format, const S2DSizeU16& size);
														CDirectXTexture(const CDirectXTexture& other);
														~CDirectXTexture();

														// CGPUTexture methods
				CGPUTexture*							copy() const
															{ return new CDirectXTexture(*this); }
		const	S2DSizeU16&								getSize() const;

														// Temporary methods - will be removed in the future
		const	S2DSizeU16&								getUsedSize() const;

														// Instance methods
		const	TArray<CI<ID3D11ShaderResourceView> >&	getShaderResourceViews() const;
				bool									hasTransparency() const;

	// Properties
	private:
		Internals* mInternals;
};
