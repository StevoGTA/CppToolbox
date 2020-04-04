//----------------------------------------------------------------------------------------------------------------------
//	COpenGLBuiltIns.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CGPUProgramBuiltins.h"

#include "COpenGLProgram.h"
#include "CString.h"

#if TARGET_OS_IOS
	#include <OpenGLES/ES3/glext.h>
#endif

//#if TARGET_OS_LINUX
//	#include <GLES/gl.h>
//#endif

#if TARGET_OS_MACOS
	#include <OpenGL/gl3ext.h>
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

#if TARGET_OS_IOS
static	CString	sBasicVertexShaderString("																\
											uniform             mat4    modelViewProjectionMatrix;		\
																										\
											attribute           vec4    position;						\
											attribute           vec2    texCoord0;						\
																										\
											varying     highp   vec2    v_texPosition0;					\
																										\
											void main() {												\
												 gl_Position = modelViewProjectionMatrix * position;	\
												 v_texPosition0 = texCoord0;							\
											}															\
										");
static	CString	sClipVertexShaderString("																		\
											#extension GL_APPLE_clip_distance : require\n						\
											uniform             mat4    modelMatrix;							\
											uniform             mat4    viewProjectionMatrix;					\
											uniform				vec4	clipPlane;								\
																												\
											attribute           vec4    position;								\
											attribute           vec2    texCoord0;								\
																												\
											varying		highp	float	gl_ClipDistance[1];						\
											varying     highp   vec2    v_texPosition0;							\
																												\
											void main() {														\
												 gl_Position = modelMatrix * viewProjectionMatrix * position;	\
												 gl_ClipDistance[0] = dot(modelMatrix * position, clipPlane);	\
												 v_texPosition0 = texCoord0;									\
											}																	\
										");
static	CString	sOpaqueFragmentShaderString("																	\
												uniform         sampler2D   diffuseTexture;						\
																												\
												varying highp   vec2        v_texPosition0;						\
																												\
												void main() {													\
													gl_FragColor = texture2D(diffuseTexture, v_texPosition0);	\
												}																\
											 ");
static	CString	sOpacityFragmentShaderString("																	\
												uniform         sampler2D   diffuseTexture;						\
												uniform lowp    float       opacity;							\
																												\
												varying highp   vec2        v_texPosition0;						\
																												\
												void main() {													\
													highp	vec4	color = texture2D(diffuseTexture, v_texPosition0);	\
													color.a *= opacity;											\
													gl_FragColor = color;										\
												}																\
											 ");
static	CString	sClipOpacityFragmentShaderString("																	\
													uniform         sampler2D   diffuseTexture;						\
													uniform lowp    float       opacity;							\
																													\
													varying highp   vec2        v_texPosition0;						\
																													\
													void main() {													\
														highp	vec4	color = texture2D(diffuseTexture, v_texPosition0);	\
														color.a *= opacity;											\
														gl_FragColor = color;										\
													}																\
												 ");
#elif TARGET_OS_MACOS
static	CString	sBasicVertexShaderString("																\
											#version 330 core											\
											uniform	mat4    modelViewProjectionMatrix;					\
																										\
											in		vec4    position;									\
											in		vec2    texCoord0;									\
																										\
											out		vec2    v_texPosition0;								\
																										\
											void main() {												\
												 gl_Position = modelViewProjectionMatrix * position;	\
												 v_texPosition0 = texCoord0;							\
											}															\
										");
static	CString	sClipVertexShaderString("																		\
											#version 330 core													\
											uniform	mat4	modelMatrix;										\
											uniform	mat4	viewProjectionMatrix;								\
											uniform	vec4	clipPlane;											\
																												\
											in		vec4    position;											\
											in		vec2    texCoord0;											\
																												\
											out		float	gl_ClipDistance[1];									\
											out		vec2    v_texPosition0;										\
																												\
											void main() {														\
												 gl_Position = modelMatrix * viewProjectionMatrix * position;	\
												 gl_ClipDistance[0] = dot(modelMatrix * position, clipPlane);	\
												 v_texPosition0 = texCoord0;									\
											}																	\
										");
static	CString	sOpaqueFragmentShaderString("																		\
												#version 330 core													\
												uniform	sampler2D   diffuseTexture;									\
																													\
												in		vec2		v_texPosition0;									\
																													\
												out		vec4		fragColor;										\
																													\
												void main() {														\
													fragColor = texture(diffuseTexture, v_texPosition0.st, 0.0);	\
												}																	\
											");
static	CString	sOpacityFragmentShaderString("																	\
												#version 330 core												\
												uniform	sampler2D   diffuseTexture;								\
												uniform	float       opacity;									\
																												\
												in		vec2		v_texPosition0;								\
																												\
												out		vec4		fragColor;									\
																												\
												void main() {													\
													vec4	color = texture(diffuseTexture, v_texPosition0.st);	\
													color.a *= opacity;											\
													fragColor = color;											\
												}																\
											 ");
