//----------------------------------------------------------------------------------------------------------------------
//	COpenGLTexture.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CGPUTexture.h"

#if TARGET_OS_IOS
	#include <CoreVideo/CoreVideo.h>
	#include <OpenGLES/ES3/gl.h>
#endif

//#if TARGET_OS_LINUX
//	#include <GLES/gl.h>
//#endif

#if TARGET_OS_MACOS
	#include <CoreVideo/CoreVideo.h>
	#include <OpenGL/gl3.h>
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: COpenGLTexture

class COpenGLTextureInternals;
class COpenGLTexture : public CGPUTexture {
	// Methods
	public:
								// Lifecycle methods
								COpenGLTexture(const CData& data, DataFormat dataFormat, const S2DSizeU16& size);
#if TARGET_OS_IOS
								COpenGLTexture(CVOpenGLESTextureCacheRef openGLTextureCacheRef,
										CVImageBufferRef imageBufferRef, UInt32 planeIndex);
#endif
#if TARGET_OS_MACOS
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
		COpenGLTextureInternals*	mInternals;
};
