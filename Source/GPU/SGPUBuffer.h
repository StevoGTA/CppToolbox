//----------------------------------------------------------------------------------------------------------------------
//	SGPUBuffer.h			©2019 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CData.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SGPUBuffer

struct SGPUBuffer {
	// Lifecycle methods
	SGPUBuffer(CData data, void* internalReference = nil) : mData(data), mInternalReference(internalReference) {}
	SGPUBuffer(const SGPUBuffer& other) : mData(other.mData), mInternalReference(other.mInternalReference) {}

	// Properties
	CData	mData;
	void*	mInternalReference;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - EGPUVertexBufferType

enum EGPUVertexBufferType {
	kGPUVertexBufferType2Vertex2Texture,
	kGPUVertexBufferType3Vertex2Texture,
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SGPUVertexBuffer

struct SGPUVertexBuffer : SGPUBuffer {
	// Lifecycle methods
	SGPUVertexBuffer(EGPUVertexBufferType gpuVertexBufferType, CData data, void* internalReference) :
		SGPUBuffer(data, internalReference), mGPUVertexBufferType(gpuVertexBufferType)
		{}
	SGPUVertexBuffer(const SGPUVertexBuffer& other) :
		SGPUBuffer(other), mGPUVertexBufferType(other.mGPUVertexBufferType)
		{}

	// Properties
	EGPUVertexBufferType	mGPUVertexBufferType;
};
