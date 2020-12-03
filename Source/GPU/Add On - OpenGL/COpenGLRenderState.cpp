//----------------------------------------------------------------------------------------------------------------------
//	COpenGLRenderState.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CGPURenderState.h"

#include "CDictionary.h"
#include "COpenGLProgram.h"
#include "COpenGLRenderState.h"
#include "COpenGLTexture.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPURenderStateInternals

class CGPURenderStateInternals {
	public:
		CGPURenderStateInternals(EGPURenderMode renderMode, COpenGLVertexShader& vertexShader,
				COpenGLFragmentShader& fragmentShader) :
			mRenderMode(renderMode), mVertexShader(vertexShader), mFragmentShader(fragmentShader)
			{}

		EGPURenderMode							mRenderMode;
		COpenGLVertexShader&					mVertexShader;
		COpenGLFragmentShader&					mFragmentShader;

		SMatrix4x4_32							mModelMatrix;

		OR<const SGPUVertexBuffer>				mVertexBuffer;
		OR<const SGPUBuffer>					mIndexBuffer;
		OR<const TArray<const CGPUTexture> >	mTextures;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPURenderState

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CGPURenderState::CGPURenderState(EGPURenderMode renderMode, CGPUVertexShader& vertexShader,
		CGPUFragmentShader& fragmentShader)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals =
			new CGPURenderStateInternals(renderMode, (COpenGLVertexShader&) vertexShader,
					(COpenGLFragmentShader&) fragmentShader);

	// Configure GL
	mInternals->mVertexShader.configureGL();
}

//----------------------------------------------------------------------------------------------------------------------
CGPURenderState::~CGPURenderState()
//----------------------------------------------------------------------------------------------------------------------
{
	// Reset
	if (glIsEnabled(GL_BLEND))
		// Disable
		glDisable(GL_BLEND);

	// Reset GL
	mInternals->mVertexShader.resetGL();

	// Cleanup
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderState::setModelMatrix(const SMatrix4x4_32& modelMatrix)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mModelMatrix = modelMatrix;
}

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderState::setVertexBuffer(const SGPUVertexBuffer& gpuVertexBuffer)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mVertexBuffer = OR<const SGPUVertexBuffer>(gpuVertexBuffer);
}

//----------------------------------------------------------------------------------------------------------------------
const OR<const SGPUBuffer>& CGPURenderState::getIndexBuffer() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mIndexBuffer;
}

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderState::setIndexBuffer(const SGPUBuffer& gpuIndexBuffer)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mIndexBuffer = OR<const SGPUBuffer>(gpuIndexBuffer);
}

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderState::setTextures(const TArray<const CGPUTexture>& gpuTextures)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mTextures = OR<const TArray<const CGPUTexture> >(gpuTextures);
}

//----------------------------------------------------------------------------------------------------------------------
EGPURenderMode CGPURenderState::getRenderMode() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mRenderMode;
}

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderState::commit(const SGPURenderStateCommitInfo& renderStateCommitInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	TDictionary<COpenGLProgram>	sPrograms;

	// Setup buffers
	((COpenGLVertexBufferInfo*) mInternals->mVertexBuffer->mPlatformReference)->makeCurrent();
	if (mInternals->mIndexBuffer.hasReference())
		// Setup index buffer
		((COpenGLIndexBufferInfo*) mInternals->mIndexBuffer->mPlatformReference)->makeCurrent();

	// Check for textures
	if (mInternals->mTextures.hasReference()) {
		// Setup textures
				bool						needBlend = false;
		const	TArray<const CGPUTexture>&	gpuTextures = mInternals->mTextures.getReference();
		for (CArray::ItemIndex i = 0; i < gpuTextures.getCount(); i++) {
			// Setup
			const	COpenGLTexture&	openGLTexture = (const COpenGLTexture&) gpuTextures[i];

			// Setup this texture
			glActiveTexture(GL_TEXTURE0 + i);
			glBindTexture(GL_TEXTURE_2D, openGLTexture.getTextureName());
			needBlend |= openGLTexture.hasTransparency();
		}

		// Setup blend
		if (needBlend) {
			// Need to blend
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		}
	}

	// Setup program
	CString	programKey =
					mInternals->mVertexShader.getUUID().getBase64String() + CString(OSSTR("/")) +
							mInternals->mFragmentShader.getUUID().getBase64String();

	// Ensure we have this program
	if (!sPrograms[programKey].hasReference())
		// Create and cache
		sPrograms.set(programKey, COpenGLProgram(mInternals->mVertexShader, mInternals->mFragmentShader));

	// Prepare program
	sPrograms[programKey]->prepare(renderStateCommitInfo.mProjectionMatrix, renderStateCommitInfo.mViewMatrix,
			mInternals->mModelMatrix);
}
