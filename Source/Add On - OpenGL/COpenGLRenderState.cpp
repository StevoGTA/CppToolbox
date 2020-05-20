//----------------------------------------------------------------------------------------------------------------------
//	COpenGLRenderState.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CGPURenderState.h"

#include "CDictionary.h"
#include "COpenGLProgram.h"
#include "COpenGLTexture.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPURenderStateInternals

class CGPURenderStateInternals {
	public:
		CGPURenderStateInternals(CGPUVertexShader& vertexShader, CGPUFragmentShader& fragmentShader) :
			mVertexShader(vertexShader), mFragmentShader(fragmentShader), mTriangleOffset(0)
			{
				// Finish setup
				glGenVertexArrays(1, &mVertexArray);
				glGenBuffers(1, &mModelDataBuffer);
			}
		~CGPURenderStateInternals()
			{
				// Cleanup
				glDeleteBuffers(1, &mModelDataBuffer);
				glDeleteVertexArrays(1, &mVertexArray);
			}

		CGPUVertexShader&						mVertexShader;
		CGPUFragmentShader&						mFragmentShader;

		SMatrix4x4_32							mProjectionMatrix;
		SMatrix4x4_32							mViewMatrix;
		SMatrix4x4_32							mModelMatrix;

		OR<const SGPUVertexBuffer>				mVertexBuffer;
		UInt32									mTriangleOffset;
		OR<const TArray<const CGPUTexture> >	mGPUTextures;

		GLuint									mVertexArray;
		GLuint									mModelDataBuffer;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPURenderState

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CGPURenderState::CGPURenderState(CGPUVertexShader& vertexShader, CGPUFragmentShader& fragmentShader)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new CGPURenderStateInternals(vertexShader, fragmentShader);

	// Configure GL
	((COpenGLVertexShader&) mInternals->mVertexShader).configureGL();
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
	((COpenGLVertexShader&) mInternals->mVertexShader).resetGL();

	// Cleanup
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderState::setProjectionMatrix(const SMatrix4x4_32& projectionMatrix)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mProjectionMatrix = projectionMatrix;
}

//----------------------------------------------------------------------------------------------------------------------
const SMatrix4x4_32& CGPURenderState::getProjectionMatrix() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mProjectionMatrix;
}

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderState::setViewMatrix(const SMatrix4x4_32& viewMatrix)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mViewMatrix = viewMatrix;
}

//----------------------------------------------------------------------------------------------------------------------
const SMatrix4x4_32& CGPURenderState::getViewMatrix() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mViewMatrix;
}

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderState::setModelMatrix(const SMatrix4x4_32& modelMatrix)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mModelMatrix = modelMatrix;
}

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderState::setVertexTextureInfo(const SGPUVertexBuffer& gpuVertexBuffer, UInt32 triangleOffset,
		const TArray<const CGPUTexture>& gpuTextures)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mVertexBuffer = OR<const SGPUVertexBuffer>(gpuVertexBuffer);
	mInternals->mTriangleOffset = triangleOffset;
	mInternals->mGPUTextures = OR<const TArray<const CGPUTexture> >(gpuTextures);
}

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderState::commit()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static			TOwningDictionary<COpenGLProgram>	sPrograms;

			const	SGPUVertexBuffer&					gpuVertexBuffer = mInternals->mVertexBuffer.getReference();
					UInt32								offset =
																gpuVertexBuffer.mGPUVertexBufferInfo.mTotalSize *
																		mInternals->mTriangleOffset;

	// Setup buffers
	glBindVertexArray(mInternals->mVertexArray);

	glBindBuffer(GL_ARRAY_BUFFER, mInternals->mModelDataBuffer);
	glBufferData(GL_ARRAY_BUFFER, gpuVertexBuffer.mData.getSize() - offset,
			(UInt8*) gpuVertexBuffer.mData.getBytePtr() + offset, GL_STATIC_DRAW);

	// Setup textures
			bool						needBlend = false;
	const	TArray<const CGPUTexture>&	gpuTextures = mInternals->mGPUTextures.getReference();
	for (CArrayItemIndex i = 0; i < gpuTextures.getCount(); i++) {
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

	// Setup program
	CString	programKey =
					mInternals->mVertexShader.getUUID().getBase64String() + CString(OSSTR("/")) +
							mInternals->mFragmentShader.getUUID().getBase64String();

	// Ensure we have this program
	if (!sPrograms[programKey].hasReference())
		// Create and cache
		sPrograms.set(programKey,
				COpenGLProgram((COpenGLVertexShader&) mInternals->mVertexShader,
						(COpenGLFragmentShader&) mInternals->mFragmentShader));

	// Create internals
	sPrograms[programKey]->prepare(mInternals->mProjectionMatrix, mInternals->mViewMatrix, mInternals->mModelMatrix,
			gpuVertexBuffer);
}
