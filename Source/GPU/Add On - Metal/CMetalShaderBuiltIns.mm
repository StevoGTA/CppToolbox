//----------------------------------------------------------------------------------------------------------------------
//	CMetalShaderBuiltIns.mm			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#import "CMetalShader.h"

#import "MetalShaderTypes.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local proc declarations

static	CGPUFragmentShader&	sYCbCrFragmentShader(Float32 opacity);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMetalVertexShaderBasic

class CMetalVertexShaderBasic : public CMetalVertexShader {
	public:
								CMetalVertexShaderBasic() :
									CMetalVertexShader(CString(OSSTR("vertexShaderBasic")))
									{
										mVertexDescriptor = [[MTLVertexDescriptor alloc] init];

										mVertexDescriptor.attributes[kVertexAttributePosition].format =
												MTLVertexFormatFloat2;
										mVertexDescriptor.attributes[kVertexAttributePosition].offset = 0;
										mVertexDescriptor.attributes[kVertexAttributePosition].bufferIndex =
												kBufferIndexVertexPosition;
										mVertexDescriptor.layouts[kBufferIndexVertexPosition].stride = 20;
										mVertexDescriptor.layouts[kBufferIndexVertexPosition].stepRate = 1;
										mVertexDescriptor.layouts[kBufferIndexVertexPosition].stepFunction =
												MTLVertexStepFunctionPerVertex;

										mVertexDescriptor.attributes[kVertexAttributeTextureCoordinate].format =
												MTLVertexFormatFloat2;
										mVertexDescriptor.attributes[kVertexAttributeTextureCoordinate].offset = 8;
										mVertexDescriptor.attributes[kVertexAttributeTextureCoordinate].bufferIndex =
												kBufferIndexVertexTextureCoordinate;
										mVertexDescriptor.layouts[kBufferIndexVertexTextureCoordinate].stride = 20;
										mVertexDescriptor.layouts[kBufferIndexVertexTextureCoordinate].stepRate = 1;
										mVertexDescriptor.layouts[kBufferIndexVertexTextureCoordinate].stepFunction =
												MTLVertexStepFunctionPerVertex;

										mVertexDescriptor.attributes[kVertexAttributeTextureIndex].format =
												MTLVertexFormatFloat;
										mVertexDescriptor.attributes[kVertexAttributeTextureIndex].offset = 16;
										mVertexDescriptor.attributes[kVertexAttributeTextureIndex].bufferIndex =
												kBufferIndexVertexTextureIndex;
										mVertexDescriptor.layouts[kBufferIndexVertexTextureIndex].stride = 20;
										mVertexDescriptor.layouts[kBufferIndexVertexTextureIndex].stepRate = 1;
										mVertexDescriptor.layouts[kBufferIndexVertexTextureIndex].stepFunction =
												MTLVertexStepFunctionPerVertex;
									}

		UInt32					getPerVertexByteCount() const
									{ return sizeof(SVertex2DMultitexture); }

		bool					requiresDepthTest() const
									{ return false; }
		MTLVertexDescriptor*	getVertexDescriptor() const
									{ return mVertexDescriptor; }
		void					setup(id<MTLRenderCommandEncoder> renderCommandEncoder, id<MTLBuffer> vertexBuffer,
										MetalBufferCache* metalBufferCache) const
									{
										// Setup render command encoder
										[renderCommandEncoder setVertexBuffer:vertexBuffer offset:0
												atIndex:kBufferIndexVertexPosition];
										[renderCommandEncoder setVertexBuffer:vertexBuffer offset:0
												atIndex:kBufferIndexVertexTextureCoordinate];
										[renderCommandEncoder setVertexBuffer:vertexBuffer offset:0
												atIndex:kBufferIndexVertexTextureIndex];

										// Setup instance uniforms
										id<MTLBuffer>	basicVertexUniformsBuffer =
																[metalBufferCache
																		bufferWithLength:sizeof(BasicVertexUniforms)
																		options:MTLResourceStorageModeShared];
										basicVertexUniformsBuffer.label = @"Basic Vertex Uniforms";

//										BasicVertexUniforms*	basicVertexUniforms =
//																		(BasicVertexUniforms*)
//																				basicVertexUniformsBuffer.contents;

										[renderCommandEncoder setVertexBuffer:basicVertexUniformsBuffer offset:0
												atIndex:kBufferIndexVertexUniforms];
									}

