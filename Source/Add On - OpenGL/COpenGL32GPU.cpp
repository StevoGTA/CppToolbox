//----------------------------------------------------------------------------------------------------------------------
//	COpenGL32GPU.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "COpenGLGPU.h"

#include "CMatrix.h"
#include "COpenGLBuiltIns.h"
#include "COpenGLTextureInfo.h"

#include <OpenGL/gl3.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: COpenGLGPUInternals

class COpenGLGPUInternals {
	public:
		COpenGLGPUInternals(const COpenGLGPUProcsInfo& procsInfo) : mProcsInfo(procsInfo), mScale(1.0) {}
		~COpenGLGPUInternals() {}

	COpenGLGPUProcsInfo	mProcsInfo;

	S2DSize32			mSize;
	Float32				mScale;
	SMatrix4x4_32		mProjectionMatrix;
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
	// Setup
	SOpenGLGPUSetupInfo*	openGLGPUSetupInfo = (SOpenGLGPUSetupInfo*) extraData;

	// Store
	mInternals->mSize = size;
	mInternals->mScale = openGLGPUSetupInfo->mScale;
	mInternals->mProjectionMatrix =
			SMatrix4x4_32(0.0, mInternals->mSize.mWidth, mInternals->mSize.mHeight, 0.0, -1.0, 1.0);
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

	glViewport(0, 0, mInternals->mSize.mWidth * (GLsizei) mInternals->mScale,
			mInternals->mSize.mHeight * (GLsizei) mInternals->mScale);
#if defined(DEBUG)
	glClearColor(0.5, 0.0, 0.25, 1.0);
#else
	glClearColor(0.0, 0.0, 0.0, 1.0);
#endif
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
	GLvoid*				textureBufferOffset;
	switch (gpuVertexBuffer.mGPUVertexBufferType) {
		case kGPUVertexBufferType2Vertex2Texture:
			// 2 Vertex, 2 Texture
			vertexCount = 2;
			stride = 4 * sizeof(Float32);
			textureBufferOffset = (GLvoid*) (2 * sizeof(Float32));
			break;

		case kGPUVertexBufferType3Vertex2Texture:
			// 3 Vertex, 2 Texture
			vertexCount = 3;
			stride = 5 * sizeof(Float32);
			textureBufferOffset = (GLvoid*) (3 * sizeof(Float32));
			break;
	}

	// Setup buffers
	GLuint	vertexArray;
	glGenVertexArrays(1, &vertexArray);
	glBindVertexArray(vertexArray);

	GLuint	modelDataBuffer;
	glGenBuffers(1, &modelDataBuffer);

	glBindBuffer(GL_ARRAY_BUFFER, modelDataBuffer);
	glBufferData(GL_ARRAY_BUFFER, gpuVertexBuffer.mData.getSize(), gpuVertexBuffer.mData.getBytePtr(), GL_STATIC_DRAW);

	// Setup texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, openGLTextureInfo->getTextureName());

	// Get info
	const	COpenGLProgram&	program = COpenGLBuiltIns::getOpacityProgram();
	program.use();

	int	attributeLocation = program.getAttributeLocation(CString(OSSTR("position")));
	glEnableVertexAttribArray(attributeLocation);
	glVertexAttribPointer(attributeLocation, vertexCount, GL_FLOAT, GL_FALSE, stride, 0);

	attributeLocation = program.getAttributeLocation(CString(OSSTR("texCoord0")));
	glEnableVertexAttribArray(attributeLocation);
	glVertexAttribPointer(attributeLocation, 2, GL_FLOAT, GL_FALSE, stride, textureBufferOffset);

	int	mvpMatrixLocation = program.getUniformLocation(CString(OSSTR("modelViewProjectionMatrix")));
	int	textureLocation = program.getUniformLocation(CString(OSSTR("diffuseTexture")));
	int	opacityLocation = program.getUniformLocation(CString(OSSTR("opacity")));

	// Setup matrices
	SMatrix4x4_32	modelMatrix;
    SMatrix4x4_32	modelViewProjectionMatrix = mInternals->mProjectionMatrix * modelMatrix;
    glUniformMatrix4fv(mvpMatrixLocation, 1, 0, (GLfloat*) &modelViewProjectionMatrix);

    glUniform1i(textureLocation, 0);

    // Setup opacity
    glUniform1f(opacityLocation, alpha.hasValue() ? alpha.getValue() : 1.0);
	if (openGLTextureInfo->hasTransparency() || (alpha.hasValue() && (alpha.getValue() != 1.0))) {
		// Need to blend
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	// Draw
	glDrawArrays(GL_TRIANGLE_STRIP, 0, triangleCount + 2);

	// Reset
	glDeleteBuffers(1, &modelDataBuffer);
	glDeleteVertexArrays(1, &vertexArray);
	glDisable(GL_BLEND);
}

//----------------------------------------------------------------------------------------------------------------------
void COpenGLGPU::renderEnd() const
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mProcsInfo.releaseContext();
}
