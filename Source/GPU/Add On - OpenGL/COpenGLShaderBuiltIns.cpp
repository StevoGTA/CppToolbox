//----------------------------------------------------------------------------------------------------------------------
//	COpenGLShaderBuiltIns.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "COpenGLShader.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

#if TARGET_OS_IOS
static	CString	sBasicVertexShaderString("#version 300 es			\
			uniform			mat4    modelViewProjectionMatrix;		\
																	\
			in				vec4    position;						\
			in				vec3    texCoord0;						\
																	\
			out		highp	vec3	v_texPosition0;					\
																	\
			void main() {											\
				gl_Position = modelViewProjectionMatrix * position;	\
				v_texPosition0 = texCoord0;							\
			}														\
		");
static	CString	sClipVertexShaderString("#version 300 es									\
			#extension GL_APPLE_clip_distance : require\n									\
			uniform			mat4    projectionMatrix;										\
			uniform			mat4    viewMatrix;												\
			uniform			mat4    modelMatrix;											\
			uniform			vec4	clipPlane;												\
																							\
			in				vec4    position;												\
			in				vec3    texCoord0;												\
																							\
			out		highp	float	gl_ClipDistance[1];										\
			out		highp	vec3	v_texPosition0;											\
																							\
			void main() {																	\
				gl_Position = projectionMatrix * viewMatrix * modelMatrix * position;		\
				gl_ClipDistance[0] = dot(viewMatrix * modelMatrix * position, clipPlane);	\
				v_texPosition0 = texCoord0;													\
			}																				\
		");
