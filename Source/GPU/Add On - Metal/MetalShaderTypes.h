//----------------------------------------------------------------------------------------------------------------------
//	MetalShaderTypes.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include <simd/simd.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: Structures

struct GlobalUniforms {
	matrix_float4x4	mModelMatrix;
	matrix_float4x4	mViewMatrix;
	matrix_float4x4	mProjectionMatrix;
};

struct BasicVertexUniforms {
};

struct ClipVertexUniforms {
	vector_float4	mClipPlane;
};

struct OpacityFragmentUniforms {
	float	mOpacity;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: Enumerations

enum VertexAttribute {
	kVertexAttributePosition,
	kVertexAttributeColor,
	kVertexAttributeTextureCoordinate,
	kVertexAttributeTextureIndex,
};

enum BufferIndex {
	kBufferIndexVertexPosition,
	kBufferIndexVertexColor,
	kBufferIndexVertexTextureCoordinate,
	kBufferIndexVertexTextureIndex,
	kBufferIndexGlobalUniforms,
	kBufferIndexVertexUniforms,
	kBufferIndexFragmentUniforms,
};
