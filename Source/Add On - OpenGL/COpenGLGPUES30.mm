//----------------------------------------------------------------------------------------------------------------------
//	COpenGLGPUES30.mm			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#import "COpenGLGPU.h"

#import "COpenGLRenderState.h"
#import "COpenGLTexture.h"

#import <OpenGLES/ES3/glext.h>
#import <QuartzCore/QuartzCore.h>

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUInternals

class CGPUInternals {
	public:
				CGPUInternals(const CGPUProcsInfo& procsInfo) :
					mProcsInfo(procsInfo), mPerformedSetup(false), mRenderBufferName(0), mFrameBufferName(0)
					{}
				~CGPUInternals()
					{
						// Cleanup
						glDeleteFramebuffers(1, &mFrameBufferName);
						glDeleteRenderbuffers(1, &mRenderBufferName);
					}

	CGPUProcsInfo	mProcsInfo;

	bool			mPerformedSetup;
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
	// Get info
	S2DSizeU16	size = mInternals->mProcsInfo.getSize();
	Float32		scale = mInternals->mProcsInfo.getScale();

	// Check if performed setup
	if (!mInternals->mPerformedSetup) {
		// Update
		mInternals->mProjectionMatrix =
				SMatrix4x4_32(
						2.0 / size.mWidth, 0.0, 0.0, 0.0,
						0.0, 2.0 / -size.mHeight, 0.0, 0.0,
						0.0, 0.0, -1.0, 0.0,
						-1.0, 1.0, 0.0, 1.0);

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
				fromDrawable:(__bridge id<EAGLDrawable>) mInternals->mProcsInfo.getRenderBufferStorageContext()];
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, mInternals->mRenderBufferName);

#if DEBUG
		glClearColor(0.5, 0.0, 0.25, 1.0);
#else
		glClearColor(0.0, 0.0, 0.0, 1.0);
#endif

		mInternals->mPerformedSetup = true;
	}

	// Prepare to render
	mInternals->mProcsInfo.acquireContext();

	glBindFramebuffer(GL_FRAMEBUFFER, mInternals->mFrameBufferName);
	glViewport(0, 0, size.mWidth * scale, size.mHeight * scale);
	glClear(GL_COLOR_BUFFER_BIT);
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::setViewMatrix(const SMatrix4x4_32& viewMatrix)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mViewMatrix = viewMatrix;
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::renderTriangleStrip(CGPURenderState& renderState, const SMatrix4x4_32& modelMatrix, UInt32 triangleCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup render state
	renderState.setProjectionMatrix(mInternals->mProjectionMatrix);
	renderState.setViewMatrix(mInternals->mViewMatrix);
	renderState.setModelMatrix(modelMatrix);

	// Commit
	renderState.commit(SGPURenderStateCommitInfo());

	// Draw
	glDrawArrays(GL_TRIANGLE_STRIP, 0, triangleCount + 2);
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::renderEnd() const
//----------------------------------------------------------------------------------------------------------------------
{
	// All done
	glBindRenderbuffer(GL_RENDERBUFFER, mInternals->mRenderBufferName);

	mInternals->mProcsInfo.releaseContext();
}
