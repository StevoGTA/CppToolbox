// Constants
cbuffer ConstantBuffer : register(b0) {
	matrix	mProjectionMatrix;
	matrix	mViewMatrix;
	matrix	mModelMatrix;
};

// VertexShaderInput
struct VertexShaderInput {
	float2	mPosition : POSITION;
	float2	mTextureCoordinate : TEXCOORD0;
};

// PixelShaderInput
struct PixelShaderInput {
	float4	mPosition : SV_POSITION;
	float2	mTextureCoordinate : TEXCOORD0;
	float	mClipDistance : SV_ClipDistance0;
};

// Function
PixelShaderInput main(VertexShaderInput input)
{
	// Setup
	float4 position = float4(input.mPosition, 0.0f, 1.0f);
	position = mul(position, mModelMatrix);
	position = mul(position, mViewMatrix);
	position = mul(position, mProjectionMatrix);

	// Compose output
	PixelShaderInput pixelShaderInput;
	pixelShaderInput.mPosition = position;
	pixelShaderInput.mTextureCoordinate = input.mTextureCoordinate;
	pixelShaderInput.mClipDistance = 1.0;

	return pixelShaderInput;
}
