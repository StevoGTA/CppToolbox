//----------------------------------------------------------------------------------------------------------------------
//	CMetalShaderBuiltIns.mm			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#import "CMetalShader.h"

#import "MetalShaderTypes.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data


//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMetalVertexShaderBasic

class CMetalVertexShaderBasic : public CMetalVertexShader {
	public:
				CMetalVertexShaderBasic() :
					CMetalVertexShader()
					{}

		CString	getName() const
					{ return CString(OSSTR("basicVertexShader")); }

		void	setup(id<MTLRenderCommandEncoder> renderCommandEncoder, id<MTLDevice> device) const
					{
						// Setup instance uniforms
						id<MTLBuffer>	basicVertexUniformsBuffer =
												[device newBufferWithLength:sizeof(BasicVertexUniforms)
														options:MTLResourceStorageModeShared];
						basicVertexUniformsBuffer.label = @"Basic Vertex Uniforms";

						BasicVertexUniforms*	basicVertexUniforms =
														(BasicVertexUniforms*) basicVertexUniformsBuffer.contents;
						basicVertexUniforms->mModelMatrix = *((matrix_float4x4*) &getModelMatrix());

						[renderCommandEncoder setVertexBuffer:basicVertexUniformsBuffer offset:0
								atIndex:kBufferIndexVertexUniforms];
					}

//		void	setAttibutes(const CDictionary& attributeInfo, const SGPUVertexBuffer& gpuVertexBuffer)
//					{
//						// Setup attributes
//						GLint	positionAttributeLocation = attributeInfo.getSInt32(mPositionAttributeName);
//						glEnableVertexAttribArray(positionAttributeLocation);
//						glVertexAttribPointer(positionAttributeLocation,
//								(GLint) gpuVertexBuffer.mGPUVertexBufferInfo.mVertexCount, GL_FLOAT, GL_FALSE,
//								(GLsizei) gpuVertexBuffer.mGPUVertexBufferInfo.mTotalSize,
//								(GLvoid*) (intptr_t) gpuVertexBuffer.mGPUVertexBufferInfo.mVertexOffset);
//
//						GLint	textureCoordinateAttributeLocation =
//										attributeInfo.getSInt32(mTextureCoordinateAttributeName);
//						glEnableVertexAttribArray(textureCoordinateAttributeLocation);
//						glVertexAttribPointer(textureCoordinateAttributeLocation,
//								(GLint) gpuVertexBuffer.mGPUVertexBufferInfo.mTextureCoordinateCount, GL_FLOAT,
//								GL_FALSE, (GLsizei) gpuVertexBuffer.mGPUVertexBufferInfo.mTotalSize,
//								(GLvoid*) (intptr_t) gpuVertexBuffer.mGPUVertexBufferInfo.mTextureCoordinateOffset);
//					}
//
//		void	setUniforms(const CDictionary& uniformInfo, const SMatrix4x4_32& projectionMatrix,
//						const SMatrix4x4_32& viewMatrix, const SMatrix4x4_32& modelMatrix)
//					{
//						// Setup uniforms
//						SMatrix4x4_32	modelViewProjectionMatrix = projectionMatrix * viewMatrix * modelMatrix;
//						GLint			modelViewProjectionMatrixUniformLocation =
//												uniformInfo.getSInt32(mModelViewProjectionMatrixUniformName);
//						glUniformMatrix4fv(modelViewProjectionMatrixUniformLocation, 1, 0,
//								(GLfloat*) &modelViewProjectionMatrix);
//					}

//		static	CString	mPositionAttributeName;
//		static	CString	mTextureCoordinateAttributeName;
//
//		static	CString	mModelViewProjectionMatrixUniformName;
};

//CString	CMetalVertexShaderBasic::mPositionAttributeName(OSSTR("position"));
//CString	CMetalVertexShaderBasic::mTextureCoordinateAttributeName(OSSTR("texCoord0"));
//
//CString	CMetalVertexShaderBasic::mModelViewProjectionMatrixUniformName(OSSTR("modelViewProjectionMatrix"));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMetalVertexShaderClip

class CMetalVertexShaderClip : public CMetalVertexShader {
	public:
				CMetalVertexShaderClip() :
					CMetalVertexShader()
					{}

		CString	getName() const
					{ return CString(OSSTR("clipVertexShader")); }