static	CString	sBasicFragmentShaderString("#version 300 es										\
			uniform			sampler2D   diffuseTexture[16];										\
																								\
			in		highp	vec3		v_texPosition0;											\
																								\
			out		highp	vec4		fragColor;												\
																								\
			void main() {																		\
				highp	vec2	size;															\
				highp	vec2	position;														\
				highp	vec4	color;															\
				switch (int(v_texPosition0.p)) {												\
					case 0:																		\
						size = vec2(textureSize(diffuseTexture[0], 0));							\
						position = vec2(v_texPosition0.s / size.x, v_texPosition0.t / size.y);	\
						color = texture(diffuseTexture[0], position, 0.0);						\
						break;																	\
																								\
					case 1:																		\
						size = vec2(textureSize(diffuseTexture[1], 0));							\
						position = vec2(v_texPosition0.s / size.x, v_texPosition0.t / size.y);	\
						color = texture(diffuseTexture[1], position, 0.0);						\
						break;																	\
																								\
					case 2:																		\
						size = vec2(textureSize(diffuseTexture[2], 0));							\
						position = vec2(v_texPosition0.s / size.x, v_texPosition0.t / size.y);	\
						color = texture(diffuseTexture[2], position, 0.0);						\
						break;																	\
																								\
					case 3:																		\
						size = vec2(textureSize(diffuseTexture[3], 0));							\
						position = vec2(v_texPosition0.s / size.x, v_texPosition0.t / size.y);	\
						color = texture(diffuseTexture[3], position, 0.0);						\
						break;																	\
																								\
					case 4:																		\
						size = vec2(textureSize(diffuseTexture[4], 0));							\
						position = vec2(v_texPosition0.s / size.x, v_texPosition0.t / size.y);	\
						color = texture(diffuseTexture[4], position, 0.0);						\
						break;																	\
																								\
					case 5:																		\
						size = vec2(textureSize(diffuseTexture[5], 0));							\
						position = vec2(v_texPosition0.s / size.x, v_texPosition0.t / size.y);	\
						color = texture(diffuseTexture[5], position, 0.0);						\
						break;																	\
																								\
					case 6:																		\
						size = vec2(textureSize(diffuseTexture[6], 0));							\
						position = vec2(v_texPosition0.s / size.x, v_texPosition0.t / size.y);	\
						color = texture(diffuseTexture[6], position, 0.0);						\
						break;																	\
																								\
					case 7:																		\
						size = vec2(textureSize(diffuseTexture[7], 0));							\
						position = vec2(v_texPosition0.s / size.x, v_texPosition0.t / size.y);	\
						color = texture(diffuseTexture[7], position, 0.0);						\
						break;																	\
																								\
					case 8:																		\
						size = vec2(textureSize(diffuseTexture[8], 0));							\
						position = vec2(v_texPosition0.s / size.x, v_texPosition0.t / size.y);	\
						color = texture(diffuseTexture[8], position, 0.0);						\
						break;																	\
																								\
					case 9:																		\
						size = vec2(textureSize(diffuseTexture[9], 0));							\
						position = vec2(v_texPosition0.s / size.x, v_texPosition0.t / size.y);	\
						color = texture(diffuseTexture[9], position, 0.0);						\
						break;																	\
																								\
					case 10:																	\
						size = vec2(textureSize(diffuseTexture[10], 0));						\
						position = vec2(v_texPosition0.s / size.x, v_texPosition0.t / size.y);	\
						color = texture(diffuseTexture[10], position, 0.0);						\
						break;																	\
																								\
					case 11:																	\
						size = vec2(textureSize(diffuseTexture[11], 0));						\
						position = vec2(v_texPosition0.s / size.x, v_texPosition0.t / size.y);	\
						color = texture(diffuseTexture[11], position, 0.0);						\
						break;																	\
																								\
					case 12:																	\
						size = vec2(textureSize(diffuseTexture[12], 0));						\
						position = vec2(v_texPosition0.s / size.x, v_texPosition0.t / size.y);	\
						color = texture(diffuseTexture[12], position, 0.0);						\
						break;																	\
																								\
					case 13:																	\
						size = vec2(textureSize(diffuseTexture[13], 0));						\
						position = vec2(v_texPosition0.s / size.x, v_texPosition0.t / size.y);	\
						color = texture(diffuseTexture[13], position, 0.0);						\
						break;																	\
																								\
					case 14:																	\
						size = vec2(textureSize(diffuseTexture[14], 0));						\
						position = vec2(v_texPosition0.s / size.x, v_texPosition0.t / size.y);	\
						color = texture(diffuseTexture[14], position, 0.0);						\
						break;																	\
																								\
					case 15:																	\
						size = vec2(textureSize(diffuseTexture[15], 0));						\
						position = vec2(v_texPosition0.s / size.x, v_texPosition0.t / size.y);	\
						color = texture(diffuseTexture[15], position, 0.0);						\
						break;																	\
				}																				\
																								\
				fragColor = color;																\
			}																					\
		");
