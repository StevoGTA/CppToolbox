//----------------------------------------------------------------------------------------------------------------------
//	COpenGLRenderState.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#if TARGET_OS_IOS
	#include <OpenGLES/ES3/glext.h>
#endif

//#if TARGET_OS_LINUX
//	#include <GLES/gl.h>
//#endif

#if TARGET_OS_MACOS
	#include <OpenGL/gl3.h>
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: SOpenGLVertexBufferInfo

struct SOpenGLVertexBufferInfo {
	// Lifecycle methods
	SOpenGLVertexBufferInfo(const CData& data)
		{
			// Finish setup
			glGenVertexArrays(1, &mVertexArray);
			glBindVertexArray(mVertexArray);

			glGenBuffers(1, &mVertexDataBuffer);
			glBindBuffer(GL_ARRAY_BUFFER, mVertexDataBuffer);
			glBufferData(GL_ARRAY_BUFFER, data.getSize(), (UInt8*) data.getBytePtr(), GL_STATIC_DRAW);
		}
	~SOpenGLVertexBufferInfo()
		{
			// Cleanup
			glDeleteBuffers(1, &mVertexDataBuffer);
			glDeleteVertexArrays(1, &mVertexArray);
		}

	// Properties
	GLuint	mVertexArray;
	GLuint	mVertexDataBuffer;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SGPURenderStateCommitInfo

struct SGPURenderStateCommitInfo {
	// Lifecycle methods
	SGPURenderStateCommitInfo(const SMatrix4x4_32& projectionMatrix, const SMatrix4x4_32& viewMatrix) :
		mProjectionMatrix(projectionMatrix), mViewMatrix(viewMatrix)
		{}

	// Properties
	SMatrix4x4_32	mProjectionMatrix;
	SMatrix4x4_32	mViewMatrix;
};