		MTLVertexDescriptor*	mVertexDescriptor;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMetalVertexShaderClip

class CMetalVertexShaderClip : public CMetalVertexShader {
	public:
								CMetalVertexShaderClip() :
									CMetalVertexShader(CString(OSSTR("vertexShaderClip")))
									{
										// Setup metal vertex descriptor
										mVertexDescriptor = [[MTLVertexDescriptor alloc] init];

										mVertexDescriptor.attributes[kVertexAttributePosition].format =
												MTLVertexFormatFloat2;
										mVertexDescriptor.attributes[kVertexAttributePosition].offset = 0;
										mVertexDescriptor.attributes[kVertexAttributePosition].bufferIndex =
												kBufferIndexVertexPosition;
										mVertexDescriptor.layouts[kBufferIndexVertexPosition].stride = 20;
										mVertexDescriptor.layouts[kBufferIndexVertexPosition].stepRate = 1;
										mVertexDescriptor.layouts[kBufferIndexVertexPosition].stepFunction =
												MTLVertexStepFunctionPerVertex;

										mVertexDescriptor.attributes[kVertexAttributeTextureCoordinate].format =
												MTLVertexFormatFloat2;
										mVertexDescriptor.attributes[kVertexAttributeTextureCoordinate].offset = 8;
										mVertexDescriptor.attributes[kVertexAttributeTextureCoordinate].bufferIndex =
												kBufferIndexVertexTextureCoordinate;
										mVertexDescriptor.layouts[kBufferIndexVertexTextureCoordinate].stride = 20;
										mVertexDescriptor.layouts[kBufferIndexVertexTextureCoordinate].stepRate = 1;
										mVertexDescriptor.layouts[kBufferIndexVertexTextureCoordinate].stepFunction =
												MTLVertexStepFunctionPerVertex;

										mVertexDescriptor.attributes[kVertexAttributeTextureIndex].format =
												MTLVertexFormatFloat;
										mVertexDescriptor.attributes[kVertexAttributeTextureIndex].offset = 16;
										mVertexDescriptor.attributes[kVertexAttributeTextureIndex].bufferIndex =
												kBufferIndexVertexTextureIndex;
										mVertexDescriptor.layouts[kBufferIndexVertexTextureIndex].stride = 20;
										mVertexDescriptor.layouts[kBufferIndexVertexTextureIndex].stepRate = 1;
										mVertexDescriptor.layouts[kBufferIndexVertexTextureIndex].stepFunction =
												MTLVertexStepFunctionPerVertex;
									}

		UInt32					getPerVertexByteCount() const
									{ return sizeof(SVertex2DMultitexture); }

		bool					requiresDepthTest() const
									{ return false; }
		MTLVertexDescriptor*	getVertexDescriptor() const
									{ return mVertexDescriptor; }
		void					setup(id<MTLRenderCommandEncoder> renderCommandEncoder, id<MTLBuffer> vertexBuffer,
										MetalBufferCache* metalBufferCache) const
									{
										// Setup render command encoder
										[renderCommandEncoder setVertexBuffer:vertexBuffer offset:0
												atIndex:kBufferIndexVertexPosition];
										[renderCommandEncoder setVertexBuffer:vertexBuffer offset:0
												atIndex:kBufferIndexVertexTextureCoordinate];
										[renderCommandEncoder setVertexBuffer:vertexBuffer offset:0
												atIndex:kBufferIndexVertexTextureIndex];

										// Setup instance uniforms
										id<MTLBuffer>	clipVertexUniformsBuffer =
																[metalBufferCache
																		bufferWithLength:sizeof(ClipVertexUniforms)
																		options:MTLResourceStorageModeShared];
										clipVertexUniformsBuffer.label = @"Clip Vertex Uniforms";

										ClipVertexUniforms*	clipVertexUniforms =
																		(ClipVertexUniforms*)
																				clipVertexUniformsBuffer.contents;
										clipVertexUniforms->mClipPlane = *((vector_float4*) &mClipPlane);

										[renderCommandEncoder setVertexBuffer:clipVertexUniformsBuffer offset:0
												atIndex:kBufferIndexVertexUniforms];
									}

		void					setClipPlane(const SMatrix4x1_32& clipPlane)
									{ mClipPlane = clipPlane; }

		MTLVertexDescriptor*	mVertexDescriptor;
		SMatrix4x1_32			mClipPlane;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUVertexShader

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CGPUVertexShader& CGPUVertexShader::getBasic2DMultiTexture()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	CMetalVertexShaderBasic*	sVertexShader = nil;

	// Check if have shader
	if (sVertexShader == nil)
		// Create shader
		sVertexShader = new CMetalVertexShaderBasic();

	return *sVertexShader;
}

//----------------------------------------------------------------------------------------------------------------------
CGPUVertexShader& CGPUVertexShader::getClip2DMultiTexture(const SMatrix4x1_32& clipPlane)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	CMetalVertexShaderClip*	sVertexShader = nil;

	// Check if have shader
	if (sVertexShader == nil)
		// Create shader
		sVertexShader = new CMetalVertexShaderClip();

	// Setup
	sVertexShader->setClipPlane(clipPlane);

	return *sVertexShader;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMetalFragmentShaderBasic

class CMetalFragmentShaderBasic : public CMetalFragmentShader {
	public:
				CMetalFragmentShaderBasic() : CMetalFragmentShader(CString(OSSTR("fragmentShaderRGBABasic"))) {}

		void	setup(id<MTLRenderCommandEncoder> renderCommandEncoder, MetalBufferCache* metalBufferCache) const
					{}
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMetalFragmentShaderRGBAMultiTexture

class CMetalFragmentShaderRGBAMultiTexture : public CMetalFragmentShader {
	public:
				CMetalFragmentShaderRGBAMultiTexture() :
					CMetalFragmentShader(CString(OSSTR("fragmentShaderRGBAMultiTexture"))), mOpacity(1.0)
					{}

