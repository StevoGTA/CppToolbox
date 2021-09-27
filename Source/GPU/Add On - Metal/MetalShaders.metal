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
// MARK: - Vertex Shaders

//----------------------------------------------------------------------------------------------------------------------
vertex VertexToFragmentInfo vertexShaderBasic(VertexInfo vertexInfo [[stage_in]],
		constant GlobalUniforms& globalUniforms [[ buffer(kBufferIndexGlobalUniforms) ]],
		constant BasicVertexUniforms& basicVertexUniforms [[ buffer(kBufferIndexVertexUniforms) ]])
{
	// Setup
    VertexToFragmentInfo vertexToFragmentInfo;
    vertexToFragmentInfo.mPosition =
			globalUniforms.mProjectionMatrix * globalUniforms.mViewMatrix * globalUniforms.mModelMatrix *
					vertexInfo.mPosition;
    vertexToFragmentInfo.mTextureCoordinate = vertexInfo.mTextureCoordinate;
    vertexToFragmentInfo.mTextureIndex = vertexInfo.mTextureIndex;
    vertexToFragmentInfo.mClipDistance = 0.0;

    return vertexToFragmentInfo;
}

//----------------------------------------------------------------------------------------------------------------------
vertex VertexToFragmentInfo vertexShaderClip(VertexInfo vertexInfo [[stage_in]],
		constant GlobalUniforms& globalUniforms [[ buffer(kBufferIndexGlobalUniforms) ]],
		constant ClipVertexUniforms& clipVertexUniforms [[ buffer(kBufferIndexVertexUniforms) ]])
{
	// Setup
    VertexToFragmentInfo vertexToFragmentInfo;
    vertexToFragmentInfo.mPosition =
			globalUniforms.mProjectionMatrix * globalUniforms.mViewMatrix * globalUniforms.mModelMatrix *
					vertexInfo.mPosition;
    vertexToFragmentInfo.mTextureCoordinate = vertexInfo.mTextureCoordinate;
    vertexToFragmentInfo.mTextureIndex = vertexInfo.mTextureIndex;
    vertexToFragmentInfo.mClipDistance =
    		dot(globalUniforms.mViewMatrix * globalUniforms.mModelMatrix * vertexInfo.mPosition,
    				clipVertexUniforms.mClipPlane);

    return vertexToFragmentInfo;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Fragment Shaders

//----------------------------------------------------------------------------------------------------------------------
float4 sample(float2 point, const array<texture2d<float, access::sample>, 16> textures, int textureIndex,
		sampler sampler2D)
{
	// What is texture index
	if (textureIndex == 0) {
		// 0
		texture2d<float>	texture = textures[0];
		float2				textureUV(point.x / texture.get_width(), point.y / texture.get_height());

		return texture.sample(sampler2D, textureUV.xy);
	} else if (textureIndex == 1) {
		// 1
		texture2d<float>	texture = textures[1];
		float2				textureUV(point.x / texture.get_width(), point.y / texture.get_height());

		return texture.sample(sampler2D, textureUV.xy);
	} else if (textureIndex == 2) {
		// 2
		texture2d<float>	texture = textures[2];
		float2				textureUV(point.x / texture.get_width(), point.y / texture.get_height());

		return texture.sample(sampler2D, textureUV.xy);
	} else if (textureIndex == 3) {
		// 3
		texture2d<float>	texture = textures[3];
		float2				textureUV(point.x / texture.get_width(), point.y / texture.get_height());

		return texture.sample(sampler2D, textureUV.xy);
	} else if (textureIndex == 4) {
		// 4
		texture2d<float>	texture = textures[4];
		float2				textureUV(point.x / texture.get_width(), point.y / texture.get_height());

		return texture.sample(sampler2D, textureUV.xy);
	} else if (textureIndex == 5) {
		// 5
		texture2d<float>	texture = textures[5];
		float2				textureUV(point.x / texture.get_width(), point.y / texture.get_height());

		return texture.sample(sampler2D, textureUV.xy);
	} else if (textureIndex == 6) {
		// 6
		texture2d<float>	texture = textures[6];
		float2				textureUV(point.x / texture.get_width(), point.y / texture.get_height());

		return texture.sample(sampler2D, textureUV.xy);
	} else if (textureIndex == 7) {
		// 7
		texture2d<float>	texture = textures[7];
		float2				textureUV(point.x / texture.get_width(), point.y / texture.get_height());

		return texture.sample(sampler2D, textureUV.xy);
	} else if (textureIndex == 8) {
		// 8
		texture2d<float>	texture = textures[8];
		float2				textureUV(point.x / texture.get_width(), point.y / texture.get_height());

		return texture.sample(sampler2D, textureUV.xy);
	} else if (textureIndex == 9) {
		// 9
		texture2d<float>	texture = textures[9];
		float2				textureUV(point.x / texture.get_width(), point.y / texture.get_height());

		return texture.sample(sampler2D, textureUV.xy);
	} else if (textureIndex == 10) {
		// 10
		texture2d<float>	texture = textures[10];
		float2				textureUV(point.x / texture.get_width(), point.y / texture.get_height());

		return texture.sample(sampler2D, textureUV.xy);
	} else if (textureIndex == 11) {
		// 11
		texture2d<float>	texture = textures[11];
		float2				textureUV(point.x / texture.get_width(), point.y / texture.get_height());

		return texture.sample(sampler2D, textureUV.xy);
	} else if (textureIndex == 12) {
		// 12
		texture2d<float>	texture = textures[12];
		float2				textureUV(point.x / texture.get_width(), point.y / texture.get_height());

		return texture.sample(sampler2D, textureUV.xy);
	} else if (textureIndex == 13) {
		// 13
		texture2d<float>	texture = textures[13];
		float2				textureUV(point.x / texture.get_width(), point.y / texture.get_height());

		return texture.sample(sampler2D, textureUV.xy);
	} else if (textureIndex == 14) {
		// 14
		texture2d<float>	texture = textures[14];
		float2				textureUV(point.x / texture.get_width(), point.y / texture.get_height());

		return texture.sample(sampler2D, textureUV.xy);
	} else {
		// 15
		texture2d<float>	texture = textures[15];
		float2				textureUV(point.x / texture.get_width(), point.y / texture.get_height());

		return texture.sample(sampler2D, textureUV.xy);
	}
}

//----------------------------------------------------------------------------------------------------------------------
fragment float4 fragmentShaderRGBABasic(VertexToFragmentInfo vertexToFragmentInfo [[stage_in]],
		const array<texture2d<float, access::sample>, 16> textures [[ texture(0) ]], sampler sampler2D [[sampler(0)]])
{
	return sample(vertexToFragmentInfo.mTextureCoordinate, textures, vertexToFragmentInfo.mTextureIndex, sampler2D);
}

//----------------------------------------------------------------------------------------------------------------------
fragment float4 fragmentShaderRGBAMultiTexture(VertexToFragmentInfo vertexToFragmentInfo [[stage_in]],
		constant SRGBAMultiTextureUniforms& rgbaMultiTextureUniforms [[ buffer(kBufferIndexFragmentUniforms) ]],
		const array<texture2d<float, access::sample>, 16> textures [[ texture(0) ]], sampler sampler2D [[sampler(0)]])
{
    // Compose color
    float4	rgba =
    				sample(vertexToFragmentInfo.mTextureCoordinate, textures, vertexToFragmentInfo.mTextureIndex,
    						sampler2D);
	rgba.a *= rgbaMultiTextureUniforms.mOpacity;

	return rgba;
}

//----------------------------------------------------------------------------------------------------------------------
fragment float4 fragmentShaderYCbCr(VertexToFragmentInfo vertexToFragmentInfo [[stage_in]],
		constant SYCbCrUniforms& yCbCrUniforms [[ buffer(kBufferIndexFragmentUniforms) ]],
		const array<texture2d<float, access::sample>, 16> textures [[ texture(0) ]], sampler sampler2D [[sampler(0)]])
{
	// Setup
	float2	point = vertexToFragmentInfo.mTextureCoordinate;
	float3	yuv;

	// Collect Y and UV values
	texture2d<float>	texture = textures[0];
	float2				textureUV(point.x / texture.get_width(), point.y / texture.get_height());
	yuv.x = texture.sample(sampler2D, textureUV.xy).r - 16.0 / 255.0;

	texture = textures[1];
	yuv.yz = texture.sample(sampler2D, textureUV.xy).rg - float2(128.0 / 255.0);

	// Convert to RGB
	float3	rgb = yCbCrUniforms.mColorConversionMatrix * yuv;

	return float4(rgb, yCbCrUniforms.mOpacity);
}