static	CString	sClipOpacityFragmentShaderString("																	\
													#version 330 core												\
													uniform	sampler2D   diffuseTexture;								\
													uniform	float       opacity;									\
																													\
													in		vec2		v_texPosition0;								\
																													\
													out		vec4		fragColor;									\
																													\
													void main() {													\
														vec4	color = texture(diffuseTexture, v_texPosition0.st);	\
														color.a *= opacity;											\
														fragColor = color;											\
													}																\
												 ");
#endif

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc declarations

static	const	COpenGLVertexShader&	sGetBasicVertexShader();
static	const	COpenGLVertexShader&	sGetClipVertexShader();

static	const	COpenGLFragmentShader&	sGetOpaqueFragmentShader();
static	const	COpenGLFragmentShader&	sGetOpacityFragmentShader();
static	const	COpenGLFragmentShader&	sGetClipOpacityFragmentShader();

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions

//----------------------------------------------------------------------------------------------------------------------
const COpenGLVertexShader& sGetBasicVertexShader()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	COpenGLVertexShader*	sShader = nil;

	// Check if have shader
	if (sShader == nil)
		// Create shader
		sShader = new COpenGLVertexShader(sBasicVertexShaderString);

	return *sShader;
}

//----------------------------------------------------------------------------------------------------------------------
const COpenGLVertexShader& sGetClipVertexShader()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	COpenGLVertexShader*	sShader = nil;

	// Check if have shader
	if (sShader == nil)
		// Create shader
		sShader = new COpenGLVertexShader(sClipVertexShaderString);

	return *sShader;
}

//----------------------------------------------------------------------------------------------------------------------
const COpenGLFragmentShader& sGetOpaqueFragmentShader()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	COpenGLFragmentShader*	sShader = nil;

	// Check if have shader
	if (sShader == nil)
		// Create shader
		sShader = new COpenGLFragmentShader(sOpaqueFragmentShaderString);

	return *sShader;
}

//----------------------------------------------------------------------------------------------------------------------
const COpenGLFragmentShader& sGetOpacityFragmentShader()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	COpenGLFragmentShader*	sShader = nil;

	// Check if have shader
	if (sShader == nil)
		// Create shader
		sShader = new COpenGLFragmentShader(sOpacityFragmentShaderString);

	return *sShader;
}

//----------------------------------------------------------------------------------------------------------------------
const COpenGLFragmentShader& sGetClipOpacityFragmentShader()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	COpenGLFragmentShader*	sShader = nil;

	// Check if have shader
	if (sShader == nil)
		// Create shader
		sShader = new COpenGLFragmentShader(sClipOpacityFragmentShaderString);

	return *sShader;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUOpaqueProgramInternals

class CGPUOpaqueProgramInternals {
	public:
		CGPUOpaqueProgramInternals(const CGPUProgramInternals& gpuProgramInternals) :
			mModelViewProjectionMatrixUniformLocation(
					glGetUniformLocation(gpuProgramInternals.mProgram, "modelViewProjectionMatrix"))
			{}

		GLint	mModelViewProjectionMatrixUniformLocation;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUOpaqueProgram

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CGPUOpaqueProgram::CGPUOpaqueProgram() : CGPUTextureProgram(sGetBasicVertexShader(), sGetOpaqueFragmentShader())
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CGPUOpaqueProgramInternals(*mGPUProgramInternals);
}

//----------------------------------------------------------------------------------------------------------------------
CGPUOpaqueProgram::~CGPUOpaqueProgram()
//----------------------------------------------------------------------------------------------------------------------
{
	DisposeOf(mInternals);
}

// MARK: CGPUProgram methods

//----------------------------------------------------------------------------------------------------------------------
void CGPUOpaqueProgram::setModelMatrix(const SMatrix4x4_32& modelViewMatrix)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SMatrix4x4_32	modelViewProjectionMatrix = modelViewMatrix * mGPUProgramInternals->mViewProjectionMatrix;

	// Set
    glUniformMatrix4fv(mInternals->mModelViewProjectionMatrixUniformLocation, 1, 0,
    		(GLfloat*) &modelViewProjectionMatrix);;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUOpacityProgramInternals

class CGPUOpacityProgramInternals {
	public:
		CGPUOpacityProgramInternals(const CGPUProgramInternals& gpuProgramInternals) :
			mModelViewProjectionMatrixUniformLocation(
					glGetUniformLocation(gpuProgramInternals.mProgram, "modelViewProjectionMatrix")),
			mOpacityUniformLocation(glGetUniformLocation(gpuProgramInternals.mProgram, "opacity"))
			{}

		GLint	mModelViewProjectionMatrixUniformLocation;
		GLint	mOpacityUniformLocation;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUOpacityProgram

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CGPUOpacityProgram::CGPUOpacityProgram() : CGPUTextureProgram(sGetBasicVertexShader(), sGetOpacityFragmentShader())
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CGPUOpacityProgramInternals(*mGPUProgramInternals);
}

//----------------------------------------------------------------------------------------------------------------------
CGPUOpacityProgram::~CGPUOpacityProgram()
//----------------------------------------------------------------------------------------------------------------------
{
	DisposeOf(mInternals);
}

// MARK: CGPUProgram methods

