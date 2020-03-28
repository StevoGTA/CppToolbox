//----------------------------------------------------------------------------------------------------------------------
//	COpenGLES11GPU.mm			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#import "COpenGLGPU.h"

#import "COpenGLTextureInfo.h"

#import <OpenGLES/ES1/glext.h>
#import <QuartzCore/QuartzCore.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: COpenGLGPUInternals

class COpenGLGPUInternals {
	public:
		COpenGLGPUInternals(const COpenGLGPUProcsInfo& procsInfo) :
			mProcsInfo(procsInfo), mRenderBufferName(0), mFrameBufferName(0)
			{}
		~COpenGLGPUInternals()
			{
				// Cleanup
				glDeleteFramebuffersOES(1, &mFrameBufferName);
				glDeleteRenderbuffersOES(1, &mRenderBufferName);
			}

	COpenGLGPUProcsInfo	mProcsInfo;
	S2DSize32			mSize;
	GLuint				mRenderBufferName;
	GLuint				mFrameBufferName;
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
	SOpenGLESGPUSetupInfo&	openGLESGPUetupInfo = *((SOpenGLESGPUSetupInfo*) setupInfo);

	// Store
	mInternals->mSize = size;

	// Setup
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
			fromDrawable:(__bridge id<EAGLDrawable>) openGLESGPUetupInfo.mRenderBufferStorageContext];
	glFramebufferRenderbufferOES(GL_FRAMEBUFFER_OES, GL_COLOR_ATTACHMENT0_OES, GL_RENDERBUFFER_OES,
			mInternals->mRenderBufferName);

	// Setup
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
#if DEBUG
	glClearColor(0.5, 0.0, 0.25, 1.0);
#else
	glClearColor(0.0, 0.0, 0.0, 1.0);
#endif

	glOrthof(0.0, mInternals->mSize.mWidth / openGLESGPUetupInfo.mScale,
			mInternals->mSize.mHeight / openGLESGPUetupInfo.mScale, 0.0, -1000.0f, 1000.0f);
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
			textureBufferOffset = 3 * sizeof(Float32);
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

	glDrawArrays(GL_TRIANGLE_STRIP, 0, triangleCount + 2);

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
