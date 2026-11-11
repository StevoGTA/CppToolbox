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

	// Properties
	UInt32	mPerVertexByteCount;
};
