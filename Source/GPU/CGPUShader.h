//----------------------------------------------------------------------------------------------------------------------
//	CGPUShader.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CColor.h"
#include "CMatrix.h"
#include "CGPUTexture.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SVertex2DMultitexture
//	Supports up to 16 simultaneous textures

struct SVertex2DMultitexture {

	// Lifecycle methods
	SVertex2DMultitexture(const S2DPointF32& position, const S2DPointF32& texture, UInt8 textureIndex) :
		mPosition(position), mTexture(texture), mTextureIndex(textureIndex)
		{}

	// Properties
	S2DPointF32	mPosition;
	S2DPointF32	mTexture;
	Float32		mTextureIndex;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUVertexShader

class CGPUVertexShader {
	// Methods
	public:
									// Lifecycle methods
		virtual						~CGPUVertexShader() {}

									// Instance methods
		virtual	UInt32				getPerVertexByteCount() const = 0;

									// Class methods
		static	CGPUVertexShader&	getBasic2DMultiTexture();
		static	CGPUVertexShader&	getClip2DMultiTexture(const SMatrix4x1_32& clipPlane);

	protected:
									// Lifecycle methods
									CGPUVertexShader() {}
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUFragmentShader

class CGPUFragmentShader {
	// Types
	public:
		typedef	CGPUFragmentShader&	(*Proc)(Float32 opacity);

	// Methods
	public:
									// Lifecycle methods
		virtual						~CGPUFragmentShader() {}

									// Class methods
		static	CGPUFragmentShader&	getRGBAMultiTexture(Float32 opacity);
		static	Proc				getProc(CColor::Primaries primaries,
											CColor::YCbCrConversionMatrix yCbCrConversionMatrix,
											CColor::TransferFunction transferFunction);

	protected:
									// Lifecycle methods
									CGPUFragmentShader() {}
};
