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
		CGPURenderStateInternals(CGPURenderState::Mode mode, COpenGLVertexShader& vertexShader,
				COpenGLFragmentShader& fragmentShader) :
			mMode(mode), mVertexShader(vertexShader), mFragmentShader(fragmentShader)
			{}

		CGPURenderState::Mode						mMode;
		COpenGLVertexShader&						mVertexShader;
		COpenGLFragmentShader&						mFragmentShader;

		SMatrix4x4_32								mModelMatrix;

		OR<const SGPUVertexBuffer>					mVertexBuffer;
		OR<const SGPUBuffer>						mIndexBuffer;
		OR<const TArray<const I<CGPUTexture> > >	mTextures;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPURenderState

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CGPURenderState::CGPURenderState(Mode mode, CGPUVertexShader& vertexShader, CGPUFragmentShader& fragmentShader)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals =
			new CGPURenderStateInternals(mode, (COpenGLVertexShader&) vertexShader,
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
void CGPURenderState::setTextures(const TArray<const I<CGPUTexture> >& gpuTextures)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mTextures = OR<const TArray<const I<CGPUTexture> > >(gpuTextures);
}

//----------------------------------------------------------------------------------------------------------------------
const OR<const TArray<const I<CGPUTexture> > > CGPURenderState::getTextures() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mTextures;
}

//----------------------------------------------------------------------------------------------------------------------
CGPURenderState::Mode CGPURenderState::getMode() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mMode;
}

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderState::commit(const SGPURenderStateCommitInfo& renderStateCommitInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	TNDictionary<COpenGLProgram>	sPrograms;

	// Setup buffers
	((COpenGLVertexBufferInfo*) mInternals->mVertexBuffer->mPlatformReference)->makeCurrent();
	if (mInternals->mIndexBuffer.hasReference())
		// Setup index buffer
		((COpenGLIndexBufferInfo*) mInternals->mIndexBuffer->mPlatformReference)->makeCurrent();

	// Check for textures
	if (mInternals->mTextures.hasReference()) {
		// Setup textures
				bool							needBlend = false;
		const	TArray<const I<CGPUTexture> >&	gpuTextures = mInternals->mTextures.getReference();
		for (CArray::ItemIndex i = 0; i < gpuTextures.getCount(); i++) {
			// Setup
			const	COpenGLTexture&	openGLTexture = (const COpenGLTexture&) *gpuTextures[i];

			// Setup this texture
			glActiveTexture(GL_TEXTURE0 + i);
			openGLTexture.bind();
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
