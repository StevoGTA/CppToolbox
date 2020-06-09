//----------------------------------------------------------------------------------------------------------------------
//	MetalShaderTypes.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include <simd/simd.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: Structures

struct GlobalUniforms {
	matrix_float4x4 mProjectionMatrix;
	matrix_float4x4	mViewMatrix;
};

struct BasicVertexUniforms {
	matrix_float4x4 mModelMatrix;
};

struct ClipVertexUniforms {
	matrix_float4x4 mModelMatrix;
	vector_float4	mClipPlane;
};

struct OpacityFragmentUniforms {
	float	mOpacity;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: Enumerations

enum VertexAttribute {
	kVertexAttributePosition,
	kVertexAttributeTextureCoordinate,
	kVertexAttributeTextureIndex,
};

enum BufferIndex {
	kBufferIndexVertexPosition,
	kBufferIndexVertexTextureCoordinate,
	kBufferIndexVertexTextureIndex,
	kBufferIndexGlobalUniforms,
	kBufferIndexVertexUniforms,
	kBufferIndexFragmentUniforms,
};

enum TextureIndex {
	kTextureIndexColor,
};
