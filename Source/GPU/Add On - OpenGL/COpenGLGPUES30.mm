//----------------------------------------------------------------------------------------------------------------------
//	COpenGLGPUES30.mm			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#import "COpenGLGPU.h"

#import "CLogServices.h"
#import "COpenGLRenderState.h"
#import "COpenGLTexture.h"

#import <OpenGLES/ES3/glext.h>
#import <QuartzCore/QuartzCore.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPU::Internals

class CGPU::Internals {
	public:
				Internals(const Procs& procs) :
					mProcs(procs)
					{
						// Setup buffers
						glGenFramebuffers(1, &mFrameBufferName);
						glBindFramebuffer(GL_FRAMEBUFFER, mFrameBufferName);

						glGenRenderbuffers(1, &mRenderBufferName);
						glBindRenderbuffer(GL_RENDERBUFFER, mRenderBufferName);
						[EAGLContext.currentContext renderbufferStorage:GL_RENDERBUFFER
								fromDrawable:(__bridge id<EAGLDrawable>) mProcs.getRenderBufferStorageContext()];
						glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER,
								mRenderBufferName);

#if DEBUG
						glClearColor(0.5, 0.0, 0.25, 1.0);
#else
						glClearColor(0.0, 0.0, 0.0, 1.0);
#endif
					}
				~Internals()
					{
						// Cleanup
						glDeleteFramebuffers(1, &mFrameBufferName);
						glDeleteRenderbuffers(1, &mRenderBufferName);

						// Cleanup
						if (mOpenGLTextureCache.hasInstance())
							// Release
							::CFRelease(*mOpenGLTextureCache);
					}

	Procs							mProcs;

	GLuint							mFrameBufferName;
	GLuint							mRenderBufferName;

	SMatrix4x4_32					mViewMatrix2D;
	SMatrix4x4_32					mProjectionMatrix2D;

	SMatrix4x4_32					mViewMatrix3D;
	SMatrix4x4_32					mProjectionMatrix3D;

	OI<CVOpenGLESTextureCacheRef>	mOpenGLTextureCache;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPU

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CGPU::CGPU(const Procs& procs)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(procs);
}

//----------------------------------------------------------------------------------------------------------------------
CGPU::~CGPU()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CGPU methods

//----------------------------------------------------------------------------------------------------------------------
CVideoFrame::Compatibility CGPU::getVideoFrameCompatibility() const
//----------------------------------------------------------------------------------------------------------------------
{
	return CVideoFrame::kCompatibilityOpenGLES;
}

//----------------------------------------------------------------------------------------------------------------------
I<CGPUTexture> CGPU::registerTexture(const CData& data, CGPUTexture::DataFormat dataFormat, const S2DSizeU16& size)
//----------------------------------------------------------------------------------------------------------------------
{
	// Register texture
	mInternals->mProcs.acquireContext();
	I<CGPUTexture>	gpuTexture = I<CGPUTexture>(new COpenGLTexture(data, dataFormat, size));
	mInternals->mProcs.releaseContext();

	return gpuTexture;
}

//----------------------------------------------------------------------------------------------------------------------
TArray<I<CGPUTexture> > CGPU::registerTextures(const CVideoFrame& videoFrame)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals->mProcs.acquireContext();

	// Setup OpenGL Texture Cache
	if (!mInternals->mOpenGLTextureCache.hasInstance()) {
		// Create
		CVOpenGLESTextureCacheRef	openGLTextureCacheRef;
		CVReturn					result =
											::CVOpenGLESTextureCacheCreate(kCFAllocatorDefault, nil,
													mInternals->mProcs.getContext(), nil, &openGLTextureCacheRef);
		if (result != kCVReturnSuccess) {
			// Error
			CLogServices::logError(CString(OSSTR("COpenGLESGPU - error when creating texture cache: ")) + CString(result));

			return TNArray<I<CGPUTexture> >();
		}

		// Store
		mInternals->mOpenGLTextureCache = OI<CVOpenGLESTextureCacheRef>(openGLTextureCacheRef);
	}

	// Load textures
	CVImageBufferRef			imageBufferRef = videoFrame.getImageBufferRef();
	UInt32						planeCount =
										::CVPixelBufferIsPlanar(imageBufferRef) ?
												(UInt32) ::CVPixelBufferGetPlaneCount(imageBufferRef) : 1;
	TNArray<I<CGPUTexture> >	textures;
	for (UInt32 i = 0; i < planeCount; i++)
		// Add texture
		textures += I<CGPUTexture>(new COpenGLTexture(*mInternals->mOpenGLTextureCache, imageBufferRef, i));

	// Reset
	mInternals->mProcs.releaseContext();

	return textures;
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::unregisterTexture(const I<CGPUTexture>& gpuTexture)
//----------------------------------------------------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------------------------------------------------
SGPUVertexBuffer CGPU::allocateVertexBuffer(UInt32 perVertexByteCount, const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mProcs.acquireContext();
	SGPUVertexBuffer	gpuVertexBuffer(perVertexByteCount, new COpenGLVertexBufferInfo(data));
	mInternals->mProcs.releaseContext();

	return gpuVertexBuffer;
}

