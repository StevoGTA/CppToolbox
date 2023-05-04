//----------------------------------------------------------------------------------------------------------------------
//	COpenGLShader.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "COpenGLShader.h"

#include "CLogServices.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: COpenGLShader::Internals

class COpenGLShader::Internals {
	public:
		Internals(GLenum type, const CString& string)
			{
				// Create shader
				mShader = glCreateShader(type);
				if (mShader > 0) {
					// Compile shader
					const	CString::C	cString = string.getCString();
					const	char*		string = *cString;
					glShaderSource(mShader, 1, &string, nil);
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
		~Internals()
			{
				// Cleanup
				glDeleteShader(mShader);
			}

		GLuint	mShader;
		CUUID	mUUID;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - COpenGLVertexShaderInternals

class COpenGLVertexShaderInternals : public COpenGLShader::Internals {
	public:
		COpenGLVertexShaderInternals(const CString& string, const TArray<CString>& attributeNames,
				const TArray<CString>& uniformNames) :
			COpenGLShader::Internals(GL_VERTEX_SHADER, string),
					mAttributeNames(attributeNames), mUniformNames(uniformNames)
			{}

		TArray<CString>	mAttributeNames;
		TArray<CString>	mUniformNames;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - COpenGLVertexShader

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
COpenGLVertexShader::COpenGLVertexShader(const CString& string, const TArray<CString>& attributeNames,
		const TArray<CString>& uniformNames) : CGPUVertexShader()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new COpenGLVertexShaderInternals(string, attributeNames, uniformNames);
}

//----------------------------------------------------------------------------------------------------------------------
COpenGLVertexShader::~COpenGLVertexShader()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
GLuint COpenGLVertexShader::getShader() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mShader;
}

//----------------------------------------------------------------------------------------------------------------------
const CUUID& COpenGLVertexShader::getUUID() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mUUID;
}

//----------------------------------------------------------------------------------------------------------------------
const TArray<CString>& COpenGLVertexShader::getAttributeNames() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mAttributeNames;
}

//----------------------------------------------------------------------------------------------------------------------
const TArray<CString>& COpenGLVertexShader::getUniformNames() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mUniformNames;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - COpenGLFragmentShaderInternals

class COpenGLFragmentShaderInternals : public COpenGLShader::Internals {
	public:
		COpenGLFragmentShaderInternals(const CString& string, const TArray<CString>& uniformNames) :
			COpenGLShader::Internals(GL_FRAGMENT_SHADER, string), mUniformNames(uniformNames)
			{}

		TArray<CString>	mUniformNames;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - COpenGLFragmentShader

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
COpenGLFragmentShader::COpenGLFragmentShader(const CString& string, const TArray<CString>& uniformNames) :
		CGPUFragmentShader()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new COpenGLFragmentShaderInternals(string, uniformNames);
}

//----------------------------------------------------------------------------------------------------------------------
COpenGLFragmentShader::~COpenGLFragmentShader()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
GLuint COpenGLFragmentShader::getShader() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mShader;
}

//----------------------------------------------------------------------------------------------------------------------
const CUUID& COpenGLFragmentShader::getUUID() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mUUID;
}

//----------------------------------------------------------------------------------------------------------------------
const TArray<CString>& COpenGLFragmentShader::getUniformNames() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mUniformNames;
}
