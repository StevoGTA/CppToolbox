//----------------------------------------------------------------------------------------------------------------------
//	SGPUBuffer.h			©2019 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "C2DGeometry.h"
#include "CData.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SGPUBuffer

struct SGPUBuffer {
	// Lifecycle methods
	SGPUBuffer(void* internalReference = nil) : mInternalReference(internalReference) {}
	SGPUBuffer(const SGPUBuffer& other) : mInternalReference(other.mInternalReference) {}

	// Properties
	void*	mInternalReference;
};
