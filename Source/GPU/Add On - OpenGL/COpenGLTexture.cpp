//----------------------------------------------------------------------------------------------------------------------
//	COpenGLTexture.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "COpenGLTexture.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: COpenGLTextureInternals

class COpenGLTextureInternals : public TReferenceCountable<COpenGLTextureInternals> {
	public:
		COpenGLTextureInternals(const CData& data, EGPUTextureDataFormat gpuTextureDataFormat, const S2DSizeU16& size) :
mUsedPixelsSize(size),
			TReferenceCountable(), mTotalPixelsSize(S2DSizeU16(SNumber::getNextPowerOf2(size.mWidth),
					SNumber::getNextPowerOf2(size.mHeight)))
			{
				// Setup
//				GLint	format = (gpuTextureDataFormat == kGPUTextureDataFormatRGB565) ? GL_RGB : GL_RGBA;
				GLint	format = GL_RGBA;

				switch (gpuTextureDataFormat) {
//					case kGPUTextureDataFormatRGB565:	mPixelFormat = GL_UNSIGNED_SHORT_5_6_5;		break;
//					case kGPUTextureDataFormatRGBA4444:	mPixelFormat = GL_UNSIGNED_SHORT_4_4_4_4;	break;
//					case kGPUTextureDataFormatRGBA5551:	mPixelFormat = GL_UNSIGNED_SHORT_5_5_5_1;	break;

					case kGPUTextureDataFormatRGBA8888:	mPixelFormat = GL_UNSIGNED_BYTE;			break;
				}

				// Setup GL texture
				glGenTextures(1, &mTextureName);
				glBindTexture(GL_TEXTURE_2D, mTextureName);

				if (mUsedPixelsSize == mTotalPixelsSize)
					// Width and height are powers of 2 so use all
					glTexImage2D(GL_TEXTURE_2D, 0, format, mUsedPixelsSize.mWidth, mUsedPixelsSize.mHeight, 0, format,
							mPixelFormat, data.getBytePtr());
				else {
					// Width or height is not a power of 2 so expand texture space and use what we need
					UInt8*	empty = (UInt8*) calloc(mTotalPixelsSize.mWidth * mTotalPixelsSize.mHeight, 4);
					glTexImage2D(GL_TEXTURE_2D, 0, format, mTotalPixelsSize.mWidth, mTotalPixelsSize.mHeight, 0, format,
							mPixelFormat, empty);
					free(empty);

					glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mUsedPixelsSize.mWidth, mUsedPixelsSize.mHeight, format,
							mPixelFormat, data.getBytePtr());
				}

				// Finish up the rest of the GL setup
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
		~COpenGLTextureInternals()
			{
				// Cleanup
				glDeleteTextures(1, &mTextureName);
			}

	GLuint		mTextureName;
	GLenum		mPixelFormat;
	S2DSizeU16	mUsedPixelsSize;
	S2DSizeU16	mTotalPixelsSize;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - COpenGLTexture

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
COpenGLTexture::COpenGLTexture(const CData& data, EGPUTextureDataFormat gpuTextureDataFormat, const S2DSizeU16& size)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new COpenGLTextureInternals(data, gpuTextureDataFormat, size);
}

//----------------------------------------------------------------------------------------------------------------------
COpenGLTexture::COpenGLTexture(const COpenGLTexture& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
COpenGLTexture::~COpenGLTexture()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: CGPUTexture methods

//----------------------------------------------------------------------------------------------------------------------
const S2DSizeU16& COpenGLTexture::getSize() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mTotalPixelsSize;
}

// MARK: Temporary methods - will be removed in the future

//----------------------------------------------------------------------------------------------------------------------
const S2DSizeU16& COpenGLTexture::getUsedSize() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mUsedPixelsSize;
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
GLuint COpenGLTexture::getTextureName() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mTextureName;
}

//----------------------------------------------------------------------------------------------------------------------
bool COpenGLTexture::hasTransparency() const
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
