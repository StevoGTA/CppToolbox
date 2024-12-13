//----------------------------------------------------------------------------------------------------------------------
//	COpenGLTexture.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CBitmap.h"
#include "CGPUTexture.h"

#if defined(TARGET_OS_IOS)
	#include <CoreVideo/CoreVideo.h>
	#include <OpenGLES/ES3/gl.h>
#endif

//#if defined(TARGET_OS_LINUX)
//	#include <GLES/gl.h>
//#endif

#if defined(TARGET_OS_MACOS)
	#include <CoreVideo/CoreVideo.h>
	#include <OpenGL/gl3.h>
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: COpenGLTexture

class COpenGLTexture : public CGPUTexture {
	// Classes
	private:
		class Internals;

	// Methods
	public:
								// Lifecycle methods
								COpenGLTexture(const CBitmap& bitmap, CGPUTexture::DataFormat gpuTextureDataFormat);
								COpenGLTexture(const CData& data, const S2DSizeU16& dimensions,
										CGPUTexture::DataFormat gpuTextureDataFormat);
#if defined(TARGET_OS_IOS)
								COpenGLTexture(CVOpenGLESTextureCacheRef openGLTextureCacheRef,
										CVImageBufferRef imageBufferRef, UInt32 planeIndex);
#endif
#if defined(TARGET_OS_MACOS)
								COpenGLTexture(CGLContextObj context,
										CVImageBufferRef imageBufferRef, UInt32 planeIndex);
#endif
								COpenGLTexture(const COpenGLTexture& other);
								~COpenGLTexture();

								// CGPUTexture methods
				CGPUTexture*	copy() const
									{ return new COpenGLTexture(*this); }
		const	S2DSizeU16&		getSize() const;

								// Temporary methods - will be removed in the future
		const	S2DSizeU16&		getUsedSize() const;

								// Instance methods
				void			bind() const;
				bool			hasTransparency() const;

	// Properties
	private:
		Internals*	mInternals;
};