		void	setup(id<MTLRenderCommandEncoder> renderCommandEncoder, MetalBufferCache* metalBufferCache) const
					{
						// Setup instance uniforms
						id<MTLBuffer>	rgbaMultiTextureUniformsBuffer =
												[metalBufferCache bufferWithLength:sizeof(SRGBAMultiTextureUniforms)
														options:MTLResourceStorageModeShared];
						rgbaMultiTextureUniformsBuffer.label = @"RGBA Multi-Texture Fragment Shader";

						SRGBAMultiTextureUniforms*	rgbaMultiTextureUniforms =
															(SRGBAMultiTextureUniforms*)
																	rgbaMultiTextureUniformsBuffer.contents;
						rgbaMultiTextureUniforms->mOpacity = mOpacity;

						[renderCommandEncoder setFragmentBuffer:rgbaMultiTextureUniformsBuffer offset:0
								atIndex:kBufferIndexFragmentUniforms];
					}

		void	setOpacity(Float32 opacity)
					{ mOpacity = opacity; }

		Float32	mOpacity;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMetalFragmentShaderYCbCr

class CMetalFragmentShaderYCbCr : public CMetalFragmentShader {
	public:
				CMetalFragmentShaderYCbCr() :
					CMetalFragmentShader(CString(OSSTR("fragmentShaderYCbCr"))), mOpacity(1.0)
					{}

		void	setup(id<MTLRenderCommandEncoder> renderCommandEncoder, MetalBufferCache* metalBufferCache) const
					{
						// Setup instance uniforms
						id<MTLBuffer>	yCbCrUniformsBuffer =
												[metalBufferCache bufferWithLength:sizeof(SYCbCrUniforms)
														options:MTLResourceStorageModeShared];
						yCbCrUniformsBuffer.label = @"YCbCr Fragment Shader";

						SYCbCrUniforms*	yCbCrUniforms = (SYCbCrUniforms*) yCbCrUniformsBuffer.contents;
						yCbCrUniforms->mColorConversionMatrix = mColorConversionMatrix;
						yCbCrUniforms->mOpacity = mOpacity;

						[renderCommandEncoder setFragmentBuffer:yCbCrUniformsBuffer offset:0
								atIndex:kBufferIndexFragmentUniforms];
					}

		void	setColorConversionMatrix(const SMatrix3x3_32& colorConversionMatrix)
					{
						mColorConversionMatrix.columns[0] =
								simd_make_float3(colorConversionMatrix.m1_1, colorConversionMatrix.m2_1,
										colorConversionMatrix.m3_1);
						mColorConversionMatrix.columns[1] =
								simd_make_float3(colorConversionMatrix.m1_2, colorConversionMatrix.m2_2,
										colorConversionMatrix.m3_2);
						mColorConversionMatrix.columns[2] =
								simd_make_float3(colorConversionMatrix.m1_3, colorConversionMatrix.m2_3,
										colorConversionMatrix.m3_3);
					}
		void	setOpacity(Float32 opacity)
					{ mOpacity = opacity; }

		matrix_float3x3	mColorConversionMatrix;
		Float32			mOpacity;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUFragmentShader

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CGPUFragmentShader& CGPUFragmentShader::getRGBAMultiTexture(Float32 opacity)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check opacity
	if (opacity == 1.0) {
		// No opacity
		static	CMetalFragmentShaderBasic*		sFragmentShaderBasic = nil;
		if (sFragmentShaderBasic == nil)
			// Create shader
			sFragmentShaderBasic = new CMetalFragmentShaderBasic();

		return *sFragmentShaderBasic;
	} else {
		// Have opacity
		static	CMetalFragmentShaderRGBAMultiTexture*	sFragmentShaderRGBAMultiTexture = nil;
		if (sFragmentShaderRGBAMultiTexture == nil)
			// Create shader
			sFragmentShaderRGBAMultiTexture = new CMetalFragmentShaderRGBAMultiTexture();

		// Setup
		sFragmentShaderRGBAMultiTexture->setOpacity(opacity);

		return *sFragmentShaderRGBAMultiTexture;
	}
}

//----------------------------------------------------------------------------------------------------------------------
CGPUFragmentShader::Proc CGPUFragmentShader::getProc(CColor::Primaries primaries,
		CColor::YCbCrConversionMatrix yCbCrConversionMatrix, CColor::TransferFunction transferFunction)
//----------------------------------------------------------------------------------------------------------------------
{
	return sYCbCrFragmentShader;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions

//----------------------------------------------------------------------------------------------------------------------
CGPUFragmentShader&	sYCbCrFragmentShader(Float32 opacity)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	CMetalFragmentShaderYCbCr*	sFragmentShader = nil;

	// Check if have shader
	if (sFragmentShader == nil)
		// Create shader
		sFragmentShader = new CMetalFragmentShaderYCbCr();

	// Setup
	sFragmentShader->setColorConversionMatrix(CColor::mYCbCrConverstionMatrixRec601VideoRange);
	sFragmentShader->setOpacity(opacity);

	return *sFragmentShader;
}
