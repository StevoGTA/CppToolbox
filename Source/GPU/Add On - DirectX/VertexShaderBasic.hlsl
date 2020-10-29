// Constants
cbuffer ConstantBuffer : register(b0) {
	matrix	mProjectionMatrix;
	matrix	mViewMatrix;
	matrix	mModelMatrix;
};

// VertexShaderInput
struct VertexShaderInput {
	float3	mPosition : POSITION;
	float3	mTextureCoordinate : TEXCOORD0;
};

// PixelShaderInput
struct PixelShaderInput {
	float4	mPosition : SV_POSITION;
	float3	mTextureCoordinate : TEXCOORD0;
	float	mClipDistance : SV_ClipDistance0;
};

// Function
PixelShaderInput main(VertexShaderInput input) {
	// Setup
	float4 position = float4(input.mPosition, 1.0f);
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
