//----------------------------------------------------------------------------------------------------------------------
//	COpenGLProgram.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "COpenGLProgram.h"

#include "CLogServices.h"
#include "COpenGLTextureInfo.h"

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
// MARK: - COpenGLVertexShader

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
COpenGLVertexShader::COpenGLVertexShader(const CString& string) : CGPUVertexShader()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new COpenGLShaderInternals(GL_VERTEX_SHADER, string);
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
//----------------------------------------------------------------------------------------------------------------------
// MARK: - COpenGLFragmentShader

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
COpenGLFragmentShader::COpenGLFragmentShader(const CString& string) : CGPUFragmentShader()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new COpenGLShaderInternals(GL_FRAGMENT_SHADER, string);
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
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUProgramInternals

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CGPUProgramInternals::CGPUProgramInternals(const COpenGLVertexShader& vertexShader,
		const COpenGLFragmentShader& fragmentShader) : mProgram(glCreateProgram())
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
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

//----------------------------------------------------------------------------------------------------------------------
CGPUProgramInternals::~CGPUProgramInternals()
//----------------------------------------------------------------------------------------------------------------------
{
	// Cleanup
	glDeleteProgram(mProgram);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUProgram

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CGPUProgram::CGPUProgram(const CGPUVertexShader& vertexShader, const CGPUFragmentShader& fragmentShader)
//----------------------------------------------------------------------------------------------------------------------
{
	mGPUProgramInternals =
			new CGPUProgramInternals((const COpenGLVertexShader&) vertexShader,
					(const COpenGLFragmentShader&) fragmentShader);
}

//----------------------------------------------------------------------------------------------------------------------
CGPUProgram::~CGPUProgram()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mGPUProgramInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CGPUProgram::setup(const SMatrix4x4_32& viewMatrix, const SMatrix4x4_32& projectionMatrix)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mGPUProgramInternals->mViewMatrix = viewMatrix;
	mGPUProgramInternals->mProjectionMatrix = projectionMatrix;
}

//----------------------------------------------------------------------------------------------------------------------
const SMatrix4x4_32& CGPUProgram::getViewMatrix() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mGPUProgramInternals->mViewMatrix;
}

//----------------------------------------------------------------------------------------------------------------------
const SMatrix4x4_32& CGPUProgram::getProjectionMatrix() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mGPUProgramInternals->mProjectionMatrix;
}

//----------------------------------------------------------------------------------------------------------------------
void CGPUProgram::willUse() const
//----------------------------------------------------------------------------------------------------------------------
{
	glUseProgram(mGPUProgramInternals->mProgram);
}

//----------------------------------------------------------------------------------------------------------------------
void CGPUProgram::didFinish() const
//----------------------------------------------------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUTextureProgramInternals

class CGPUTextureProgramInternals {
	public:
		CGPUTextureProgramInternals(const CGPUProgramInternals& gpuProgramInternals) :
			mPositionAttributeLocation(glGetAttribLocation(gpuProgramInternals.mProgram, "position")),
			mTextureCoordinateAttributeLocation(glGetAttribLocation(gpuProgramInternals.mProgram, "texCoord0")),
			mTextureSampler0UniformLocation(glGetUniformLocation(gpuProgramInternals.mProgram, "diffuseTexture[0]")),
			mTextureSampler1UniformLocation(glGetUniformLocation(gpuProgramInternals.mProgram, "diffuseTexture[1]")),
			mTextureSampler2UniformLocation(glGetUniformLocation(gpuProgramInternals.mProgram, "diffuseTexture[2]")),
			mTextureSampler3UniformLocation(glGetUniformLocation(gpuProgramInternals.mProgram, "diffuseTexture[3]")),
			mTextureSampler4UniformLocation(glGetUniformLocation(gpuProgramInternals.mProgram, "diffuseTexture[4)")),
			mTextureSampler5UniformLocation(glGetUniformLocation(gpuProgramInternals.mProgram, "diffuseTexture[5]")),
			mTextureSampler6UniformLocation(glGetUniformLocation(gpuProgramInternals.mProgram, "diffuseTexture[6]")),
			mTextureSampler7UniformLocation(glGetUniformLocation(gpuProgramInternals.mProgram, "diffuseTexture[7]")),
			mTextureSampler8UniformLocation(glGetUniformLocation(gpuProgramInternals.mProgram, "diffuseTexture[8]")),
			mTextureSampler9UniformLocation(glGetUniformLocation(gpuProgramInternals.mProgram, "diffuseTexture[9]")),
			mTextureSampler10UniformLocation(glGetUniformLocation(gpuProgramInternals.mProgram, "diffuseTexture[10]")),
			mTextureSampler11UniformLocation(glGetUniformLocation(gpuProgramInternals.mProgram, "diffuseTexture[11]")),
			mTextureSampler12UniformLocation(glGetUniformLocation(gpuProgramInternals.mProgram, "diffuseTexture[12]")),
			mTextureSampler13UniformLocation(glGetUniformLocation(gpuProgramInternals.mProgram, "diffuseTexture[13]")),
			mTextureSampler14UniformLocation(glGetUniformLocation(gpuProgramInternals.mProgram, "diffuseTexture[14]")),
			mTextureSampler15UniformLocation(glGetUniformLocation(gpuProgramInternals.mProgram, "diffuseTexture[15]"))
			{
				// Finish setup
				glGenVertexArrays(1, &mVertexArray);
				glGenBuffers(1, &mModelDataBuffer);
			}
		~CGPUTextureProgramInternals()
			{
				// Cleanup
				glDeleteBuffers(1, &mModelDataBuffer);
				glDeleteVertexArrays(1, &mVertexArray);
			}

