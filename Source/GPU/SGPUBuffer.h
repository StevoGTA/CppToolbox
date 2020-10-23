//----------------------------------------------------------------------------------------------------------------------
//	SGPUBuffer.h			©2019 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "C2DGeometry.h"
#include "CData.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SGPUBuffer

struct SGPUBuffer {
	// Lifecycle methods
	SGPUBuffer(void* platformReference) : mPlatformReference(platformReference) {}
	SGPUBuffer(const SGPUBuffer& other) : mPlatformReference(other.mPlatformReference) {}

	// Properties
	void*	mPlatformReference;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SGPUVertexBuffer

struct SGPUVertexBuffer : SGPUBuffer {
	// Lifecycle methods
	SGPUVertexBuffer(UInt32 perVertexByteCount, void* platformReference) :
		SGPUBuffer(platformReference), mPerVertexByteCount(perVertexByteCount)
		{}
	SGPUVertexBuffer(const SGPUVertexBuffer& other) : SGPUBuffer(other) {}

	// Properties
	UInt32	mPerVertexByteCount;
};
