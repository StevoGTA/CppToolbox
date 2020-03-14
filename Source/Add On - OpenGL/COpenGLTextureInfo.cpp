//----------------------------------------------------------------------------------------------------------------------
//	COpenGLTextureInfo.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "COpenGLTextureInfo.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: COpenGLTextureInfoInternals

class COpenGLTextureInfoInternals {
	public:
		COpenGLTextureInfoInternals(const CGPUTexture& gpuTexture) :
			mUsedPixelsSize(gpuTexture.getGPUTextureSize())
			{
				// Setup
				mTotalPixelsSize.mWidth = SNumber::getNextPowerOf2(mUsedPixelsSize.mWidth);
				mTotalPixelsSize.mHeight = SNumber::getNextPowerOf2(mUsedPixelsSize.mHeight);

				GLint	format = (gpuTexture.getGPUTextureFormat() == kGPUTextureFormatRGB565) ? GL_RGB : GL_RGBA;

				switch (gpuTexture.getGPUTextureFormat()) {
					case kGPUTextureFormatRGB565:	mPixelFormat = GL_UNSIGNED_SHORT_5_6_5;
					case kGPUTextureFormatRGBA4444:	mPixelFormat = GL_UNSIGNED_SHORT_4_4_4_4;
					case kGPUTextureFormatRGBA5551:	mPixelFormat = GL_UNSIGNED_SHORT_5_5_5_1;

					case kGPUTextureFormatRGBA8888:	mPixelFormat = GL_UNSIGNED_BYTE;
				}

				// Setup GL texture
				glGenTextures(1, &mTextureName);
				glBindTexture(GL_TEXTURE_2D, mTextureName);

				if (mTotalPixelsSize == mUsedPixelsSize) {
					// Width and height are powers of 2 so use all
					glTexImage2D(GL_TEXTURE_2D, 0, format, mUsedPixelsSize.mWidth, mUsedPixelsSize.mHeight, 0,
							format, mPixelFormat, gpuTexture.getPixelData().getBytePtr());
				} else {
					// Width or height is not a power of 2 so expand texture space and use what we need
					UInt8*	empty = (UInt8*) calloc(mTotalPixelsSize.mWidth * mTotalPixelsSize.mHeight, 4);
					glTexImage2D(GL_TEXTURE_2D, 0, format, mTotalPixelsSize.mWidth, mTotalPixelsSize.mHeight, 0,
							format, mPixelFormat, empty);
					free(empty);

					glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mUsedPixelsSize.mWidth, mUsedPixelsSize.mHeight, format,
							mPixelFormat, gpuTexture.getPixelData().getBytePtr());
				}

				// Finish up the rest of the GL setup
//				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
		~COpenGLTextureInfoInternals()
			{
				// Cleanup
				glDeleteTextures(1, &mTextureName);
			}

	GLuint				mTextureName;
	GLenum				mPixelFormat;
	SGPUTextureSize		mUsedPixelsSize;
	SGPUTextureSize		mTotalPixelsSize;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - COpenGLTextureInfo

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
COpenGLTextureInfo::COpenGLTextureInfo(const CGPUTexture& gpuTexture)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new COpenGLTextureInfoInternals(gpuTexture);
}

//----------------------------------------------------------------------------------------------------------------------
COpenGLTextureInfo::~COpenGLTextureInfo()
//----------------------------------------------------------------------------------------------------------------------
{
	DisposeOf(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
GLuint COpenGLTextureInfo::getTextureName() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mTextureName;
}

//----------------------------------------------------------------------------------------------------------------------
bool COpenGLTextureInfo::hasTransparency() const
//----------------------------------------------------------------------------------------------------------------------
{
	switch (mInternals->mPixelFormat) {
		case GL_UNSIGNED_SHORT_4_4_4_4:
		case GL_UNSIGNED_SHORT_5_5_5_1:
		case GL_UNSIGNED_BYTE:
			// Have transparency
			return true;

		default:
			// No transparency
			return false;
	}
}

//----------------------------------------------------------------------------------------------------------------------
SGPUTextureSize COpenGLTextureInfo::getUsedPixelsGPUTextureSize() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mUsedPixelsSize;
}

//----------------------------------------------------------------------------------------------------------------------
SGPUTextureSize COpenGLTextureInfo::getTotalPixelsGPUTextureSize() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mTotalPixelsSize;
}
