//----------------------------------------------------------------------------------------------------------------------
//	COpenGLTextureInfo.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CGPUTexture.h"

#if TARGET_OS_IOS
	#include <OpenGLES/ES2/gl.h>
#endif

//#if TARGET_OS_LINUX
//	#include <GLES/gl.h>
//#endif

#if TARGET_OS_MACOS
	#include <OpenGL/gl.h>
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: COpenGLTextureInfo

class COpenGLTextureInfoInternals;
class COpenGLTextureInfo {
	// Methods
	public:
						// Lifecycle methods
						COpenGLTextureInfo(const CGPUTexture& gpuTexture);
						~COpenGLTextureInfo();

						// Instance methods
		GLuint			getTextureName() const;
		bool			hasTransparency() const;
		SGPUTextureSize	getUsedPixelsGPUTextureSize() const;
		SGPUTextureSize	getTotalPixelsGPUTextureSize() const;

	// Properties
	private:
		COpenGLTextureInfoInternals*	mInternals;
};
