//----------------------------------------------------------------------------------------------------------------------
//	COpenGLBuiltIns.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "COpenGLBuiltIns.h"

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
static	CString	sOpacityFragmentShaderString("																			\
												uniform         sampler2D   diffuseTexture;								\
												uniform lowp    float       opacity;									\
																														\
												varying highp   vec2        v_texPosition0;								\
																														\
												void main() {															\
													gl_FragColor = texture2D(diffuseTexture, v_texPosition0) * opacity;	\
												}																		\
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
static	CString	sOpacityFragmentShaderString("																				\
												#version 330 core															\
												uniform	sampler2D   diffuseTexture;											\
												uniform	float       opacity;												\
																															\
												in		vec2		v_texPosition0;											\
																															\
												out		vec4		fragColor;												\
																															\
												void main() {																\
													fragColor = texture(diffuseTexture, v_texPosition0.st, 0.0) * opacity;	\
												}																			\
											 ");
#endif

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - COpenGLBuiltIns

//----------------------------------------------------------------------------------------------------------------------
const COpenGLVertexShader& COpenGLBuiltIns::getBasicVertexShader()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	OO<COpenGLVertexShader>	sShader;

	// Check if have shader
	if (!sShader.hasObject())
		// Create shader
		sShader = OO<COpenGLVertexShader>(new COpenGLVertexShader(sBasicVertexShaderString));

	return *sShader;
}

//----------------------------------------------------------------------------------------------------------------------
const COpenGLFragmentShader& COpenGLBuiltIns::getOpacityFragmentShader()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	OO<COpenGLFragmentShader>	sShader;

	// Check if have shader
	if (!sShader.hasObject())
		// Create shader
		sShader = OO<COpenGLFragmentShader>(new COpenGLFragmentShader(sOpacityFragmentShaderString));

	return *sShader;
}

//----------------------------------------------------------------------------------------------------------------------
const COpenGLProgram& COpenGLBuiltIns::getOpacityProgram()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	OO<COpenGLProgram>	sProgram;

	// Check if have program
	if (!sProgram.hasObject())
		// Create program
		sProgram = OO<COpenGLProgram>(new COpenGLProgram(getBasicVertexShader(), getOpacityFragmentShader()));

	return *sProgram;
}