static	CString	sOpacityFragmentShaderString("#version 300 es									\
			uniform			sampler2D   diffuseTexture[16];										\
			uniform lowp	float       opacity;												\
																								\
			in		highp	vec3		v_texPosition0;											\
																								\
			out		highp	vec4		fragColor;												\
																								\
			void main() {																		\
				highp	vec2	size;															\
				highp	vec2	position;														\
				highp	vec4	color;															\
				switch (int(v_texPosition0.p)) {												\
					case 0:																		\
						size = vec2(textureSize(diffuseTexture[0], 0));							\
						position = vec2(v_texPosition0.s / size.x, v_texPosition0.t / size.y);	\
						color = texture(diffuseTexture[0], position, 0.0);						\
						break;																	\
																								\
					case 1:																		\
						size = vec2(textureSize(diffuseTexture[1], 0));							\
						position = vec2(v_texPosition0.s / size.x, v_texPosition0.t / size.y);	\
						color = texture(diffuseTexture[1], position, 0.0);						\
						break;																	\
																								\
					case 2:																		\
						size = vec2(textureSize(diffuseTexture[2], 0));							\
						position = vec2(v_texPosition0.s / size.x, v_texPosition0.t / size.y);	\
						color = texture(diffuseTexture[2], position, 0.0);						\
						break;																	\
																								\
					case 3:																		\
						size = vec2(textureSize(diffuseTexture[3], 0));							\
						position = vec2(v_texPosition0.s / size.x, v_texPosition0.t / size.y);	\
						color = texture(diffuseTexture[3], position, 0.0);						\
						break;																	\
																								\
					case 4:																		\
						size = vec2(textureSize(diffuseTexture[4], 0));							\
						position = vec2(v_texPosition0.s / size.x, v_texPosition0.t / size.y);	\
						color = texture(diffuseTexture[4], position, 0.0);						\
						break;																	\
																								\
					case 5:																		\
						size = vec2(textureSize(diffuseTexture[5], 0));							\
						position = vec2(v_texPosition0.s / size.x, v_texPosition0.t / size.y);	\
						color = texture(diffuseTexture[5], position, 0.0);						\
						break;																	\
																								\
					case 6:																		\
						size = vec2(textureSize(diffuseTexture[6], 0));							\
						position = vec2(v_texPosition0.s / size.x, v_texPosition0.t / size.y);	\
						color = texture(diffuseTexture[6], position, 0.0);						\
						break;																	\
																								\
					case 7:																		\
						size = vec2(textureSize(diffuseTexture[7], 0));							\
						position = vec2(v_texPosition0.s / size.x, v_texPosition0.t / size.y);	\
						color = texture(diffuseTexture[7], position, 0.0);						\
						break;																	\
																								\
					case 8:																		\
						size = vec2(textureSize(diffuseTexture[8], 0));							\
						position = vec2(v_texPosition0.s / size.x, v_texPosition0.t / size.y);	\
						color = texture(diffuseTexture[8], position, 0.0);						\
						break;																	\
																								\
					case 9:																		\
						size = vec2(textureSize(diffuseTexture[9], 0));							\
						position = vec2(v_texPosition0.s / size.x, v_texPosition0.t / size.y);	\
						color = texture(diffuseTexture[9], position, 0.0);						\
						break;																	\
																								\
					case 10:																	\
						size = vec2(textureSize(diffuseTexture[10], 0));						\
						position = vec2(v_texPosition0.s / size.x, v_texPosition0.t / size.y);	\
						color = texture(diffuseTexture[10], position, 0.0);						\
						break;																	\
																								\
					case 11:																	\
						size = vec2(textureSize(diffuseTexture[11], 0));						\
						position = vec2(v_texPosition0.s / size.x, v_texPosition0.t / size.y);	\
						color = texture(diffuseTexture[11], position, 0.0);						\
						break;																	\
																								\
					case 12:																	\
						size = vec2(textureSize(diffuseTexture[12], 0));						\
						position = vec2(v_texPosition0.s / size.x, v_texPosition0.t / size.y);	\
						color = texture(diffuseTexture[12], position, 0.0);						\
						break;																	\
																								\
					case 13:																	\
						size = vec2(textureSize(diffuseTexture[13], 0));						\
						position = vec2(v_texPosition0.s / size.x, v_texPosition0.t / size.y);	\
						color = texture(diffuseTexture[13], position, 0.0);						\
						break;																	\
																								\
					case 14:																	\
						size = vec2(textureSize(diffuseTexture[14], 0));						\
						position = vec2(v_texPosition0.s / size.x, v_texPosition0.t / size.y);	\
						color = texture(diffuseTexture[14], position, 0.0);						\
						break;																	\
																								\
					case 15:																	\
						size = vec2(textureSize(diffuseTexture[15], 0));						\
						position = vec2(v_texPosition0.s / size.x, v_texPosition0.t / size.y);	\
						color = texture(diffuseTexture[15], position, 0.0);						\
						break;																	\
				}																				\
																								\
				color.a *= opacity;																\
																								\
				fragColor = color;																\
			}																					\
		");
