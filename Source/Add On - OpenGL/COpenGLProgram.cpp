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
	DisposeOf(mInternals);
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
	DisposeOf(mInternals);
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
	DisposeOf(mGPUProgramInternals);
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
			mTextureSamplerUniformLocation(glGetUniformLocation(gpuProgramInternals.mProgram, "diffuseTexture"))
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
		GLint	mTextureSamplerUniformLocation;

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
}

//----------------------------------------------------------------------------------------------------------------------
CGPUTextureProgram::~CGPUTextureProgram()
//----------------------------------------------------------------------------------------------------------------------
{
	DisposeOf(mInternals);
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
		const SGPUTextureInfo& gpuTextureInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	COpenGLTextureInfo*	openGLTextureInfo = (COpenGLTextureInfo*) gpuTextureInfo.mInternalReference;

	GLint				vertexCount;
	GLsizei				stride;
	GLvoid*				textureBufferOffset;
	switch (gpuVertexBuffer.mGPUVertexBufferType) {
		case kGPUVertexBufferType2Vertex2Texture:
			// 2 Vertex, 2 Texture
			vertexCount = 2;
			stride = 4 * sizeof(Float32);
			textureBufferOffset = (GLvoid*) (2 * sizeof(Float32));
			break;

		case kGPUVertexBufferType3Vertex2Texture:
			// 3 Vertex, 2 Texture
			vertexCount = 3;
			stride = 5 * sizeof(Float32);
			textureBufferOffset = (GLvoid*) (3 * sizeof(Float32));
			break;
	}

	// Setup buffers
	glBindVertexArray(mInternals->mVertexArray);

	glBindBuffer(GL_ARRAY_BUFFER, mInternals->mModelDataBuffer);
	glBufferData(GL_ARRAY_BUFFER, gpuVertexBuffer.mData.getSize(), gpuVertexBuffer.mData.getBytePtr(), GL_STATIC_DRAW);

	// Setup texture
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, openGLTextureInfo->getTextureName());

	// Setup program
	glEnableVertexAttribArray(mInternals->mPositionAttributeLocation);
	glVertexAttribPointer(mInternals->mPositionAttributeLocation, vertexCount, GL_FLOAT, GL_FALSE, stride, 0);

	glEnableVertexAttribArray(mInternals->mTextureCoordinateAttributeLocation);
	glVertexAttribPointer(mInternals->mTextureCoordinateAttributeLocation, 2, GL_FLOAT, GL_FALSE, stride,
			textureBufferOffset);

    glUniform1i(mInternals->mTextureSamplerUniformLocation, 0);

    // Setup blend
	if (openGLTextureInfo->hasTransparency()) {
		// Need to blend
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}
}
