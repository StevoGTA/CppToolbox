//----------------------------------------------------------------------------------------------------------------------
//	COpenGLProgram.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CGPUProgram.h"
#include "CString.h"

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
// MARK: COpenGLShaderInternals

class COpenGLShaderInternals;

//----------------------------------------------------------------------------------------------------------------------
// MARK: - COpenGLVertexShader

class COpenGLVertexShader : public CGPUVertexShader {
	// Methods
	public:
				// Lifecycle methods
				COpenGLVertexShader(const CString& string);
				~COpenGLVertexShader();

				// Instance methods
		GLuint	getShader() const;

	// Properties
	private:
		COpenGLShaderInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - COpenGLFragmentShader

class COpenGLFragmentShader : public CGPUFragmentShader {
	// Methods
	public:
				// Lifecycle methods
				COpenGLFragmentShader(const CString& string);
				~COpenGLFragmentShader();

				// Instance methods
		GLuint	getShader() const;

	// Properties
	private:
		COpenGLShaderInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPUProgramInternals

class CGPUProgramInternals {
	// Methods
	public:
		// Lifecycle methods
		CGPUProgramInternals(const COpenGLVertexShader& vertexShader, const COpenGLFragmentShader& fragmentShader);
		~CGPUProgramInternals();

	// Properties
	public:
		GLuint			mProgram;

		SMatrix4x4_32	mViewMatrix;
		SMatrix4x4_32	mProjectionMatrix;
};