#elif TARGET_OS_MACOS
static	CString	sBasicVertexShaderString("#version 330 core			\
			uniform	mat4    modelViewProjectionMatrix;				\
																	\
			in		vec4    position;								\
			in		vec3    texCoord0;								\
																	\
			out		vec3    v_texPosition0;							\
																	\
			void main() {											\
				gl_Position = modelViewProjectionMatrix * position;	\
				v_texPosition0 = texCoord0;							\
			}														\
		");
static	CString	sClipVertexShaderString("#version 330 core									\
			uniform	mat4	projectionMatrix;												\
			uniform	mat4	viewMatrix;														\
			uniform	mat4	modelMatrix;													\
			uniform	vec4	clipPlane;														\
																							\
			in		vec4    position;														\
			in		vec3    texCoord0;														\
																							\
			out		float	gl_ClipDistance[1];												\
			out		vec3    v_texPosition0;													\
																							\
			void main() {																	\
				gl_Position = projectionMatrix * viewMatrix * modelMatrix * position;		\
				gl_ClipDistance[0] = dot(viewMatrix * modelMatrix * position, clipPlane);	\
				v_texPosition0 = texCoord0;													\
			}																				\
		");
static	CString	sBasicFragmentShaderString("#version 330 core									\
			uniform	sampler2D   diffuseTexture[16];												\
																								\
			in		vec3		v_texPosition0;													\
																								\
			out		vec4		fragColor;														\
																								\
			void main() {																		\
				int		samplerIndex = int(v_texPosition0.p);									\
				ivec2	size = textureSize(diffuseTexture[samplerIndex], 0);					\
				vec2	position = vec2(v_texPosition0.s / size.x, v_texPosition0.t / size.y);	\
				vec4	color = texture(diffuseTexture[samplerIndex], position, 0.0);			\
																								\
				fragColor = color;																\
			}																					\
		");
static	CString	sOpacityFragmentShaderString("#version 330 core									\
			uniform	sampler2D   diffuseTexture[16];												\
			uniform	float       opacity;														\
																								\
			in		vec3		v_texPosition0;													\
																								\
			out		vec4		fragColor;														\
																								\
			void main() {																		\
				int		samplerIndex = int(v_texPosition0.p);									\
				ivec2	size = textureSize(diffuseTexture[samplerIndex], 0);					\
				vec2	position = vec2(v_texPosition0.s / size.x, v_texPosition0.t / size.y);	\
				vec4	color = texture(diffuseTexture[samplerIndex], position, 0.0);			\
																								\
				color.a *= opacity;																\
																								\
				fragColor = color;																\
			}																					\
		");
#endif

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - COpenGLVertexShaderBasic

class COpenGLVertexShaderBasic : public COpenGLVertexShader {
	public:
				COpenGLVertexShaderBasic(const TArray<CString>& attributeNames, const TArray<CString>& uniformNames) :
					COpenGLVertexShader(sBasicVertexShaderString, attributeNames, uniformNames)
					{}

		void	setAttibutes(const CDictionary& attributeInfo, const SGPUVertexBuffer& gpuVertexBuffer)
					{
						// Setup attributes
						GLint	positionAttributeLocation = attributeInfo.getSInt32(mPositionAttributeName);
						glEnableVertexAttribArray(positionAttributeLocation);
						glVertexAttribPointer(positionAttributeLocation,
								(GLint) gpuVertexBuffer.mGPUVertexBufferInfo.mVertexCount, GL_FLOAT, GL_FALSE,
								(GLsizei) gpuVertexBuffer.mGPUVertexBufferInfo.mTotalSize,
								(GLvoid*) (intptr_t) gpuVertexBuffer.mGPUVertexBufferInfo.mVertexOffset);

						GLint	textureCoordinateAttributeLocation =
										attributeInfo.getSInt32(mTextureCoordinateAttributeName);
						glEnableVertexAttribArray(textureCoordinateAttributeLocation);
						glVertexAttribPointer(textureCoordinateAttributeLocation,
								(GLint) gpuVertexBuffer.mGPUVertexBufferInfo.mTextureCoordinateCount, GL_FLOAT,
								GL_FALSE, (GLsizei) gpuVertexBuffer.mGPUVertexBufferInfo.mTotalSize,
								(GLvoid*) (intptr_t) gpuVertexBuffer.mGPUVertexBufferInfo.mTextureCoordinateOffset);
					}

