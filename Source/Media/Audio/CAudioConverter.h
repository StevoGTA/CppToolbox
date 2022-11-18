//----------------------------------------------------------------------------------------------------------------------
//	CAudioConverter.h			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioProcessor.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioConverter

class CAudioConverter : public CBasicAudioProcessor {
	// Methods
	public:
												// CAudioProcessor methods
				TArray<SAudioProcessingSetup>	getOutputSetups() const
													{ return TNArray<SAudioProcessingSetup>(
															SAudioProcessingSetup::mUnspecified); }

												// Instance methods
		virtual	bool							supportsNoninterleaved() const = 0;

	protected:
												// Lifecycle methods
												CAudioConverter() {}
};
