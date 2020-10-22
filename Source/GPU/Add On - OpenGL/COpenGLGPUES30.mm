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
				CGPUInternals(const SGPUProcsInfo& procsInfo) :
					mProcsInfo(procsInfo), mPerformedSetup(false), mRenderBufferName(0), mFrameBufferName(0)
					{}
				~CGPUInternals()
					{
						// Cleanup
						glDeleteFramebuffers(1, &mFrameBufferName);
						glDeleteRenderbuffers(1, &mRenderBufferName);
					}

	SGPUProcsInfo	mProcsInfo;

	bool			mPerformedSetup;
	GLuint			mRenderBufferName;
	GLuint			mFrameBufferName;

	SMatrix4x4_32	mViewMatrix2D;
	SMatrix4x4_32	mProjectionMatrix2D;

	SMatrix4x4_32	mViewMatrix3D;
	SMatrix4x4_32	mProjectionMatrix3D;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPU

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CGPU::CGPU(const SGPUProcsInfo& procsInfo)
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
SGPUBuffer CGPU::allocateIndexBuffer(const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
}

//----------------------------------------------------------------------------------------------------------------------
SGPUVertexBuffer CGPU::allocateVertexBuffer(const SGPUVertexBufferInfo& gpuVertexBufferInfo, const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mProcsInfo.acquireContext();
	SGPUVertexBuffer gpuVertexBuffer(gpuVertexBufferInfo, new SOpenGLVertexBufferInfo(data));
	mInternals->mProcsInfo.releaseContext();

	return gpuVertexBuffer;
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::disposeBuffer(const SGPUBuffer& buffer)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals->mProcsInfo.acquireContext();
	SOpenGLVertexBufferInfo*	openGLVertexBufferInfo = (SOpenGLVertexBufferInfo*) buffer.mInternalReference;
	Delete(openGLVertexBufferInfo);
	mInternals->mProcsInfo.releaseContext();
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::renderStart(const S2DSizeF32& size2D, Float32 fieldOfViewAngle3D, Float32 aspectRatio3D, Float32 nearZ3D,
		Float32 farZ3D, const S3DPointF32& camera3D, const S3DPointF32& target3D, const S3DPointF32& up3D) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get info
	S2DSizeU16	viewSize = mInternals->mProcsInfo.getSize();
	Float32		scale = mInternals->mProcsInfo.getScale();

	// Check if performed setup
	if (!mInternals->mPerformedSetup) {
		// Update
		mInternals->mProjectionMatrix2D =
				SMatrix4x4_32(
						2.0 / size2D.mWidth, 0.0, 0.0, 0.0,
						0.0, 2.0 / -size2D.mHeight, 0.0, 0.0,
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
	glViewport(0, 0, viewSize.mWidth * scale, viewSize.mHeight * scale);
	glClear(GL_COLOR_BUFFER_BIT);
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::render(CGPURenderState& renderState, EGPURenderType type, UInt32 count, UInt32 offset)
//----------------------------------------------------------------------------------------------------------------------
{
	// Finalize render state
	switch (renderState.getRenderMode()) {
		case kGPURenderMode2D:
			// 2D
			renderState.commit(SGPURenderStateCommitInfo(mInternals->mViewMatrix2D, mInternals->mProjectionMatrix2D));
			break;

		case kGPURenderMode3D:
			// 3D
			renderState.commit(SGPURenderStateCommitInfo(mInternals->mViewMatrix3D, mInternals->mProjectionMatrix3D));
			break;
	}

	// Check type
	switch (type) {
		case kGPURenderTypeTriangleList:
			// Triangle list
			AssertFailUnimplemented();
			break;

		case kGPURenderTypeTriangleStrip:
			// Triangle strip
			glDrawArrays(GL_TRIANGLE_STRIP, offset, count);
			break;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::renderIndexed(CGPURenderState& renderState, EGPURenderType type, UInt32 count, UInt32 offset)
//----------------------------------------------------------------------------------------------------------------------
{
	// Finalize render state
	switch (renderState.getRenderMode()) {
		case kGPURenderMode2D:
			// 2D
			renderState.commit(SGPURenderStateCommitInfo(mInternals->mViewMatrix2D, mInternals->mProjectionMatrix2D));
			break;

		case kGPURenderMode3D:
			// 3D
			renderState.commit(SGPURenderStateCommitInfo(mInternals->mViewMatrix3D, mInternals->mProjectionMatrix3D));
			break;
	}

	// Check type
	switch (type) {
		case kGPURenderTypeTriangleList:
			// Triangle list
			AssertFailUnimplemented();
			break;

		case kGPURenderTypeTriangleStrip:
			// Triangle strip
			AssertFailUnimplemented();
			break;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::renderEnd() const
//----------------------------------------------------------------------------------------------------------------------
{
	// All done
	glBindRenderbuffer(GL_RENDERBUFFER, mInternals->mRenderBufferName);

	mInternals->mProcsInfo.releaseContext();
}