		void	setUniforms(const CDictionary& uniformInfo, const SMatrix4x4_32& projectionMatrix,
						const SMatrix4x4_32& viewMatrix, const SMatrix4x4_32& modelMatrix)
					{
						// Setup uniforms
						SMatrix4x4_32	modelViewProjectionMatrix = projectionMatrix * viewMatrix * modelMatrix;
						GLint			modelViewProjectionMatrixUniformLocation =
												uniformInfo.getSInt32(mModelViewProjectionMatrixUniformName);
						glUniformMatrix4fv(modelViewProjectionMatrixUniformLocation, 1, 0,
								(GLfloat*) &modelViewProjectionMatrix);
					}

		static	CString	mPositionAttributeName;
		static	CString	mTextureCoordinateAttributeName;

		static	CString	mModelViewProjectionMatrixUniformName;
};

CString	COpenGLVertexShaderBasic::mPositionAttributeName(OSSTR("position"));
CString	COpenGLVertexShaderBasic::mTextureCoordinateAttributeName(OSSTR("texCoord0"));

CString	COpenGLVertexShaderBasic::mModelViewProjectionMatrixUniformName(OSSTR("modelViewProjectionMatrix"));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - COpenGLVertexShaderClip

class COpenGLVertexShaderClip : public COpenGLVertexShader {
	public:
				COpenGLVertexShaderClip(const TArray<CString>& attributeNames, const TArray<CString>& uniformNames) :
					COpenGLVertexShader(sClipVertexShaderString, attributeNames, uniformNames)
					{}

		void	setAttibutes(const CDictionary& attributeInfo, const SGPUVertexBuffer& gpuVertexBuffer)
					{
						// Setup attributes
						GLint	positionAttributeLocation = attributeInfo.getSInt32(mPositionAttributeName);
						glEnableVertexAttribArray(positionAttributeLocation);
						glVertexAttribPointer(positionAttributeLocation,
								(GLint) gpuVertexBuffer.mGPUVertexBufferInfo.mVertexCount, GL_FLOAT, GL_FALSE,
								(GLsizei) gpuVertexBuffer.mGPUVertexBufferInfo.mTotalSize,
								(GLvoid*) (intptr_t) gpuVertexBuffer.mGPUVertexBufferInfo.mVertexOffset);

						GLint	textureCoordinateAttributeLocation =
										attributeInfo.getSInt32(mTextureCoordinateAttributeName);
						glEnableVertexAttribArray(textureCoordinateAttributeLocation);
						glVertexAttribPointer(textureCoordinateAttributeLocation,
								(GLint) gpuVertexBuffer.mGPUVertexBufferInfo.mTextureCoordinateCount, GL_FLOAT,
								GL_FALSE, (GLsizei) gpuVertexBuffer.mGPUVertexBufferInfo.mTotalSize,
								(GLvoid*) (intptr_t) gpuVertexBuffer.mGPUVertexBufferInfo.mTextureCoordinateOffset);
					}

		void	setUniforms(const CDictionary& uniformInfo, const SMatrix4x4_32& projectionMatrix,
						const SMatrix4x4_32& viewMatrix, const SMatrix4x4_32& modelMatrix)
					{
						// Setup uniforms
						GLint	projectionMatrixUniformLocation = uniformInfo.getSInt32(mProjectionMatrixUniformName);
						GLint	viewMatrixUniformLocation = uniformInfo.getSInt32(mViewMatrixUniformName);
						GLint	modelMatrixUniformLocation = uniformInfo.getSInt32(mModelMatrixUniformName);
						GLint	clipPlaneUniformLocation = uniformInfo.getSInt32(mClipPlaneUniformName);

						glUniformMatrix4fv(projectionMatrixUniformLocation, 1, 0, (GLfloat*) &projectionMatrix);
						glUniformMatrix4fv(viewMatrixUniformLocation, 1, 0, (GLfloat*) &viewMatrix);
						glUniformMatrix4fv(modelMatrixUniformLocation, 1, 0, (GLfloat*) &modelMatrix);
						glUniform4fv(clipPlaneUniformLocation, 1, (GLfloat*) &mClipPlane);
					}