//----------------------------------------------------------------------------------------------------------------------
SGPUBuffer CGPU::allocateIndexBuffer(const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mProcs.acquireContext();
	SGPUBuffer	gpuBuffer(new COpenGLIndexBufferInfo(data));
	mInternals->mProcs.releaseContext();

	return gpuBuffer;
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::disposeBuffer(const SGPUBuffer& buffer)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals->mProcs.acquireContext();
	COpenGLBufferInfo*	openGLBufferInfo = (COpenGLBufferInfo*) buffer.mPlatformReference;
	Delete(openGLBufferInfo);
	mInternals->mProcs.releaseContext();
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::renderStart(const S2DSizeF32& size2D, Float32 fieldOfViewAngle3D, Float32 aspectRatio3D, Float32 nearZ3D,
		Float32 farZ3D, const S3DPointF32& camera3D, const S3DPointF32& target3D, const S3DVectorF32& up3D) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get info
	S2DSizeU16	viewSize = mInternals->mProcs.getSize();
	Float32		scale = mInternals->mProcs.getScale();

	// Projection 2D
	mInternals->mProjectionMatrix2D =
			SMatrix4x4_32(
					2.0 / size2D.mWidth, 0.0, 0.0, 0.0,
					0.0, -2.0 / size2D.mHeight, 0.0, 0.0,
					0.0, 0.0, -1.0, 0.0,
					-1.0, 1.0, 0.0, 1.0);

	// View 3D
	S3DVectorF32	camera3DVector = S3DVectorF32(camera3D.mX, camera3D.mY, camera3D.mZ);
	S3DVectorF32	target3DVector = S3DVectorF32(target3D.mX, target3D.mY, target3D.mZ);
	S3DVectorF32	f = (target3DVector - camera3DVector).normalized();
	S3DVectorF32	s = f.crossed(up3D).normalized();
	S3DVectorF32	t = s.crossed(f);
	mInternals->mViewMatrix3D =
			SMatrix4x4_32(
					s.mDX, t.mDX, -f.mDX, 0.0,
					s.mDY, t.mDY, -f.mDY, 0.0,
					s.mDZ, t.mDZ, -f.mDZ, 0.0,
					0.0, 0.0, 0.0, 1.0)
			.translated(S3DOffsetF32(-camera3D.mX, -camera3D.mY, -camera3D.mZ));

	// Projection 3D
	Float32	ys = 1.0 / tanf(fieldOfViewAngle3D * 0.5);
	Float32	xs = ys / aspectRatio3D;
	Float32	zs = farZ3D / (nearZ3D - farZ3D);
	mInternals->mProjectionMatrix3D =
			SMatrix4x4_32(
					xs,		0.0,	0.0,			0.0,
					0.0,	ys,		0.0,			0.0,
					0.0,	0.0,	zs,				-1.0,
					0.0,	0.0,	nearZ3D * zs,	0.0);

	// Prepare to render
	mInternals->mProcs.acquireContext();

	glBindFramebuffer(GL_FRAMEBUFFER, mInternals->mFrameBufferName);
	glViewport(0, 0, viewSize.mWidth * scale, viewSize.mHeight * scale);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Check if have texture cache
	if (mInternals->mOpenGLTextureCache.hasInstance())
		// Flush
		::CVOpenGLESTextureCacheFlush(*mInternals->mOpenGLTextureCache, 0);
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::render(CGPURenderState& renderState, RenderType renderType, UInt32 count, UInt32 offset)
//----------------------------------------------------------------------------------------------------------------------
{
	// Finalize render state
	switch (renderState.getMode()) {
		case CGPURenderState::kMode2D:
			// 2D
			renderState.commit(CommitInfo(mInternals->mViewMatrix2D, mInternals->mProjectionMatrix2D));
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);
			break;

		case CGPURenderState::kMode3D:
			// 3D
			renderState.commit(CommitInfo(mInternals->mViewMatrix3D, mInternals->mProjectionMatrix3D));
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glFrontFace(GL_CW);
			break;
	}

	// Check type
	switch (renderType) {
		case kRenderTypeTriangleList:
			// Triangle list
			glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, (GLvoid*) (GLintptr) offset);
			break;

		case kRenderTypeTriangleStrip:
			// Triangle strip
			glDrawArrays(GL_TRIANGLE_STRIP, offset, count);
			break;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::renderIndexed(CGPURenderState& renderState, RenderType renderType, UInt32 count, UInt32 offset)
//----------------------------------------------------------------------------------------------------------------------
{
	// Finalize render state
	switch (renderState.getMode()) {
		case CGPURenderState::kMode2D:
			// 2D
			renderState.commit(CommitInfo(mInternals->mViewMatrix2D, mInternals->mProjectionMatrix2D));
			glDisable(GL_DEPTH_TEST);
			glDisable(GL_CULL_FACE);
			break;

		case CGPURenderState::kMode3D:
			// 3D
			renderState.commit(CommitInfo(mInternals->mViewMatrix3D, mInternals->mProjectionMatrix3D));
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glFrontFace(GL_CW);

			break;
	}

	// Check type
	switch (renderType) {
		case kRenderTypeTriangleList:
			// Triangle list
			glDrawElements(GL_TRIANGLES, count, GL_UNSIGNED_SHORT, (GLvoid*) (GLintptr) offset);
			break;

		case kRenderTypeTriangleStrip:
			// Triangle strip
			glDrawArrays(GL_TRIANGLE_STRIP, offset, count);
			break;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::renderEnd() const
//----------------------------------------------------------------------------------------------------------------------
{
	// All done
	glBindRenderbuffer(GL_RENDERBUFFER, mInternals->mRenderBufferName);

	mInternals->mProcs.releaseContext();
}