		void	setup(id<MTLRenderCommandEncoder> renderCommandEncoder, id<MTLDevice> device) const
					{
//						// Setup instance uniforms
//						id<MTLBuffer>	basicVertexUniformsBuffer =
//												[device newBufferWithLength:sizeof(BasicVertexUniforms)
//														options:MTLResourceStorageModeShared];
//						basicVertexUniformsBuffer.label = @"Basic Vertex Uniforms";
//
//						BasicVertexUniforms*	basicVertexUniforms =
//														(BasicVertexUniforms*) basicVertexUniformsBuffer.contents;
//						basicVertexUniforms->mModelMatrix = *((matrix_float4x4*) &getModelMatrix());
//
//						[renderCommandEncoder setVertexBuffer:basicVertexUniformsBuffer offset:0
//								atIndex:kBufferIndexVertexUniforms];
					}
//		void	setAttibutes(const CDictionary& attributeInfo, const SGPUVertexBuffer& gpuVertexBuffer)
//					{
//						// Setup attributes
//						GLint	positionAttributeLocation = attributeInfo.getSInt32(mPositionAttributeName);
//						glEnableVertexAttribArray(positionAttributeLocation);
//						glVertexAttribPointer(positionAttributeLocation,
//								(GLint) gpuVertexBuffer.mGPUVertexBufferInfo.mVertexCount, GL_FLOAT, GL_FALSE,
//								(GLsizei) gpuVertexBuffer.mGPUVertexBufferInfo.mTotalSize,
//								(GLvoid*) (intptr_t) gpuVertexBuffer.mGPUVertexBufferInfo.mVertexOffset);
//
//						GLint	textureCoordinateAttributeLocation =
//										attributeInfo.getSInt32(mTextureCoordinateAttributeName);
//						glEnableVertexAttribArray(textureCoordinateAttributeLocation);
//						glVertexAttribPointer(textureCoordinateAttributeLocation,
//								(GLint) gpuVertexBuffer.mGPUVertexBufferInfo.mTextureCoordinateCount, GL_FLOAT,
//								GL_FALSE, (GLsizei) gpuVertexBuffer.mGPUVertexBufferInfo.mTotalSize,
//								(GLvoid*) (intptr_t) gpuVertexBuffer.mGPUVertexBufferInfo.mTextureCoordinateOffset);
//					}
//
//		void	setUniforms(const CDictionary& uniformInfo, const SMatrix4x4_32& projectionMatrix,
//						const SMatrix4x4_32& viewMatrix, const SMatrix4x4_32& modelMatrix)
//					{
//						// Setup uniforms
//						SMatrix4x4_32	viewProjectionMatrix = projectionMatrix * viewMatrix;
//						GLint			viewProjectionMatrixUniformLocation =
//												uniformInfo.getSInt32(mViewProjectionMatrixUniformName);
//						GLint			modelMatrixUniformLocation = uniformInfo.getSInt32(mModelMatrixUniformName);
//						GLint			clipPlaneUniformLocation = uniformInfo.getSInt32(mClipPlaneUniformName);
//
//						glUniformMatrix4fv(viewProjectionMatrixUniformLocation, 1, 0, (GLfloat*) &viewProjectionMatrix);
//						glUniformMatrix4fv(modelMatrixUniformLocation, 1, 0, (GLfloat*) &modelMatrix);
//						glUniform4fv(clipPlaneUniformLocation, 1, (GLfloat*) &mClipPlane);
//					}

//		void	configureGL()
//					{
//						// Setup GL
//#if TARGET_OS_IOS
////						glEnable(GL_CLIP_DISTANCE0_APPLE);
//#endif
//
//#if TARGET_OS_MACOS
////						glEnable(GL_CLIP_DISTANCE0);
//#endif
//					}
//		void	resetGL()
//					{
//#if TARGET_OS_IOS
////						glDisable(GL_CLIP_DISTANCE0_APPLE);
//#endif
//
//#if TARGET_OS_MACOS
////						glDisable(GL_CLIP_DISTANCE0);
//#endif
//					}

		void	setClipPlane(const SMatrix4x1_32& clipPlane)
					{
						mClipPlane = clipPlane;
					}

				SMatrix4x1_32	mClipPlane;

//		static	CString			mPositionAttributeName;
//		static	CString			mTextureCoordinateAttributeName;

//		static	CString			mViewProjectionMatrixUniformName;
//		static	CString			mModelMatrixUniformName;
//		static	CString			mClipPlaneUniformName;
};

//CString	CMetalVertexShaderClip::mPositionAttributeName(OSSTR("position"));
//CString	CMetalVertexShaderClip::mTextureCoordinateAttributeName(OSSTR("texCoord0"));

//CString	CMetalVertexShaderClip::mViewProjectionMatrixUniformName(OSSTR("viewProjectionMatrix"));
//CString	CMetalVertexShaderClip::mModelMatrixUniformName(OSSTR("modelMatrix"));
//CString	CMetalVertexShaderClip::mClipPlaneUniformName(OSSTR("clipPlane"));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUVertexShader

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CGPUVertexShader& CGPUVertexShader::getBasic()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	CMetalVertexShaderBasic*	sVertexShader = nil;

	// Check if have shader
	if (sVertexShader == nil) {
//		// Create shader
//		TNArray<CString>	attributeNames;
//		attributeNames += CMetalVertexShaderBasic::mPositionAttributeName;
//		attributeNames += CMetalVertexShaderBasic::mTextureCoordinateAttributeName;
//
//		TNArray<CString>	uniformNames;
//		uniformNames += CMetalVertexShaderBasic::mModelViewProjectionMatrixUniformName;

		sVertexShader = new CMetalVertexShaderBasic();
	}

	return *sVertexShader;
}

