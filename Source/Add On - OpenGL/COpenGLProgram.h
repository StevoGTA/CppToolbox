//----------------------------------------------------------------------------------------------------------------------
//	COpenGLProgram.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CString.h"

#if TARGET_OS_IOS
	#include <OpenGLES/ES2/gl.h>
#endif

//#if TARGET_OS_LINUX
//	#include <GLES/gl.h>
//#endif

#if TARGET_OS_MACOS
	#include <OpenGL/gl.h>
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: COpenGLShader

class COpenGLShaderInternals;
class COpenGLShader {
	// Methods
	public:
				// Lifecycle methods
				~COpenGLShader();

				// Instance methods
		GLuint	getShader() const;

	protected:
				// Lifecycle methods
				COpenGLShader(GLenum type, const CString& string);

	// Properties
	private:
		COpenGLShaderInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - COpenGLVertexShader

class COpenGLVertexShader : public COpenGLShader {
	// Methods
	public:
		// Lifecycle methods
		COpenGLVertexShader(const CString& string) : COpenGLShader(GL_VERTEX_SHADER, string) {}
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - COpenGLFragmentShader

class COpenGLFragmentShader : public COpenGLShader {
	// Methods
	public:
		// Lifecycle methods
		COpenGLFragmentShader(const CString& string) : COpenGLShader(GL_FRAGMENT_SHADER, string) {}
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - COpenGLProgram

class COpenGLProgramInternals;
class COpenGLProgram {
	// Methods
	public:
				// Lifecycle methods
				COpenGLProgram(const COpenGLVertexShader& vertexShader,
						const COpenGLFragmentShader& fragmentShader);
				~COpenGLProgram();

				// Instance methods
		void	use() const;

		int		getAttributeLocation(const CString& attributeName) const;
		int		getUniformLocation(const CString& uniformName) const;

	// Properties
	private:
		COpenGLProgramInternals*	mInternals;
};
