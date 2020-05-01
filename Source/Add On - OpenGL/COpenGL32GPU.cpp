//----------------------------------------------------------------------------------------------------------------------
//	COpenGL32GPU.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "COpenGLGPU.h"

#include "COpenGLTexture.h"

#include <OpenGL/gl3.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPUInternals

class CGPUInternals {
	public:
		CGPUInternals(const CGPUProcsInfo& procsInfo) : mProcsInfo(procsInfo), mScale(1.0) {}

	CGPUProcsInfo	mProcsInfo;

	S2DSizeF32		mSize;
	Float32			mScale;
	SMatrix4x4_32	mProjectionMatrix;
	SMatrix4x4_32	mViewMatrix;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPU

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CGPU::CGPU(const CGPUProcsInfo& procsInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CGPUInternals(procsInfo);
}

//----------------------------------------------------------------------------------------------------------------------
CGPU::~CGPU()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CGPU methods

//----------------------------------------------------------------------------------------------------------------------
void CGPU::setup(const S2DSizeF32& size, void* extraData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SOpenGLGPUSetupInfo*	openGLGPUSetupInfo = (SOpenGLGPUSetupInfo*) extraData;

	// Store
	mInternals->mSize = size;
	mInternals->mScale = openGLGPUSetupInfo->mScale;
	mInternals->mProjectionMatrix =
			SMatrix4x4_32::makeOrthographicProjection(0.0, mInternals->mSize.mWidth, mInternals->mSize.mHeight, 0.0,
					-1.0, 1.0);
}

//----------------------------------------------------------------------------------------------------------------------
SGPUTextureReference CGPU::registerTexture(const CData& data, EGPUTextureDataFormat gpuTextureDataFormat,
		const S2DSizeU16& size)
//----------------------------------------------------------------------------------------------------------------------
{
	// Register texture
	mInternals->mProcsInfo.acquireContext();
	CGPUTexture*	gpuTexture = new COpenGLTexture(data, gpuTextureDataFormat, size);
	mInternals->mProcsInfo.releaseContext();

	return SGPUTextureReference(*gpuTexture);
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::unregisterTexture(SGPUTextureReference& gpuTexture)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	COpenGLTexture*	openGLTexture = (COpenGLTexture*) gpuTexture.mGPUTexture;
	gpuTexture.reset();

	// Cleanup
	mInternals->mProcsInfo.acquireContext();
	Delete(openGLTexture);
	mInternals->mProcsInfo.releaseContext();
}

//----------------------------------------------------------------------------------------------------------------------
SGPUVertexBuffer CGPU::allocateVertexBuffer(const SGPUVertexBufferInfo& gpuVertexBufferInfo, UInt32 vertexCount)
//----------------------------------------------------------------------------------------------------------------------
{
	return SGPUVertexBuffer(gpuVertexBufferInfo, CData(gpuVertexBufferInfo.mTotalSize * vertexCount), nil);
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::disposeBuffer(const SGPUBuffer& buffer)
//----------------------------------------------------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::renderStart() const
//----------------------------------------------------------------------------------------------------------------------
{
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
void CGPU::setViewMatrix(const SMatrix4x4_32& viewMatrix)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mViewMatrix = viewMatrix;
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::renderTriangleStrip(CGPUProgram& program, const SMatrix4x4_32& modelMatrix, UInt32 triangleCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup matrices
	program.setup(mInternals->mViewMatrix, mInternals->mProjectionMatrix);
	program.setModelMatrix(modelMatrix);

	// Draw
	glDrawArrays(GL_TRIANGLE_STRIP, 0, triangleCount + 2);

	program.didFinish();
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::renderEnd() const
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mProcsInfo.releaseContext();
}
