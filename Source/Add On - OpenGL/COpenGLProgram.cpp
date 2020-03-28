//----------------------------------------------------------------------------------------------------------------------
//	COpenGLProgram.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "COpenGLProgram.h"

#include "CLogServices.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: COpenGLShaderInternals

class COpenGLShaderInternals {
	public:
		COpenGLShaderInternals(GLenum type, const CString& string)
			{
				// Create shader
				mShader = glCreateShader(type);
				if (mShader > 0) {
					// Compile shader
					const	SCString	cString = string.getCString();
					glShaderSource(mShader, 1, &cString.mBuffer, nil);
					glCompileShader(mShader);

					// Get status
					GLint	status;
					glGetShaderiv(mShader, GL_COMPILE_STATUS, &status);
					if (status == GL_FALSE) {
						// Log error
						GLint	logLength;
						glGetShaderiv(mShader, GL_INFO_LOG_LENGTH, &logLength);
						if (logLength > 0) {
							char	log[logLength];
							glGetShaderInfoLog(mShader, logLength, &logLength, log);
							CLogServices::logError(CString("Shader failed to compile with log:\n") + CString(log));
						}

						// Cleanup
						glDeleteShader(mShader);
						mShader = 0;
					}
				} else
					// Error
					CLogServices::logError(CString("Could not create shader"));
			}
		~COpenGLShaderInternals()
			{
				// Cleanup
				glDeleteShader(mShader);
			}

		GLuint	mShader;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - COpenGLShader

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
COpenGLShader::COpenGLShader(GLenum type, const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new COpenGLShaderInternals(type, string);
}

//----------------------------------------------------------------------------------------------------------------------
COpenGLShader::~COpenGLShader()
//----------------------------------------------------------------------------------------------------------------------
{
	DisposeOf(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
GLuint COpenGLShader::getShader() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mShader;
}

//----------------------------------------------------------------------------------------------------------------------
// MARK: COpenGLProgramInternals

class COpenGLProgramInternals {
	public:
		COpenGLProgramInternals(const COpenGLVertexShader& vertexShader, const COpenGLFragmentShader& fragmentShader)
			{
				// Create program
				mProgram = glCreateProgram();
				if (mProgram > 0) {
					// Attach shaders
					glAttachShader(mProgram, vertexShader.getShader());
					glAttachShader(mProgram, fragmentShader.getShader());

					// Link
					glLinkProgram(mProgram);

					// Cleanup
					glDetachShader(mProgram, vertexShader.getShader());
					glDetachShader(mProgram, fragmentShader.getShader());

					// Get status
					GLint	status;
					glGetProgramiv(mProgram, GL_LINK_STATUS, &status);
					if (status == GL_FALSE) {
						// Log error
						GLint	logLength;
						glGetShaderiv(mProgram, GL_INFO_LOG_LENGTH, &logLength);
						if (logLength > 0) {
							char	log[logLength];
							glGetProgramInfoLog(mProgram, logLength, &logLength, log);
							CLogServices::logError(CString("Program failed to link with log:\n") + CString(log));
						}

						// Cleanup
						glDeleteProgram(mProgram);
						mProgram = 0;
					}
				} else
					// Error
					CLogServices::logError(CString("Could not create program"));
			}
		~COpenGLProgramInternals()
			{
				// Cleanup
				glDeleteProgram(mProgram);
			}

		GLuint	mProgram;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - COpenGLProgram

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
COpenGLProgram::COpenGLProgram(const COpenGLVertexShader& vertexShader, const COpenGLFragmentShader& fragmentShader)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new COpenGLProgramInternals(vertexShader, fragmentShader);
}

//----------------------------------------------------------------------------------------------------------------------
COpenGLProgram::~COpenGLProgram()
//----------------------------------------------------------------------------------------------------------------------
{
	DisposeOf(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void COpenGLProgram::use() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Use
	glUseProgram(mInternals->mProgram);
}

//----------------------------------------------------------------------------------------------------------------------
int COpenGLProgram::getAttributeLocation(const CString& attributeName) const
//----------------------------------------------------------------------------------------------------------------------
{
	return glGetAttribLocation(mInternals->mProgram, *attributeName.getCString());
}

//----------------------------------------------------------------------------------------------------------------------
int COpenGLProgram::getUniformLocation(const CString& uniformName) const
//----------------------------------------------------------------------------------------------------------------------
{
	return glGetUniformLocation(mInternals->mProgram, *uniformName.getCString());
}
