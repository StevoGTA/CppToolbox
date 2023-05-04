//----------------------------------------------------------------------------------------------------------------------
//	COpenGLRenderState.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#if defined(TARGET_OS_IOS)
	#include <OpenGLES/ES3/glext.h>
#endif

//#if defined(TARGET_OS_LINUX)
//	#include <GLES/gl.h>
//#endif

#if defined(TARGET_OS_MACOS)
	#include <OpenGL/gl3.h>
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: COpenGLBufferInfo

class COpenGLBufferInfo {
	// Methods
	public:
		// Lifecycle methods
		virtual	~COpenGLBufferInfo() {}

	protected:
		// Lifecycle methods
		COpenGLBufferInfo() {}
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - COpenGLIndexBufferInfo

class COpenGLIndexBufferInfo : COpenGLBufferInfo {
	// Methods
	public:
				// Lifecycle methods
				COpenGLIndexBufferInfo(const CData& data) : COpenGLBufferInfo()
					{
						// Setup
						glGenBuffers(1, &mIndexDataBuffer);
						glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexDataBuffer);
						glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.getByteCount(), (UInt8*) data.getBytePtr(),
								GL_STATIC_DRAW);
					}
				~COpenGLIndexBufferInfo()
					{
						// Cleanup
						glDeleteBuffers(1, &mIndexDataBuffer);
					}

				// Instance methods
		void	makeCurrent() const
					{ glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexDataBuffer); }

	// Properties
	private:
		GLuint	mIndexDataBuffer;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - COpenGLVertexBufferInfo

class COpenGLVertexBufferInfo : COpenGLBufferInfo {
	// Methods
	public:
				// Lifecycle methods
				COpenGLVertexBufferInfo(const CData& data) : COpenGLBufferInfo()
					{
						// Setup
						glGenVertexArrays(1, &mVertexArray);
						glBindVertexArray(mVertexArray);

						glGenBuffers(1, &mVertexDataBuffer);
						glBindBuffer(GL_ARRAY_BUFFER, mVertexDataBuffer);
						glBufferData(GL_ARRAY_BUFFER, data.getByteCount(), (UInt8*) data.getBytePtr(), GL_STATIC_DRAW);
					}
				~COpenGLVertexBufferInfo()
					{
						// Cleanup
						glDeleteBuffers(1, &mVertexDataBuffer);
						glDeleteVertexArrays(1, &mVertexArray);
					}

				// Instance methods
		void	makeCurrent() const
					{
						// Make current
						glBindVertexArray(mVertexArray);
						glBindBuffer(GL_ARRAY_BUFFER, mVertexDataBuffer);
					}

	// Properties
	private:
		GLuint	mVertexArray;
		GLuint	mVertexDataBuffer;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPU::CommitInfo

struct CGPU::CommitInfo {
	// Lifecycle methods
	CommitInfo(const SMatrix4x4_32& viewMatrix, const SMatrix4x4_32& projectionMatrix) :
		mViewMatrix(viewMatrix), mProjectionMatrix(projectionMatrix)
		{}

	// Properties
	const	SMatrix4x4_32&	mViewMatrix;
	const	SMatrix4x4_32&	mProjectionMatrix;
};
