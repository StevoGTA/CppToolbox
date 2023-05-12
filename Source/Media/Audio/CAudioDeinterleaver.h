//----------------------------------------------------------------------------------------------------------------------
//	CAudioDeinterleaver.h			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioProcessor.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioDeinterleaver

class CAudioDeinterleaver : public CBasicAudioProcessor {
	// Classes
	private:
		class Internals;

	// Methods
	public:
										// Lifecycle methods
										CAudioDeinterleaver();
										~CAudioDeinterleaver();

										// CAudioProcessor methods
		OV<SError>						connectInput(const I<CAudioProcessor>& audioProcessor,
												const SAudioProcessingFormat& audioProcessingFormat);
		TArray<CString>					getSetupDescription(const CString& indent);

		SAudioSourceStatus				performInto(CAudioFrames& audioFrames);

		TArray<SAudioProcessingSetup>	getInputSetups() const;

		TArray<SAudioProcessingSetup>	getOutputSetups() const;

	// Properties
	private:
		Internals*	mInternals;
};
