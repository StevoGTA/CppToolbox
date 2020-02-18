//----------------------------------------------------------------------------------------------------------------------
//	COpenGLGPU.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "COpenGLGPU.h"

#include "CData.h"
#include "CThread.h"

//#if TARGET_OS_IOS
//#include <OpenGLES/ES1/gl.h>
//#endif
//
//#if TARGET_OS_LINUX
//#include <GLES/gl.h>
//#endif

#if TARGET_OS_MACOS
//#include <math.h>
#include <OpenGL/gl.h>
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: SOpenGLTextureInfo

struct SOpenGLTextureInfo {
			// Lifecycle methods
			SOpenGLTextureInfo(const CGPUTexture& gpuTexture) :
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
//					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				}
			~SOpenGLTextureInfo()
				{
					// Cleanup
					glDeleteTextures(1, &mTextureName);
				}

	bool	hasTransparency()
				{
					switch (mPixelFormat) {
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

	// Properties
	GLuint				mTextureName;
	GLenum				mPixelFormat;
	SGPUTextureSize		mUsedPixelsSize;
	SGPUTextureSize		mTotalPixelsSize;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - COpenGLGPUInternals

class COpenGLGPUInternals {
	public:
		COpenGLGPUInternals(const COpenGLGPUProcsInfo& procsInfo) : mProcsInfo(procsInfo) {}
		~COpenGLGPUInternals() {}

	COpenGLGPUProcsInfo	mProcsInfo;
	S2DSize32			mSize;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - COpenGLGPU

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
COpenGLGPU::COpenGLGPU(const COpenGLGPUProcsInfo& procsInfo) : CGPU(procsInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new COpenGLGPUInternals(procsInfo);
}

//----------------------------------------------------------------------------------------------------------------------
COpenGLGPU::~COpenGLGPU()
//----------------------------------------------------------------------------------------------------------------------
{
	DisposeOf(mInternals);
}

// MARK: CGPU methods

//----------------------------------------------------------------------------------------------------------------------
void COpenGLGPU::setup(const S2DSize32& size)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mSize = size;

	// Setup
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
#if defined(DEBUG)
	glClearColor(0.0f, 1.0f, 0.0f, 1.0f);
#else
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
#endif

	glOrtho(0.0, mInternals->mSize.mWidth, mInternals->mSize.mHeight, 0.0, -1000.0f, 1000.0f);
	glTranslatef(0.375, 0.375, 0.0);
}

//----------------------------------------------------------------------------------------------------------------------
SGPUTextureInfo COpenGLGPU::registerTexture(const CGPUTexture& gpuTexture)
//----------------------------------------------------------------------------------------------------------------------
{
	// Register texture
	mInternals->mProcsInfo.acquireContext();
	SOpenGLTextureInfo*	openGLTextureInfo = new SOpenGLTextureInfo(gpuTexture);
	mInternals->mProcsInfo.releaseContext();

	return SGPUTextureInfo(gpuTexture.getGPUTextureSize(),
			(Float32) openGLTextureInfo->mUsedPixelsSize.mWidth /
					(Float32) openGLTextureInfo->mTotalPixelsSize.mWidth,
			(Float32) openGLTextureInfo->mUsedPixelsSize.mHeight /
					(Float32) openGLTextureInfo->mTotalPixelsSize.mHeight,
			openGLTextureInfo);
}

//----------------------------------------------------------------------------------------------------------------------
void COpenGLGPU::unregisterTexture(const SGPUTextureInfo& gpuTextureInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SOpenGLTextureInfo*	openGLTextureInfo = (SOpenGLTextureInfo*) gpuTextureInfo.mInternalReference;

	// Cleanup
//	acquireContext();
	mInternals->mProcsInfo.acquireContext();
	DisposeOf(openGLTextureInfo);
//	releaseContext();
	mInternals->mProcsInfo.releaseContext();
}

//----------------------------------------------------------------------------------------------------------------------
SGPUVertexBuffer COpenGLGPU::allocateVertexBuffer(EGPUVertexBufferType gpuVertexBufferType, UInt32 vertexCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt32	bytesPerVertex;
	switch (gpuVertexBufferType) {
		case kGPUVertexBufferType2Vertex2Texture:	bytesPerVertex = 4 * sizeof(Float32);	break;
		case kGPUVertexBufferType3Vertex2Texture:	bytesPerVertex = 5 * sizeof(Float32);	break;
	}

	return SGPUVertexBuffer(gpuVertexBufferType, CData(bytesPerVertex * vertexCount), nil);
}

//----------------------------------------------------------------------------------------------------------------------
void COpenGLGPU::disposeBuffer(const SGPUBuffer& buffer)
//----------------------------------------------------------------------------------------------------------------------
{
}

////----------------------------------------------------------------------------------------------------------------------
//void COpenGLGPU::useTexture(const SGPUTextureInfo& gpuTextureInfo) const
////----------------------------------------------------------------------------------------------------------------------
//{
//	//
//	SOpenGLTextureInfo*	openGLTextureInfo = (SOpenGLTextureInfo*) gpuTextureInfo.mInternalReference;
//
//	// Use this texture
//	glBindTexture(GL_TEXTURE_2D, openGLTextureInfo->mTextureName);
//
//	// Previous Notes:
//	// Use GL_SRC_ALPHA for non-premultiplied alpha, use GL_ONE for premultiplied alpha
//	// SceneApp from PNG sources generates premultiplied alpha
//
//	// Current Notes:
//	// There are only 2 kinds of textures that have transparency, PNG and Animations
//	// PNG:
//	//		on iOS, it doesn't seem to matter which blend func as they both look almost identical
//	//		on Android, GL_SRC_ALPHA works great, but GL_ONE doesn't use the transparency in the image
//	// Animation:
//	//		GL_SRC_ALPHA shows gray outlines around the clouds
//	//		GL_ONE does not show gray outlines around the clouds
//
//	// Conclusion:
//	//	CoreGraphics converts transparent PNGs to pre-multiplied when loading.
//	//	Since SceneApp Builder uses CoreGraphics to load PNGs and then simply processes them
//	//	further, animations come through with pre-multiplied alpha.  PNGs loaded directly on iOS
//	//	of course will also have pre-multiplied alpha.  PNGs loaded on Android through libpng
//	//	will *not* have pre-multiplied alpha, which I believe is correct.  For now, CBitmap on
//	//	Android will go ahead and transform to pre-multiplied alpha to maintain compatibility with
//	//	existing iOS functionality.  This needs to be fixed globally which should allow us to change
//	//	this blend func to be a single blend func globally and the removal of the need for the
//	//	option.
////	if (mInternals->mOptions & kCGLTextureOptionWillChangeAlpha)
////		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
////	else
//		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
//}

//----------------------------------------------------------------------------------------------------------------------
void COpenGLGPU::renderStart() const
//----------------------------------------------------------------------------------------------------------------------
{
//	acquireContext();
	mInternals->mProcsInfo.acquireContext();

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(0, 0, mInternals->mSize.mWidth, mInternals->mSize.mHeight);
	glClear(GL_COLOR_BUFFER_BIT);
}

//----------------------------------------------------------------------------------------------------------------------
void COpenGLGPU::renderSetClipPlane(Float32 clipPlane[4])
//----------------------------------------------------------------------------------------------------------------------
{
#if defined(TARGET_OS_MACOS)
	GLdouble	values[4] = {clipPlane[0], clipPlane[1], clipPlane[2], clipPlane[3]};
	glClipPlane(GL_CLIP_PLANE0, values);
#else
	glClipPlanef(GL_CLIP_PLANE0, clipPlane);
#endif
	glEnable(GL_CLIP_PLANE0);
}

//----------------------------------------------------------------------------------------------------------------------
void COpenGLGPU::renderClearClipPlane()
//----------------------------------------------------------------------------------------------------------------------
{
	glDisable(GL_CLIP_PLANE0);
}

//----------------------------------------------------------------------------------------------------------------------
void COpenGLGPU::renderTriangleStrip(const SGPUVertexBuffer& gpuVertexBuffer, UInt32 vertexCount,
		const SGPUTextureInfo& gpuTextureInfo, OV<Float32> alpha)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SOpenGLTextureInfo*	openGLTextureInfo = (SOpenGLTextureInfo*) gpuTextureInfo.mInternalReference;

	GLsizei				stride;
	UInt32				textureBufferOffset;
	switch (gpuVertexBuffer.mGPUVertexBufferType) {
		case kGPUVertexBufferType2Vertex2Texture:
			// 2 Vertex, 2 Texture
			stride = 4 * sizeof(Float32);
			textureBufferOffset = 2 * sizeof(Float32);
			break;

		case kGPUVertexBufferType3Vertex2Texture:
			// 3 Vertex, 2 Texture
			stride = 5 * sizeof(Float32);
			textureBufferOffset = 2 * sizeof(Float32);
			break;
	}

	// Call OpenGL
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(2, GL_FLOAT, stride, gpuVertexBuffer.mData.getBytePtr());

	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, stride, (UInt8*) gpuVertexBuffer.mData.getBytePtr() + textureBufferOffset);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, openGLTextureInfo->mTextureName);

