//----------------------------------------------------------------------------------------------------------------------
//	COpenGLProgram.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "COpenGLProgram.h"

#include "CReferenceCountable.h"
#include "CLogServices.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: COpenGLProgram::Internals

class COpenGLProgram::Internals : public TReferenceCountableAutoDelete<Internals> {
	public:
		Internals(COpenGLVertexShader& vertexShader, COpenGLFragmentShader& fragmentShader) :
			TReferenceCountableAutoDelete(),
					mVertexShader(vertexShader), mFragmentShader(fragmentShader), mProgram(glCreateProgram())
			{
				// Setup
				if (mProgram > 0) {
					// Attach shaders
					glAttachShader(mProgram, mVertexShader.getShader());
					glAttachShader(mProgram, mFragmentShader.getShader());

					// Link
					glLinkProgram(mProgram);

					// Cleanup
					glDetachShader(mProgram, mVertexShader.getShader());
					glDetachShader(mProgram, mFragmentShader.getShader());

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
							CLogServices::logError(
									CString(OSSTR("Program failed to link with log:\n")) +
											CString(log, logLength, CString::kEncodingASCII));
						}

						// Cleanup
						glDeleteProgram(mProgram);
						mProgram = 0;
					} else {
						// Setup
						const	TArray<CString>&	attributeNames = mVertexShader.getAttributeNames();
						for (TIteratorD<CString> iterator = attributeNames.getIterator(); iterator.hasValue();
								iterator.advance())
							// Store attribute location
							mAttributeInfo.set(*iterator, glGetAttribLocation(mProgram, *iterator->getUTF8String()));

						const	TArray<CString>&	vertexShaderUniformNames = mVertexShader.getUniformNames();
						for (TIteratorD<CString> iterator = vertexShaderUniformNames.getIterator(); iterator.hasValue();
								iterator.advance())
							// Store attribute location
							mUniformInfo.set(*iterator, glGetUniformLocation(mProgram, *iterator->getUTF8String()));

						const	TArray<CString>&	fragmentShaderuniformNames = mFragmentShader.getUniformNames();
						for (TIteratorD<CString> iterator = fragmentShaderuniformNames.getIterator();
								iterator.hasValue(); iterator.advance())
							// Store attribute location
							mUniformInfo.set(*iterator, glGetUniformLocation(mProgram, *iterator->getUTF8String()));
					}
				} else
					// Error
					CLogServices::logError(CString(OSSTR("Could not create program")));
			}
		~Internals()
			{
				// Cleanup
				glDeleteProgram(mProgram);
			}

		COpenGLVertexShader&	mVertexShader;
		COpenGLFragmentShader&	mFragmentShader;

		GLuint					mProgram;

		CDictionary				mAttributeInfo;
		CDictionary				mUniformInfo;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - COpenGLProgram

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
COpenGLProgram::COpenGLProgram(COpenGLVertexShader& vertexShader, COpenGLFragmentShader& fragmentShader)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(vertexShader, fragmentShader);
}

//----------------------------------------------------------------------------------------------------------------------
COpenGLProgram::COpenGLProgram(const COpenGLProgram& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
COpenGLProgram::~COpenGLProgram()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

//----------------------------------------------------------------------------------------------------------------------
void COpenGLProgram::prepare(const SMatrix4x4_32& projectionMatrix, const SMatrix4x4_32& viewMatrix,
		const SMatrix4x4_32& modelMatrix) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup program
	glUseProgram(mInternals->mProgram);

	// Setup shaders
	mInternals->mVertexShader.setAttibutes(mInternals->mAttributeInfo);
	mInternals->mVertexShader.setUniforms(mInternals->mUniformInfo, projectionMatrix, viewMatrix, modelMatrix);
	mInternals->mFragmentShader.setUniforms(mInternals->mUniformInfo);
}
