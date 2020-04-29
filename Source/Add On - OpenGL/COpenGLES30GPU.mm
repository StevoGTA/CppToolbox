//----------------------------------------------------------------------------------------------------------------------
//	COpenGLES30GPU.mm			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#import "COpenGLGPU.h"

#import "COpenGLTextureInfo.h"

#import <OpenGLES/ES3/glext.h>
#import <QuartzCore/QuartzCore.h>

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUInternals

class CGPUInternals {
	public:
				CGPUInternals(const CGPUProcsInfo& procsInfo) :
					mProcsInfo(procsInfo), mScale(1.0), mRenderBufferName(0), mFrameBufferName(0)
					{}
				~CGPUInternals()
					{
						// Cleanup
						glDeleteFramebuffers(1, &mFrameBufferName);
						glDeleteRenderbuffers(1, &mRenderBufferName);
					}

	CGPUProcsInfo	mProcsInfo;

	S2DSize32		mSize;
	Float32			mScale;
	SMatrix4x4_32	mProjectionMatrix;
	SMatrix4x4_32	mViewMatrix;
	GLuint			mRenderBufferName;
	GLuint			mFrameBufferName;
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
void CGPU::setup(const S2DSize32& size, void* setupInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SOpenGLESGPUSetupInfo&	openGLESGPUetupInfo = *((SOpenGLESGPUSetupInfo*) setupInfo);

	// Store
	mInternals->mSize = size;
	mInternals->mScale = openGLESGPUetupInfo.mScale;
	mInternals->mProjectionMatrix =
			SMatrix4x4_32::makeOrthographicProjection(0.0, mInternals->mSize.mWidth / mInternals->mScale,
					mInternals->mSize.mHeight / mInternals->mScale, 0.0, -1.0, 1.0);

	// Setup
	if (mInternals->mFrameBufferName != 0) {
		glDeleteFramebuffers(1, &mInternals->mFrameBufferName);
		mInternals->mFrameBufferName = 0;
	}
	if (mInternals->mRenderBufferName != 0) {
		glDeleteRenderbuffers(1, &mInternals->mRenderBufferName);
		mInternals->mRenderBufferName = 0;
	}

	// Setup buffers
	glGenFramebuffers(1, &mInternals->mFrameBufferName);
	glBindFramebuffer(GL_FRAMEBUFFER, mInternals->mFrameBufferName);

	glGenRenderbuffers(1, &mInternals->mRenderBufferName);
	glBindRenderbuffer(GL_RENDERBUFFER, mInternals->mRenderBufferName);
	[EAGLContext.currentContext renderbufferStorage:GL_RENDERBUFFER
			fromDrawable:(__bridge id<EAGLDrawable>) openGLESGPUetupInfo.mRenderBufferStorageContext];
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, mInternals->mRenderBufferName);

#if DEBUG
	glClearColor(0.5, 0.0, 0.25, 1.0);
#else
	glClearColor(0.0, 0.0, 0.0, 1.0);
#endif
}

//----------------------------------------------------------------------------------------------------------------------
SGPUTextureInfo CGPU::registerTexture(const CGPUTexture& gpuTexture)
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
void CGPU::unregisterTexture(const SGPUTextureInfo& gpuTextureInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	COpenGLTextureInfo*	openGLTextureInfo = (COpenGLTextureInfo*) gpuTextureInfo.mInternalReference;

	// Cleanup
	mInternals->mProcsInfo.acquireContext();
	Delete(openGLTextureInfo);
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

	glBindFramebuffer(GL_FRAMEBUFFER, mInternals->mFrameBufferName);
	glViewport(0, 0, mInternals->mSize.mWidth, mInternals->mSize.mHeight);
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
	// All done
	glBindRenderbuffer(GL_RENDERBUFFER, mInternals->mRenderBufferName);

	mInternals->mProcsInfo.releaseContext();
}