		void	configureGL()
					{
						// Setup GL
#if TARGET_OS_IOS
						glEnable(GL_CLIP_DISTANCE0_APPLE);
#endif

#if TARGET_OS_MACOS
						glEnable(GL_CLIP_DISTANCE0);
#endif
					}
		void	resetGL()
					{
#if TARGET_OS_IOS
						glDisable(GL_CLIP_DISTANCE0_APPLE);
#endif

#if TARGET_OS_MACOS
						glDisable(GL_CLIP_DISTANCE0);
#endif
					}

		void	setClipPlane(const SMatrix4x1_32& clipPlane)
					{ mClipPlane = clipPlane; }

				SMatrix4x1_32	mClipPlane;

		static	CString			mPositionAttributeName;
		static	CString			mTextureCoordinateAttributeName;

		static	CString			mProjectionMatrixUniformName;
		static	CString			mViewMatrixUniformName;
		static	CString			mModelMatrixUniformName;
		static	CString			mClipPlaneUniformName;
};

CString	COpenGLVertexShaderClip::mPositionAttributeName(OSSTR("position"));
CString	COpenGLVertexShaderClip::mTextureCoordinateAttributeName(OSSTR("texCoord0"));

CString	COpenGLVertexShaderClip::mProjectionMatrixUniformName(OSSTR("projectionMatrix"));
CString	COpenGLVertexShaderClip::mViewMatrixUniformName(OSSTR("viewMatrix"));
CString	COpenGLVertexShaderClip::mModelMatrixUniformName(OSSTR("modelMatrix"));
CString	COpenGLVertexShaderClip::mClipPlaneUniformName(OSSTR("clipPlane"));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUVertexShader

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CGPUVertexShader& CGPUVertexShader::getBasic()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	COpenGLVertexShaderBasic*	sVertexShader = nil;

	// Check if have shader
	if (sVertexShader == nil) {
		// Create shader
		TNArray<CString>	attributeNames;
		attributeNames += COpenGLVertexShaderBasic::mPositionAttributeName;
		attributeNames += COpenGLVertexShaderBasic::mTextureCoordinateAttributeName;

		TNArray<CString>	uniformNames;
		uniformNames += COpenGLVertexShaderBasic::mModelViewProjectionMatrixUniformName;

		sVertexShader = new COpenGLVertexShaderBasic(attributeNames, uniformNames);
	}

	return *sVertexShader;
}

//----------------------------------------------------------------------------------------------------------------------
CGPUVertexShader& CGPUVertexShader::getClip(const SMatrix4x1_32& clipPlane)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	COpenGLVertexShaderClip*	sVertexShader = nil;

	// Check if have shader
	if (sVertexShader == nil) {
		// Create shader
		TNArray<CString>	attributeNames;
		attributeNames += COpenGLVertexShaderBasic::mPositionAttributeName;
		attributeNames += COpenGLVertexShaderBasic::mTextureCoordinateAttributeName;

		TNArray<CString>	uniformNames;
		uniformNames += COpenGLVertexShaderClip::mProjectionMatrixUniformName;
		uniformNames += COpenGLVertexShaderClip::mViewMatrixUniformName;
		uniformNames += COpenGLVertexShaderClip::mModelMatrixUniformName;
		uniformNames += COpenGLVertexShaderClip::mClipPlaneUniformName;

		sVertexShader = new COpenGLVertexShaderClip(attributeNames, uniformNames);
	}

	// Setup
	sVertexShader->setClipPlane(clipPlane);

	return *sVertexShader;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - COpenGLFragmentShaderBasic

