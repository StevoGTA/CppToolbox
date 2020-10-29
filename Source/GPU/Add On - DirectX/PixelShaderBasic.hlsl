// Globals
Texture2D	gTextures[16];

// SamplerState
SamplerState TextureSampler {
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = CLAMP;
	AddressV = CLAMP;
};

// PixelShaderInput
struct PixelShaderInput {
	float4	mPosition : SV_POSITION;
	float3	mTextureCoordinate : TEXCOORD0;
	float	mClipDistance : SV_ClipDistance0;
};

// Functions
float4 perform(PixelShaderInput input, Texture2D textureUse) {
	// Setup
	float	width, height;
	textureUse.GetDimensions(width, height);

	return textureUse.Sample(TextureSampler,
			float2(input.mTextureCoordinate.x / width, input.mTextureCoordinate.y / height));
}

float4 main(PixelShaderInput input) : SV_TARGET {
	// Check texture
	if (input.mTextureCoordinate.z == 0.0)
		// Texture 0
		return perform(input, gTextures[0]);
	else if (input.mTextureCoordinate.z == 1.0)
		// Texture 1
		return perform(input, gTextures[1]);
	else if (input.mTextureCoordinate.z == 2.0)
		// Texture 2
		return perform(input, gTextures[2]);
	else if (input.mTextureCoordinate.z == 3.0)
		// Texture 3
		return perform(input, gTextures[3]);
	else if (input.mTextureCoordinate.z == 4.0)
		// Texture 4
		return perform(input, gTextures[4]);
	else if (input.mTextureCoordinate.z == 5.0)
		// Texture 5
		return perform(input, gTextures[5]);
	else if (input.mTextureCoordinate.z == 6.0)
		// Texture 6
		return perform(input, gTextures[6]);
	else if (input.mTextureCoordinate.z == 7.0)
		// Texture 7
		return perform(input, gTextures[7]);
	else if (input.mTextureCoordinate.z == 8.0)
		// Texture 8
		return perform(input, gTextures[8]);
	else if (input.mTextureCoordinate.z == 9.0)
		// Texture 9
		return perform(input, gTextures[9]);
	else if (input.mTextureCoordinate.z == 10.0)
		// Texture 10
		return perform(input, gTextures[10]);
	else if (input.mTextureCoordinate.z == 11.0)
		// Texture 11
		return perform(input, gTextures[11]);
	else if (input.mTextureCoordinate.z == 12.0)
		// Texture 12
		return perform(input, gTextures[12]);
	else if (input.mTextureCoordinate.z == 13.0)
		// Texture 13
		return perform(input, gTextures[13]);
	else if (input.mTextureCoordinate.z == 14.0)
		// Texture 14
		return perform(input, gTextures[14]);
	else
		// Texture 15
		return perform(input, gTextures[15]);
}