//----------------------------------------------------------------------------------------------------------------------
CGPUVertexShader& CGPUVertexShader::getClip(const SMatrix4x1_32& clipPlane)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	CMetalVertexShaderClip*	sVertexShader = nil;

	// Check if have shader
	if (sVertexShader == nil) {
		// Create shader
//		TNArray<CString>	attributeNames;
//		attributeNames += CMetalVertexShaderBasic::mPositionAttributeName;
//		attributeNames += CMetalVertexShaderBasic::mTextureCoordinateAttributeName;
//
//		TNArray<CString>	uniformNames;
//		uniformNames += CMetalVertexShaderClip::mViewProjectionMatrixUniformName;
//		uniformNames += CMetalVertexShaderClip::mModelMatrixUniformName;
//		uniformNames += CMetalVertexShaderClip::mClipPlaneUniformName;

		sVertexShader = new CMetalVertexShaderClip();
	}

	// Setup
	sVertexShader->setClipPlane(clipPlane);

	return *sVertexShader;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMetalFragmentShaderBasic

class CMetalFragmentShaderBasic : public CMetalFragmentShader {
	public:
				CMetalFragmentShaderBasic() :
					CMetalFragmentShader()	//,
//							mDidSetupDiffuseTextureUniforms(false)
					{}

		CString	getName() const
					{ return CString(OSSTR("basicFragmentShader")); }

//		void	setUniforms(const CDictionary& uniformInfo)
//					{
//						// Setup uniforms
//						if (!mDidSetupDiffuseTextureUniforms) {
//							// Setup diffuse texture uniforms
//							for (UInt32 i = 0; i < 16; i++) {
//								// Setup
//								CString	uniform = CString(OSSTR("diffuseTexture[")) + CString(i) + CString(OSSTR("]"));
//								glUniform1i(uniformInfo.getSInt32(uniform), i);
//							}
//
//							mDidSetupDiffuseTextureUniforms = true;
//						}
//					}

//		bool	mDidSetupDiffuseTextureUniforms;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMetalFragmentShaderOpacity

class CMetalFragmentShaderOpacity : public CMetalFragmentShader {
	public:
				CMetalFragmentShaderOpacity() :
					CMetalFragmentShader(),
							mOpacity(1.0)//, mDidSetupDiffuseTextureUniforms(false)
					{}

		CString	getName() const
					{ return CString(OSSTR("opacityFragmentShader")); }

//		void	setUniforms(const CDictionary& uniformInfo)
//					{
//						// Setup uniforms
//						if (!mDidSetupDiffuseTextureUniforms) {
//							// Setup diffuse texture uniforms
//							for (UInt32 i = 0; i < 16; i++) {
//								// Setup
//								CString	uniform = CString(OSSTR("diffuseTexture[")) + CString(i) + CString(OSSTR("]"));
//								glUniform1i(uniformInfo.getSInt32(uniform), i);
//							}
//
//							mDidSetupDiffuseTextureUniforms = true;
//						}
//
//						GLint	opacityUniformLocation = uniformInfo.getSInt32(mOpacityUniformName);
//						glUniform1f(opacityUniformLocation, mOpacity);
//					}

		void	setOpacity(Float32 opacity)
					{
						mOpacity = opacity;
					}

				Float32	mOpacity;

//				bool	mDidSetupDiffuseTextureUniforms;

//		static	CString	mOpacityUniformName;
};

//CString	CMetalFragmentShaderOpacity::mOpacityUniformName(OSSTR("opacity"));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUFragmentShader

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CGPUFragmentShader& CGPUFragmentShader::getBasic()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	CMetalFragmentShaderBasic*	sFragmentShader = nil;

	// Check if have shader
	if (sFragmentShader == nil) {
		// Create shader
//		TNArray<CString>	uniformNames;
//		for (UInt32 i = 0; i < 16; i++)
//			// Setup
//			uniformNames += CString(OSSTR("diffuseTexture[")) + CString(i) + CString(OSSTR("]"));

		sFragmentShader = new CMetalFragmentShaderBasic();
	}

	return *sFragmentShader;
}

//----------------------------------------------------------------------------------------------------------------------
CGPUFragmentShader& CGPUFragmentShader::getOpacity(Float32 opacity)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	CMetalFragmentShaderOpacity*	sFragmentShader = nil;

	// Check if have shader
	if (sFragmentShader == nil) {
		// Create shader
//		TNArray<CString>	uniformNames;
//		for (UInt32 i = 0; i < 16; i++)
//			// Setup
//			uniformNames += CString(OSSTR("diffuseTexture[")) + CString(i) + CString(OSSTR("]"));
//		uniformNames += CMetalFragmentShaderOpacity::mOpacityUniformName;

		sFragmentShader = new CMetalFragmentShaderOpacity();
	}

	// Setup
	sFragmentShader->setOpacity(opacity);

	return *sFragmentShader;
}