class COpenGLFragmentShaderBasic : public COpenGLFragmentShader {
	public:
				COpenGLFragmentShaderBasic(const TArray<CString>& uniformNames) :
					COpenGLFragmentShader(sBasicFragmentShaderString, uniformNames),
							mDidSetupDiffuseTextureUniforms(false)
					{}

		void	setUniforms(const CDictionary& uniformInfo)
					{
						// Setup uniforms
						if (!mDidSetupDiffuseTextureUniforms) {
							// Setup diffuse texture uniforms
							for (UInt32 i = 0; i < 16; i++) {
								// Setup
								CString	uniform = CString(OSSTR("diffuseTexture[")) + CString(i) + CString(OSSTR("]"));
								glUniform1i(uniformInfo.getSInt32(uniform), i);
							}

							mDidSetupDiffuseTextureUniforms = true;
						}
					}

		bool	mDidSetupDiffuseTextureUniforms;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - COpenGLFragmentShaderOpacity

class COpenGLFragmentShaderOpacity : public COpenGLFragmentShader {
	public:
				COpenGLFragmentShaderOpacity(const TArray<CString>& uniformNames) :
					COpenGLFragmentShader(sOpacityFragmentShaderString, uniformNames),
							mOpacity(1.0), mDidSetupDiffuseTextureUniforms(false)
					{}

		void	setUniforms(const CDictionary& uniformInfo)
					{
						// Setup uniforms
						if (!mDidSetupDiffuseTextureUniforms) {
							// Setup diffuse texture uniforms
							for (UInt32 i = 0; i < 16; i++) {
								// Setup
								CString	uniform = CString(OSSTR("diffuseTexture[")) + CString(i) + CString(OSSTR("]"));
								glUniform1i(uniformInfo.getSInt32(uniform), i);
							}

							mDidSetupDiffuseTextureUniforms = true;
						}

						GLint	opacityUniformLocation = uniformInfo.getSInt32(mOpacityUniformName);
						glUniform1f(opacityUniformLocation, mOpacity);
					}

		void	setOpacity(Float32 opacity)
					{ mOpacity = opacity; }

				Float32	mOpacity;

				bool	mDidSetupDiffuseTextureUniforms;

		static	CString	mOpacityUniformName;
};

CString	COpenGLFragmentShaderOpacity::mOpacityUniformName(OSSTR("opacity"));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUFragmentShader

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CGPUFragmentShader& CGPUFragmentShader::getBasic()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	COpenGLFragmentShaderBasic*	sFragmentShader = nil;

	// Check if have shader
	if (sFragmentShader == nil) {
		// Create shader
		TNArray<CString>	uniformNames;
		for (UInt32 i = 0; i < 16; i++)
			// Setup
			uniformNames += CString(OSSTR("diffuseTexture[")) + CString(i) + CString(OSSTR("]"));

		sFragmentShader = new COpenGLFragmentShaderBasic(uniformNames);
	}

	return *sFragmentShader;
}

//----------------------------------------------------------------------------------------------------------------------
CGPUFragmentShader& CGPUFragmentShader::getOpacity(Float32 opacity)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	COpenGLFragmentShaderOpacity*	sFragmentShader = nil;

	// Check if have shader
	if (sFragmentShader == nil) {
		// Create shader
		TNArray<CString>	uniformNames;
		for (UInt32 i = 0; i < 16; i++)
			// Setup
			uniformNames += CString(OSSTR("diffuseTexture[")) + CString(i) + CString(OSSTR("]"));
		uniformNames += COpenGLFragmentShaderOpacity::mOpacityUniformName;

		sFragmentShader = new COpenGLFragmentShaderOpacity(uniformNames);
	}

	// Setup
	sFragmentShader->setOpacity(opacity);

	return *sFragmentShader;
}
