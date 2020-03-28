//----------------------------------------------------------------------------------------------------------------------
//	COpenGL11GPU.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "COpenGLGPU.h"

#include "COpenGLTextureInfo.h"

#include <OpenGL/gl.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: COpenGLGPUInternals

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
void COpenGLGPU::setup(const S2DSize32& size, void* extraData)
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
	COpenGLTextureInfo*	openGLTextureInfo = new COpenGLTextureInfo(gpuTexture);
	mInternals->mProcsInfo.releaseContext();

	return SGPUTextureInfo(gpuTexture.getGPUTextureSize(),
			(Float32) openGLTextureInfo->getUsedPixelsGPUTextureSize().mWidth /
					(Float32) openGLTextureInfo->getTotalPixelsGPUTextureSize().mWidth,
			(Float32) openGLTextureInfo->getUsedPixelsGPUTextureSize().mHeight /
					(Float32) openGLTextureInfo->getTotalPixelsGPUTextureSize().mHeight,
			openGLTextureInfo);
}

//----------------------------------------------------------------------------------------------------------------------
void COpenGLGPU::unregisterTexture(const SGPUTextureInfo& gpuTextureInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	COpenGLTextureInfo*	openGLTextureInfo = (COpenGLTextureInfo*) gpuTextureInfo.mInternalReference;

	// Cleanup
	mInternals->mProcsInfo.acquireContext();
	DisposeOf(openGLTextureInfo);
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
	GLdouble	values[4] = {clipPlane[0], clipPlane[1], clipPlane[2], clipPlane[3]};
	glClipPlane(GL_CLIP_PLANE0, values);
	glEnable(GL_CLIP_PLANE0);
}

//----------------------------------------------------------------------------------------------------------------------
void COpenGLGPU::renderClearClipPlane()
//----------------------------------------------------------------------------------------------------------------------
{
	glDisable(GL_CLIP_PLANE0);
}

//----------------------------------------------------------------------------------------------------------------------
void COpenGLGPU::renderTriangleStrip(const SGPUVertexBuffer& gpuVertexBuffer, UInt32 triangleCount,
		const SGPUTextureInfo& gpuTextureInfo, OV<Float32> alpha)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	COpenGLTextureInfo*	openGLTextureInfo = (COpenGLTextureInfo*) gpuTextureInfo.mInternalReference;

	GLint				vertexCount;
	GLsizei				stride;
	UInt32				textureBufferOffset;
	switch (gpuVertexBuffer.mGPUVertexBufferType) {
		case kGPUVertexBufferType2Vertex2Texture:
			// 2 Vertex, 2 Texture
			vertexCount = 2;
			stride = 4 * sizeof(Float32);
			textureBufferOffset = 2 * sizeof(Float32);
			break;

		case kGPUVertexBufferType3Vertex2Texture:
			// 3 Vertex, 2 Texture
			vertexCount = 3;
			stride = 5 * sizeof(Float32);
			textureBufferOffset = 2 * sizeof(Float32);
			break;
	}

	// Call OpenGL
	glEnableClientState(GL_VERTEX_ARRAY);
	glVertexPointer(vertexCount, GL_FLOAT, stride, gpuVertexBuffer.mData.getBytePtr());

	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2, GL_FLOAT, stride, (UInt8*) gpuVertexBuffer.mData.getBytePtr() + textureBufferOffset);

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, openGLTextureInfo->getTextureName());

	if (openGLTextureInfo->hasTransparency() || (alpha.hasValue() && (alpha.getValue() != 1.0))) {
		// Need to blend
		glEnable(GL_BLEND);
		glColor4f(1.0f, 1.0f, 1.0f, alpha.hasValue() ? alpha.getValue() : 1.0);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	glDrawArrays(GL_TRIANGLE_STRIP, 0, triangleCount);

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
	mInternals->mProcsInfo.releaseContext();
}
