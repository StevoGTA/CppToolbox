//----------------------------------------------------------------------------------------------------------------------
//	COpenGLTexture.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "COpenGLTexture.h"

#include "CReferenceCountable.h"
#include "CLogServices.h"

#if defined(TARGET_OS_IOS)
	#include <OpenGLES/ES3/glext.h>
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: COpenGLTexture::Internals

class COpenGLTexture::Internals : public TReferenceCountableAutoDelete<Internals> {
	public:
		Internals(const CData& data, CGPUTexture::DataFormat dataFormat, const S2DSizeU16& size) :
			TReferenceCountableAutoDelete(),
					mTextureTarget(GL_TEXTURE_2D),
					mUsedPixelsSize(size),
					mTotalPixelsSize(S2DSizeU16(SNumber::getNextPowerOf2(size.mWidth),
							SNumber::getNextPowerOf2(size.mHeight)))
			{
				// Setup
//				GLint	format = (gpuTextureDataFormat == CGPUTexture::kDataFormatRGB565) ? GL_RGB : GL_RGBA;
				GLint	format = GL_RGBA;
				GLenum	pixelFormat;
				switch (dataFormat) {
//					case CGPUTexture::kDataFormatRGB565:	mPixelFormat = GL_UNSIGNED_SHORT_5_6_5;		break;
//					case CGPUTexture::kDataFormatRGBA4444:	mPixelFormat = GL_UNSIGNED_SHORT_4_4_4_4;	break;
//					case CGPUTexture::kDataFormatRGBA5551:	mPixelFormat = GL_UNSIGNED_SHORT_5_5_5_1;	break;

					case CGPUTexture::kDataFormatRGBA8888:
						// RGBA8888
						mHasTransparency = true;
						pixelFormat = GL_UNSIGNED_BYTE;
						break;
				}

				// Setup GL texture
				glGenTextures(1, &mTextureName);
				glBindTexture(mTextureTarget, mTextureName);

				if (mUsedPixelsSize == mTotalPixelsSize)
					// Width and height are powers of 2 so use all
					glTexImage2D(mTextureTarget, 0, format, mUsedPixelsSize.mWidth, mUsedPixelsSize.mHeight, 0, format,
							pixelFormat, data.getBytePtr());
				else {
					// Width or height is not a power of 2 so expand texture space and use what we need
					UInt8*	empty = (UInt8*) calloc(mTotalPixelsSize.mWidth * mTotalPixelsSize.mHeight, 4);
					glTexImage2D(mTextureTarget, 0, format, mTotalPixelsSize.mWidth, mTotalPixelsSize.mHeight, 0,
							format, pixelFormat, empty);
					free(empty);

					glTexSubImage2D(mTextureTarget, 0, 0, 0, mUsedPixelsSize.mWidth, mUsedPixelsSize.mHeight, format,
							pixelFormat, data.getBytePtr());
				}

				// Finish up the rest of the GL setup
				glTexParameteri(mTextureTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
				glTexParameteri(mTextureTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

				glTexParameteri(mTextureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(mTextureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
#if defined(TARGET_OS_IOS)
		Internals(CVOpenGLESTextureCacheRef openGLTextureCacheRef, CVImageBufferRef imageBufferRef, UInt32 planeIndex) :
			TReferenceCountableAutoDelete(),
					mHasTransparency(false)
			{
				// Setup
				size_t	width = ::CVPixelBufferGetWidthOfPlane(imageBufferRef, planeIndex);
				size_t	height = ::CVPixelBufferGetHeightOfPlane(imageBufferRef, planeIndex);
				size_t	bytesPerRow = ::CVPixelBufferGetBytesPerRowOfPlane(imageBufferRef, planeIndex);
				OSType	formatType = ::CVPixelBufferGetPixelFormatType(imageBufferRef);

				// Set pixel format
				GLint	internalFormat;
				GLint	format;
				switch (formatType) {
					case kCVPixelFormatType_32ARGB:
					case kCVPixelFormatType_32BGRA:
						// 32 BGRA
						internalFormat = GL_RGBA;
						format = GL_BGRA;
						break;

					case kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange:
						// 420 YUV
						internalFormat = ((bytesPerRow / width) == 1) ? GL_LUMINANCE : GL_LUMINANCE_ALPHA;
						format = internalFormat;
						break;

					default:
						// Not yet implemented
						internalFormat = GL_RGBA;
						format = GL_RGBA;
						AssertFailUnimplemented();
				}

				// Create CoreVideo OpenGL texture
				CVOpenGLESTextureRef	openGLTextureRef;
				CVReturn				result =
												::CVOpenGLESTextureCacheCreateTextureFromImage(kCFAllocatorDefault,
														openGLTextureCacheRef, imageBufferRef, nil, GL_TEXTURE_2D,
														internalFormat, width, height, format, GL_UNSIGNED_BYTE,
														planeIndex, &openGLTextureRef);
				if (result == kCVReturnSuccess) {
					// Success
					mTextureTarget = ::CVOpenGLESTextureGetTarget(openGLTextureRef);
					mTextureName = ::CVOpenGLESTextureGetName(openGLTextureRef);
					mOpenGLTextureRef = OV<CVOpenGLESTextureRef>(openGLTextureRef);
					glBindTexture(mTextureTarget, mTextureName);
					glTexParameterf(mTextureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameterf(mTextureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					mUsedPixelsSize = S2DSizeU16(width, height);
					mTotalPixelsSize = S2DSizeU16(width, height);
				} else
					// Log error
					CLogServices::logError(
							CString(OSSTR("COpenGLTexture - error when creating texture from image: ")) +
									CString(result));
			}
#endif
#if defined(TARGET_OS_MACOS)
		Internals(CGLContextObj context, CVImageBufferRef imageBufferRef, UInt32 planeIndex) :
			TReferenceCountableAutoDelete()
			{
				// Setup
				size_t	width = ::CVPixelBufferGetWidthOfPlane(imageBufferRef, planeIndex);
				size_t	height = ::CVPixelBufferGetHeightOfPlane(imageBufferRef, planeIndex);
				size_t	bytesPerRow = ::CVPixelBufferGetBytesPerRowOfPlane(imageBufferRef, planeIndex);
				OSType	formatType = ::CVPixelBufferGetPixelFormatType(imageBufferRef);

				// Set pixel format
				GLenum	internalFormat;
				GLenum	format;
				switch (formatType) {
					case kCVPixelFormatType_420YpCbCr8BiPlanarVideoRange:
						// 420 YUV
						internalFormat = ((bytesPerRow / width) == 1) ? GL_R8 : GL_RG8;
						format = ((bytesPerRow / width) == 1) ? GL_RED : GL_RG;
						break;

					default:
						// Not yet implemented
						internalFormat = GL_RGBA;
						format = GL_RGBA;
						AssertFailUnimplemented();
				}

				// Setup GL texture
				mTextureTarget = GL_TEXTURE_RECTANGLE;
				glGenTextures(1, &mTextureName);
				glBindTexture(mTextureTarget, mTextureName);

				IOSurfaceRef	ioSurfaceRef = ::CVPixelBufferGetIOSurface(imageBufferRef);
				CGLError		result =
										::CGLTexImageIOSurface2D(context, GL_TEXTURE_RECTANGLE, internalFormat, width,
												height, format, GL_UNSIGNED_BYTE, ioSurfaceRef, planeIndex);
				if (result == kCGLNoError) {
					// Success
					mIOSurfaceRef = OV<IOSurfaceRef>(ioSurfaceRef);

					glTexParameterf(mTextureTarget, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameterf(mTextureTarget, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					mUsedPixelsSize = S2DSizeU16(width, height);
					mTotalPixelsSize = S2DSizeU16(width, height);
				} else
					// Log error
					CLogServices::logError(
							CString(OSSTR("COpenGLTexture - error when creating texture from image: ")) +
									CString(result));
			}
#endif
		~Internals()
			{
				// Cleanup
#if defined(TARGET_OS_IOS)
				if (mOpenGLTextureRef.hasValue())
					// Release texture ref
					::CFRelease(*mOpenGLTextureRef);
				else if (mTextureName != 0)
					// Delete texture
					glDeleteTextures(1, &mTextureName);
#else
				if (mTextureName != 0)
					// Delete texture
					glDeleteTextures(1, &mTextureName);
#endif
			}

		GLenum						mTextureTarget;
		GLuint						mTextureName;
		S2DSizeU16					mUsedPixelsSize;
		S2DSizeU16					mTotalPixelsSize;
		bool						mHasTransparency;
#if defined(TARGET_OS_IOS)
		OV<CVOpenGLESTextureRef>	mOpenGLTextureRef;
#endif
#if defined(TARGET_OS_MACOS)
		OV<IOSurfaceRef>			mIOSurfaceRef;
#endif
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - COpenGLTexture

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
COpenGLTexture::COpenGLTexture(const CData& data, CGPUTexture::DataFormat dataFormat, const S2DSizeU16& size) :
		CGPUTexture()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(data, dataFormat, size);
}

#if defined(TARGET_OS_IOS)
//----------------------------------------------------------------------------------------------------------------------
COpenGLTexture::COpenGLTexture(CVOpenGLESTextureCacheRef openGLTextureCacheRef, CVImageBufferRef imageBufferRef,
		UInt32 planeIndex) : CGPUTexture()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(openGLTextureCacheRef, imageBufferRef, planeIndex);
}
#endif

#if defined(TARGET_OS_MACOS)
//----------------------------------------------------------------------------------------------------------------------
COpenGLTexture::COpenGLTexture(CGLContextObj context, CVImageBufferRef imageBufferRef,
		UInt32 planeIndex) : CGPUTexture()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(context, imageBufferRef, planeIndex);
}
#endif

//----------------------------------------------------------------------------------------------------------------------
COpenGLTexture::COpenGLTexture(const COpenGLTexture& other) : CGPUTexture()
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
void COpenGLTexture::bind() const
//----------------------------------------------------------------------------------------------------------------------
{
	glBindTexture(mInternals->mTextureTarget, mInternals->mTextureName);
}

//----------------------------------------------------------------------------------------------------------------------
bool COpenGLTexture::hasTransparency() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mHasTransparency;
}