//----------------------------------------------------------------------------------------------------------------------
void CGPUOpacityProgram::setModelMatrix(const SMatrix4x4_32& modelViewMatrix)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SMatrix4x4_32	modelViewProjectionMatrix = modelViewMatrix * mGPUProgramInternals->mViewProjectionMatrix;

	// Set
    glUniformMatrix4fv(mInternals->mModelViewProjectionMatrixUniformLocation, 1, 0,
    		(GLfloat*) &modelViewProjectionMatrix);;
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CGPUOpacityProgram::setupVertexTextureInfo(const SGPUVertexBuffer& gpuVertexBuffer, UInt32 triangleCount,
		const SGPUTextureInfo& gpuTextureInfo, Float32 opacity)
//----------------------------------------------------------------------------------------------------------------------
{
	// Do super
	CGPUTextureProgram::setupVertexTextureInfo(gpuVertexBuffer, triangleCount, gpuTextureInfo);

    // Setup opacity
    glUniform1f(mInternals->mOpacityUniformLocation, opacity);

	// Need to blend
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUClipOpacityProgramInternals

class CGPUClipOpacityProgramInternals {
	public:
		CGPUClipOpacityProgramInternals(const CGPUProgramInternals& gpuProgramInternals) :
			mModelMatrixUniformLocation(glGetUniformLocation(gpuProgramInternals.mProgram, "modelMatrix")),
			mViewProjectionMatrixUniformLocation(
					glGetUniformLocation(gpuProgramInternals.mProgram, "viewProjectionMatrix")),
			mClipPlaneUniformLocation(glGetUniformLocation(gpuProgramInternals.mProgram, "clipPlane")),
			mOpacityUniformLocation(glGetUniformLocation(gpuProgramInternals.mProgram, "opacity"))
			{}

		GLint	mModelMatrixUniformLocation;
		GLint	mViewProjectionMatrixUniformLocation;
		GLint	mClipPlaneUniformLocation;
		GLint	mOpacityUniformLocation;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUClipOpacityProgram

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CGPUClipOpacityProgram::CGPUClipOpacityProgram() :
	CGPUTextureProgram(sGetClipVertexShader(), sGetClipOpacityFragmentShader())
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CGPUClipOpacityProgramInternals(*mGPUProgramInternals);
}

//----------------------------------------------------------------------------------------------------------------------
CGPUClipOpacityProgram::~CGPUClipOpacityProgram()
//----------------------------------------------------------------------------------------------------------------------
{
	DisposeOf(mInternals);
}

// MARK: CGPUProgram methods

//----------------------------------------------------------------------------------------------------------------------
void CGPUClipOpacityProgram::setModelMatrix(const SMatrix4x4_32& modelViewMatrix)
//----------------------------------------------------------------------------------------------------------------------
{
//	// Setup
//	SMatrix4x4_32	modelViewProjectionMatrix = modelViewMatrix * mGPUProgramInternals->mViewProjectionMatrix;
//
//	// Set
//    glUniformMatrix4fv(mInternals->mModelViewProjectionMatrixUniformLocation, 1, 0,
//    		(GLfloat*) &modelViewProjectionMatrix);
	// Set
	glUniformMatrix4fv(mInternals->mModelMatrixUniformLocation, 1, 0, (GLfloat*) &modelViewMatrix);
	glUniformMatrix4fv(mInternals->mViewProjectionMatrixUniformLocation, 1, 0,
			(GLfloat*) &mGPUProgramInternals->mViewProjectionMatrix);
}

//----------------------------------------------------------------------------------------------------------------------
void CGPUClipOpacityProgram::willUse() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Do super
	CGPUTextureProgram::willUse();

	// Setup GL
#if TARGET_OS_IOS
	glEnable(GL_CLIP_DISTANCE0_APPLE);
#endif

#if TARGET_OS_MACOS
	glEnable(GL_CLIP_DISTANCE0);
#endif
}

//----------------------------------------------------------------------------------------------------------------------
void CGPUClipOpacityProgram::didFinish() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Do super
	CGPUTextureProgram::didFinish();

	// Reset GL
#if TARGET_OS_IOS
	glDisable(GL_CLIP_DISTANCE0_APPLE);
#endif

#if TARGET_OS_MACOS
	glDisable(GL_CLIP_DISTANCE0);
#endif
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CGPUClipOpacityProgram::setupVertexTextureInfo(const SGPUVertexBuffer& gpuVertexBuffer, UInt32 triangleCount,
		const SGPUTextureInfo& gpuTextureInfo, Float32 opacity)
//----------------------------------------------------------------------------------------------------------------------
{
	// Do super
	CGPUTextureProgram::setupVertexTextureInfo(gpuVertexBuffer, triangleCount, gpuTextureInfo);

    // Setup opacity
    glUniform1f(mInternals->mOpacityUniformLocation, opacity);

	// Need to blend
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

//----------------------------------------------------------------------------------------------------------------------
void CGPUClipOpacityProgram::setClipPlane(const SMatrix4x1_32& clipPlane)
//----------------------------------------------------------------------------------------------------------------------
{
	glUniform4fv(mInternals->mClipPlaneUniformLocation, 1, (GLfloat*) &clipPlane);
}
