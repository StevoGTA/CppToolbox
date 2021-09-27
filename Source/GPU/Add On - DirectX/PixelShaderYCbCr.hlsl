// Globals
Texture2D	gTextures[16];

// Constants
cbuffer ConstantBuffer {
	float3x3	mColorConversionMatrix;
	float		mOpacity;
};

// SamplerState
SamplerState	textureSampler : s0;

// PixelShaderInput
struct PixelShaderInput {
	float4	mPosition : SV_POSITION;
	float3	mTextureCoordinate : TEXCOORD0;
	float	mClipDistance : SV_ClipDistance0;
};

float4 main(PixelShaderInput input) : SV_TARGET
{
	// Setup
	float	textureWidth, textureHeight;
	gTextures[0].GetDimensions(textureWidth, textureHeight);
	float2	textureSize = float2(textureWidth, textureHeight);

	float2	textureUV = input.mTextureCoordinate.xy / textureSize;

	// Collect Y and UV values
	float3	yuv;
	yuv.x = gTextures[0].Sample(textureSampler, textureUV).x - 16.0 / 255.0;
	yuv.yz = gTextures[1].Sample(textureSampler, textureUV).xy - float2(128.0 / 255.0, 128.0 / 255.0);

	// Convert to RGB
	float3	rgb = mul(yuv, mColorConversionMatrix);

    return float4(rgb, mOpacity);
}
