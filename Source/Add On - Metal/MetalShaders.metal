//----------------------------------------------------------------------------------------------------------------------
//	MetalShaders.metal			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include <metal_stdlib>

#include "MetalShaderTypes.h"

using namespace metal;

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
	float	mClipDistance [[clip_distance]];
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: Vertex Shaders

vertex VertexToFragmentInfo vertexShaderBasic(VertexInfo vertexInfo [[stage_in]],
		constant GlobalUniforms& globalUniforms [[ buffer(kBufferIndexGlobalUniforms) ]],
		constant BasicVertexUniforms& basicVertexUniforms [[ buffer(kBufferIndexVertexUniforms) ]])
{
	// Setup
    VertexToFragmentInfo vertexToFragmentInfo;
    vertexToFragmentInfo.mPosition =
			globalUniforms.mProjectionViewMatrix * basicVertexUniforms.mModelMatrix * vertexInfo.mPosition;
    vertexToFragmentInfo.mTextureCoordinate = vertexInfo.mTextureCoordinate;
    vertexToFragmentInfo.mTextureIndex = vertexInfo.mTextureIndex;
    vertexToFragmentInfo.mClipDistance = 0.0;

    return vertexToFragmentInfo;
}

vertex VertexToFragmentInfo vertexShaderClip(VertexInfo vertexInfo [[stage_in]],
		constant GlobalUniforms& globalUniforms [[ buffer(kBufferIndexGlobalUniforms) ]],
		constant ClipVertexUniforms& clipVertexUniforms [[ buffer(kBufferIndexVertexUniforms) ]])
{
	// Setup
    VertexToFragmentInfo vertexToFragmentInfo;
    vertexToFragmentInfo.mPosition =
			globalUniforms.mProjectionViewMatrix * clipVertexUniforms.mModelMatrix * vertexInfo.mPosition;
    vertexToFragmentInfo.mTextureCoordinate = vertexInfo.mTextureCoordinate;
    vertexToFragmentInfo.mTextureIndex = vertexInfo.mTextureIndex;
    vertexToFragmentInfo.mClipDistance =
    		dot(clipVertexUniforms.mModelMatrix * vertexInfo.mPosition, clipVertexUniforms.mClipPlane);

    return vertexToFragmentInfo;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: Fragment Shaders

fragment float4 fragmentShaderBasic(VertexToFragmentInfo vertexToFragmentInfo [[stage_in]],
		texture2d<half> colorMap [[ texture(0) ]], sampler sampler2d [[sampler(0)]])
{
    // Compose color
	float2	normalizedTextureCoordinate(vertexToFragmentInfo.mTextureCoordinate.x / colorMap.get_width(),
					vertexToFragmentInfo.mTextureCoordinate.y / colorMap.get_height());
	float4	color = float4(colorMap.sample(sampler2d, normalizedTextureCoordinate.xy));

	return color;
}

fragment float4 fragmentShaderOpacity(VertexToFragmentInfo vertexToFragmentInfo [[stage_in]],
		constant OpacityFragmentUniforms& opacityFragmentUniforms [[ buffer(kBufferIndexFragmentUniforms) ]],
		texture2d<half> colorMap [[ texture(0) ]], sampler sampler2d [[sampler(0)]])
{
    // Compose color
	float2	normalizedTextureCoordinate(vertexToFragmentInfo.mTextureCoordinate.x / colorMap.get_width(),
					vertexToFragmentInfo.mTextureCoordinate.y / colorMap.get_height());
	float4	color = float4(colorMap.sample(sampler2d, normalizedTextureCoordinate.xy));
	color.a *= opacityFragmentUniforms.mOpacity;

	return color;
}
