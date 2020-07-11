//----------------------------------------------------------------------------------------------------------------------
//	CMetalShaderBuiltIns.mm			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#import "CMetalShader.h"

#import "MetalShaderTypes.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMetalVertexShaderBasic

class CMetalVertexShaderBasic : public CMetalVertexShader {
	public:
				CMetalVertexShaderBasic() : CMetalVertexShader() {}

		CString	getName() const
					{ return CString(OSSTR("vertexShaderBasic")); }

		void	setup(id<MTLRenderCommandEncoder> renderCommandEncoder, MetalBufferCache* metalBufferCache) const
					{
						// Setup instance uniforms
						id<MTLBuffer>	basicVertexUniformsBuffer =
												[metalBufferCache bufferWithLength:sizeof(BasicVertexUniforms)
														options:MTLResourceStorageModeShared];
						basicVertexUniformsBuffer.label = @"Basic Vertex Uniforms";

						BasicVertexUniforms*	basicVertexUniforms =
														(BasicVertexUniforms*) basicVertexUniformsBuffer.contents;
						basicVertexUniforms->mModelMatrix = *((matrix_float4x4*) &getModelMatrix());

						[renderCommandEncoder setVertexBuffer:basicVertexUniformsBuffer offset:0
								atIndex:kBufferIndexVertexUniforms];
					}
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMetalVertexShaderClip

class CMetalVertexShaderClip : public CMetalVertexShader {
	public:
				CMetalVertexShaderClip() : CMetalVertexShader() {}

		CString	getName() const
					{ return CString(OSSTR("vertexShaderClip")); }

		void	setup(id<MTLRenderCommandEncoder> renderCommandEncoder, MetalBufferCache* metalBufferCache) const
					{
						// Setup instance uniforms
						id<MTLBuffer>	clipVertexUniformsBuffer =
												[metalBufferCache bufferWithLength:sizeof(ClipVertexUniforms)
														options:MTLResourceStorageModeShared];
						clipVertexUniformsBuffer.label = @"Clip Vertex Uniforms";

						ClipVertexUniforms*	clipVertexUniforms =
														(ClipVertexUniforms*) clipVertexUniformsBuffer.contents;
						clipVertexUniforms->mModelMatrix = *((matrix_float4x4*) &getModelMatrix());
						clipVertexUniforms->mClipPlane = *((vector_float4*) &mClipPlane);

						[renderCommandEncoder setVertexBuffer:clipVertexUniformsBuffer offset:0
								atIndex:kBufferIndexVertexUniforms];
					}

		void	setClipPlane(const SMatrix4x1_32& clipPlane)
					{ mClipPlane = clipPlane; }

		SMatrix4x1_32	mClipPlane;
};

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
	if (sVertexShader == nil)
		// Create shader
		sVertexShader = new CMetalVertexShaderBasic();

	return *sVertexShader;
}

//----------------------------------------------------------------------------------------------------------------------
CGPUVertexShader& CGPUVertexShader::getClip(const SMatrix4x1_32& clipPlane)
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
				CMetalFragmentShaderBasic() : CMetalFragmentShader() {}

		CString	getName() const
					{ return CString(OSSTR("fragmentShaderBasic")); }

		void	setup(id<MTLRenderCommandEncoder> renderCommandEncoder, MetalBufferCache* metalBufferCache) const
					{}
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMetalFragmentShaderOpacity

class CMetalFragmentShaderOpacity : public CMetalFragmentShader {
	public:
				CMetalFragmentShaderOpacity() : CMetalFragmentShader(), mOpacity(1.0) {}

		CString	getName() const
					{ return CString(OSSTR("fragmentShaderOpacity")); }

		void	setup(id<MTLRenderCommandEncoder> renderCommandEncoder, MetalBufferCache* metalBufferCache) const
					{
						// Setup instance uniforms
						id<MTLBuffer>	opacityFragmentUniformsBuffer =
												[metalBufferCache bufferWithLength:sizeof(OpacityFragmentUniforms)
														options:MTLResourceStorageModeShared];
						opacityFragmentUniformsBuffer.label = @"Opacity Fragment Shader";

						OpacityFragmentUniforms*	opacityFragmentUniforms =
														(OpacityFragmentUniforms*)
																opacityFragmentUniformsBuffer.contents;
						opacityFragmentUniforms->mOpacity = mOpacity;

						[renderCommandEncoder setFragmentBuffer:opacityFragmentUniformsBuffer offset:0
								atIndex:kBufferIndexFragmentUniforms];
					}

		void	setOpacity(Float32 opacity)
					{ mOpacity = opacity; }

		Float32	mOpacity;
};

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
	if (sFragmentShader == nil)
		// Create shader
		sFragmentShader = new CMetalFragmentShaderBasic();

	return *sFragmentShader;
}

//----------------------------------------------------------------------------------------------------------------------
CGPUFragmentShader& CGPUFragmentShader::getOpacity(Float32 opacity)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	CMetalFragmentShaderOpacity*	sFragmentShader = nil;

	// Check if have shader
	if (sFragmentShader == nil)
		// Create shader
		sFragmentShader = new CMetalFragmentShaderOpacity();

	// Setup
	sFragmentShader->setOpacity(opacity);

	return *sFragmentShader;
}
