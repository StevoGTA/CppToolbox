//----------------------------------------------------------------------------------------------------------------------
//	MetalShaders.metal			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include <metal_stdlib>

#include "MetalShaderTypes.h"

using namespace metal;

/*
	Basic Vertex
	Clip Vertex

	Basic Fragment
	Opacity Fragment
 */
//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

struct VertexInfo {
	float4	mPosition [[attribute(kVertexAttributePosition)]];
	float2	mTextureCoordinate [[attribute(kVertexAttributeTextureCoordinate)]];
	float	mTextureIndex [[attribute(kVertexAttributeTextureIndex)]];
};

struct VertexToFragmentInfo {
	float4	mPosition [[position]];
	float2	mTextureCoordinate;
	float	mTextureIndex;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: Vertex Shaders

vertex VertexToFragmentInfo basicVertexShader(
											VertexInfo				vertexInfo [[stage_in]],
								constant	GlobalUniforms&			globalUniforms [[ buffer(kBufferIndexGlobalUniforms) ]],
								constant	BasicVertexUniforms&	basicVertexUniforms [[ buffer(kBufferIndexVertexUniforms) ]])
{
	// Setup
    VertexToFragmentInfo vertexToFragmentInfo;
    vertexToFragmentInfo.mPosition =
    		globalUniforms.mProjectionMatrix * globalUniforms.mViewMatrix * basicVertexUniforms.mModelMatrix *
    				vertexInfo.mPosition;
    vertexToFragmentInfo.mTextureCoordinate = vertexInfo.mTextureCoordinate;
    vertexToFragmentInfo.mTextureIndex = vertexInfo.mTextureIndex;

    return vertexToFragmentInfo;
}

vertex VertexToFragmentInfo clipVertexShader(
											VertexInfo			vertexInfo [[stage_in]],
								constant	GlobalUniforms&		globalUniforms [[ buffer(kBufferIndexGlobalUniforms) ]],
								constant	ClipVertexUniforms&	clipVertexUniforms [[ buffer(kBufferIndexVertexUniforms) ]])
{
	// Setup
    VertexToFragmentInfo vertexToFragmentInfo;
    vertexToFragmentInfo.mPosition =
    		globalUniforms.mProjectionMatrix * globalUniforms.mViewMatrix * clipVertexUniforms.mModelMatrix *
    				vertexInfo.mPosition;
    vertexToFragmentInfo.mTextureCoordinate = vertexInfo.mTextureCoordinate;
    vertexToFragmentInfo.mTextureIndex = vertexInfo.mTextureIndex;

    return vertexToFragmentInfo;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: Fragment Shaders

fragment float4 basicFragmentShader(	VertexToFragmentInfo	vertexToFragmentInfo [[stage_in]],
										texture2d<half>			colorMap [[ texture(kTextureIndexColor) ]])
{
	// Setup
    constexpr	sampler	colorSampler(mip_filter::linear, mag_filter::linear, min_filter::linear);

    // Compose color
	float2	normalizedTextureCoordinate(vertexToFragmentInfo.mTextureCoordinate.x / colorMap.get_width(),
					vertexToFragmentInfo.mTextureCoordinate.y / colorMap.get_height());
    half4 colorSample   = colorMap.sample(colorSampler, normalizedTextureCoordinate.xy);

    return float4(colorSample);
}

fragment float4 opacityFragmentShader(				VertexToFragmentInfo		vertexToFragmentInfo [[stage_in]],
										constant	OpacityFragmentUniforms&	opacityFragmentUniforms [[ buffer(kBufferIndexFragmentUniforms) ]],
													texture2d<half>				colorMap [[ texture(kTextureIndexColor) ]])
{
	// Setup
    constexpr	sampler	colorSampler(mip_filter::linear, mag_filter::linear, min_filter::linear);

    // Compose color
	float2	normalizedTextureCoordinate(vertexToFragmentInfo.mTextureCoordinate.x / colorMap.get_width(),
					vertexToFragmentInfo.mTextureCoordinate.y / colorMap.get_height());
    half4 colorSample   = colorMap.sample(colorSampler, normalizedTextureCoordinate.xy);

    return float4(colorSample);
}