	if (openGLTextureInfo->hasTransparency() || (alpha.hasValue() && (alpha.getValue() != 1.0))) {
		// Need to blend
		glEnable(GL_BLEND);
		glColor4f(1.0f, 1.0f, 1.0f, alpha.hasValue() ? alpha.getValue() : 1.0);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexCount);

	// Reset
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_TEXTURE_COORD_ARRAY);
	glDisable(GL_BLEND);
	glDisable(GL_TEXTURE_2D);
}

//----------------------------------------------------------------------------------------------------------------------
void COpenGLGPU::renderEnd() const
//----------------------------------------------------------------------------------------------------------------------
{
//	GLfloat	triangleVertices[] = {10.0, 10.0,	310.0, 10.0,	10.0, 470.0,	310.0, 470.0};
//	int		vertexCount = sizeof(triangleVertices) / sizeof(GLfloat) / 2;
//	glEnableClientState(GL_VERTEX_ARRAY);
//	glVertexPointer(2, GL_FLOAT, 0, triangleVertices);
//	glColor4f(0.9f, 0.3f, 0.3f, 0.5f);
//	glDrawArrays(GL_TRIANGLE_STRIP, 0, vertexCount);
//	glDisableClientState(GL_VERTEX_ARRAY);

//	releaseContext();
	mInternals->mProcsInfo.releaseContext();
}
