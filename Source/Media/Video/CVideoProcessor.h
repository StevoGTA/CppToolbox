//----------------------------------------------------------------------------------------------------------------------
//	CVideoProcessor.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "SError.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CVideoProcessor

class CVideoProcessorInternals;
class CVideoProcessor {
	// Methods
	public:
							// Lifecycle methods
							CVideoProcessor();
		virtual				~CVideoProcessor();

							// Instance methods
		virtual	OI<SError>	reset() = 0;

	// Properties
	private:
		CVideoProcessorInternals*	mInternals;
};
