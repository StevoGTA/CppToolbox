//----------------------------------------------------------------------------------------------------------------------
//	COpenGLES11GPU.mm			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#import "COpenGLGPU.h"

#import "COpenGLES11GPU.h"
#import "COpenGLTextureInfo.h"

#import <OpenGLES/ES1/glext.h>
#import <QuartzCore/QuartzCore.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: COpenGLGPUInternals

class COpenGLGPUInternals {
	public:
		COpenGLGPUInternals(const COpenGLGPUProcsInfo& procsInfo) :
			mProcsInfo(procsInfo), mRenderBufferName(0), mFrameBufferName(0), mDepthBufferName(0)
			{}
		~COpenGLGPUInternals()
			{
				// Cleanup
				glDeleteRenderbuffersOES(1, &mDepthBufferName);
				glDeleteFramebuffersOES(1, &mFrameBufferName);
				glDeleteRenderbuffersOES(1, &mRenderBufferName);
			}

	COpenGLGPUProcsInfo	mProcsInfo;
	S2DSize32			mSize;
	GLuint				mRenderBufferName;
	GLuint				mFrameBufferName;
	GLuint				mDepthBufferName;
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
void COpenGLGPU::setup(const S2DSize32& size, void* setupInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SOpenGLES11GPUSetupInfo&	openGLES11GPUetupInfo = *((SOpenGLES11GPUSetupInfo*) setupInfo);

	// Store
	mInternals->mSize = size;

	// Setup
	if (mInternals->mDepthBufferName != 0) {
		glDeleteRenderbuffersOES(1, &mInternals->mDepthBufferName);
		mInternals->mDepthBufferName = 0;
	}
	if (mInternals->mFrameBufferName != 0) {
		glDeleteFramebuffersOES(1, &mInternals->mFrameBufferName);
		mInternals->mFrameBufferName = 0;
	}
	if (mInternals->mRenderBufferName != 0) {
		glDeleteRenderbuffersOES(1, &mInternals->mRenderBufferName);
		mInternals->mRenderBufferName = 0;
	}

	// Setup buffers
	glGenFramebuffersOES(1, &mInternals->mFrameBufferName);
	glBindFramebufferOES(GL_FRAMEBUFFER_OES, mInternals->mFrameBufferName);

	glGenRenderbuffersOES(1, &mInternals->mRenderBufferName);
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, mInternals->mRenderBufferName);
	[EAGLContext.currentContext renderbufferStorage:GL_RENDERBUFFER_OES
			fromDrawable:(__bridge id<EAGLDrawable>) openGLES11GPUetupInfo.mRenderBufferStorageContext];
	glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES,
			mInternals->mRenderBufferName);

	glGenRenderbuffersOES(1, &mInternals->mDepthBufferName);
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, mInternals->mDepthBufferName);
	glRenderbufferStorageOES(GL_RENDERBUFFER_OES, GL_DEPTH_COMPONENT16_OES,
			mInternals->mSize.mWidth, mInternals->mSize.mHeight);
	glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_DEPTH_ATTACHMENT_OES, GL_RENDERBUFFER_OES,
			mInternals->mDepthBufferName);

	// Setup
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
#if DEBUG
	glClearColor(0.5f, 0.0f, 0.25f, 1.0f);
#else
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
#endif

//	mViewportPointForScreenUL = CSceneAppPlayerX::shared().viewportPointFromScreenPoint(S2DPoint32::mZero);
//	mViewportPointForScreenBR =
//			CSceneAppPlayerX::shared().viewportPointFromScreenPoint(S2DPoint32(mWidth, mHeight));
//	glOrthof(mViewportPointForScreenUL.mX, mViewportPointForScreenBR.mX,
//			mViewportPointForScreenBR.mY, mViewportPointForScreenUL.mY, -1000.0f, 1000.0f);
	glOrthof(0.0, mInternals->mSize.mWidth / openGLES11GPUetupInfo.mScale,
			mInternals->mSize.mHeight / openGLES11GPUetupInfo.mScale, 0.0, -1000.0f, 1000.0f);
//	glOrthof(0.0, width, height, 0.0, -1000.0f, 1000.0f);
//	glTranslatef(0.375, 0.375, 0.0);
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
	mInternals->mProcsInfo.acquireContext();

	glBindFramebufferOES(GL_FRAMEBUFFER_OES, mInternals->mFrameBufferName);
	glViewport(0, 0, mInternals->mSize.mWidth, mInternals->mSize.mHeight);
	glClear(GL_COLOR_BUFFER_BIT);
}

//----------------------------------------------------------------------------------------------------------------------
void COpenGLGPU::renderSetClipPlane(Float32 clipPlane[4])
//----------------------------------------------------------------------------------------------------------------------
{
	glClipPlanef(GL_CLIP_PLANE0, clipPlane);
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
	COpenGLTextureInfo*	openGLTextureInfo = (COpenGLTextureInfo*) gpuTextureInfo.mInternalReference;

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
	glBindTexture(GL_TEXTURE_2D, openGLTextureInfo->getTextureName());

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
	// All done
	glBindRenderbufferOES(GL_RENDERBUFFER_OES, mInternals->mRenderBufferName);

	mInternals->mProcsInfo.releaseContext();
}