		GLint	mPositionAttributeLocation;
		GLint	mTextureCoordinateAttributeLocation;
		GLint	mTextureSampler0UniformLocation;
		GLint	mTextureSampler1UniformLocation;
		GLint	mTextureSampler2UniformLocation;
		GLint	mTextureSampler3UniformLocation;
		GLint	mTextureSampler4UniformLocation;
		GLint	mTextureSampler5UniformLocation;
		GLint	mTextureSampler6UniformLocation;
		GLint	mTextureSampler7UniformLocation;
		GLint	mTextureSampler8UniformLocation;
		GLint	mTextureSampler9UniformLocation;
		GLint	mTextureSampler10UniformLocation;
		GLint	mTextureSampler11UniformLocation;
		GLint	mTextureSampler12UniformLocation;
		GLint	mTextureSampler13UniformLocation;
		GLint	mTextureSampler14UniformLocation;
		GLint	mTextureSampler15UniformLocation;

		GLuint	mVertexArray;
		GLuint	mModelDataBuffer;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUTextureProgram

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CGPUTextureProgram::CGPUTextureProgram(const CGPUVertexShader& vertexShader, const CGPUFragmentShader& fragmentShader) :
		CGPUProgram(vertexShader, fragmentShader)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CGPUTextureProgramInternals(*mGPUProgramInternals);
    glUniform1i(mInternals->mTextureSampler0UniformLocation, 0);
    glUniform1i(mInternals->mTextureSampler1UniformLocation, 1);
    glUniform1i(mInternals->mTextureSampler2UniformLocation, 2);
    glUniform1i(mInternals->mTextureSampler3UniformLocation, 3);
    glUniform1i(mInternals->mTextureSampler4UniformLocation, 4);
    glUniform1i(mInternals->mTextureSampler5UniformLocation, 5);
    glUniform1i(mInternals->mTextureSampler6UniformLocation, 6);
    glUniform1i(mInternals->mTextureSampler7UniformLocation, 7);
    glUniform1i(mInternals->mTextureSampler8UniformLocation, 8);
    glUniform1i(mInternals->mTextureSampler9UniformLocation, 9);
    glUniform1i(mInternals->mTextureSampler10UniformLocation, 10);
    glUniform1i(mInternals->mTextureSampler11UniformLocation, 11);
    glUniform1i(mInternals->mTextureSampler12UniformLocation, 12);
    glUniform1i(mInternals->mTextureSampler13UniformLocation, 13);
    glUniform1i(mInternals->mTextureSampler14UniformLocation, 14);
    glUniform1i(mInternals->mTextureSampler15UniformLocation, 15);
}

//----------------------------------------------------------------------------------------------------------------------
CGPUTextureProgram::~CGPUTextureProgram()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CGPUProgram methods

//----------------------------------------------------------------------------------------------------------------------
void CGPUTextureProgram::willUse() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Do super
	CGPUProgram::willUse();
}

//----------------------------------------------------------------------------------------------------------------------
void CGPUTextureProgram::didFinish() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Do super
	CGPUProgram::didFinish();

	// Reset
	if (glIsEnabled(GL_BLEND))
		// Disable
		glDisable(GL_BLEND);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CGPUTextureProgram::setupVertexTextureInfo(const SGPUVertexBuffer& gpuVertexBuffer, UInt32 triangleCount,
		const TArray<const SGPUTextureInfo>& gpuTextureInfos)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup buffers
	glBindVertexArray(mInternals->mVertexArray);

	glBindBuffer(GL_ARRAY_BUFFER, mInternals->mModelDataBuffer);
	glBufferData(GL_ARRAY_BUFFER, gpuVertexBuffer.mData.getSize(), gpuVertexBuffer.mData.getBytePtr(), GL_STATIC_DRAW);

	// Setup textures
	bool	needBlend = false;
	for (CArrayItemIndex i = 0; i < gpuTextureInfos.getCount(); i++) {
		// Setup
		COpenGLTextureInfo*	openGLTextureInfo = (COpenGLTextureInfo*) gpuTextureInfos[i].mInternalReference;

		// Setup this texture
		glActiveTexture(GL_TEXTURE0 + i);
		glBindTexture(GL_TEXTURE_2D, openGLTextureInfo->getTextureName());
		needBlend |= openGLTextureInfo->hasTransparency();
	}

	// Setup program
	glEnableVertexAttribArray(mInternals->mPositionAttributeLocation);
	glVertexAttribPointer(mInternals->mPositionAttributeLocation,
			(GLint) gpuVertexBuffer.mGPUVertexBufferInfo.mVertexCount, GL_FLOAT, GL_FALSE,
			(GLsizei) gpuVertexBuffer.mGPUVertexBufferInfo.mTotalSize,
			(GLvoid*) (intptr_t) gpuVertexBuffer.mGPUVertexBufferInfo.mVertexOffset);

	glEnableVertexAttribArray(mInternals->mTextureCoordinateAttributeLocation);
	glVertexAttribPointer(mInternals->mTextureCoordinateAttributeLocation,
			(GLint) gpuVertexBuffer.mGPUVertexBufferInfo.mTextureCoordinateCount, GL_FLOAT, GL_FALSE,
			(GLsizei) gpuVertexBuffer.mGPUVertexBufferInfo.mTotalSize,
			(GLvoid*) (intptr_t) gpuVertexBuffer.mGPUVertexBufferInfo.mTextureCoordinateOffset);


    // Setup blend
	if (needBlend) {
		// Need to blend
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
}
