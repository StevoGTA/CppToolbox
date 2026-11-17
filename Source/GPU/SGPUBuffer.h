//----------------------------------------------------------------------------------------------------------------------
//	SGPUBuffer.h			©2019 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "C2DGeometry.h"
#include "CData.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SGPUBuffer

struct SGPUBuffer {
	// Methods
	public:
					// Lifecycle methods
		explicit	SGPUBuffer(void* platformReference) : mPlatformReference(platformReference) {}

	// Properties
	private:
		void*	mPlatformReference;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SGPUVertexBuffer

struct SGPUVertexBuffer : SGPUBuffer {
	// Methods
	public:
		// Lifecycle methods
		SGPUVertexBuffer(UInt32 perVertexByteCount, void* platformReference) :
			SGPUBuffer(platformReference), mPerVertexByteCount(perVertexByteCount)
			{}

	// Properties
	private:
		UInt32	mPerVertexByteCount;
};
