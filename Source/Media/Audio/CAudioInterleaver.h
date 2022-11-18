//----------------------------------------------------------------------------------------------------------------------
//	CAudioInterleaver.h			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioProcessor.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioInterleaver

class CAudioInterleaverInternals;
class CAudioInterleaver : public CBasicAudioProcessor {
	// Methods
	public:
										// Lifecycle methods
										CAudioInterleaver();
										~CAudioInterleaver();

										// CAudioProcessor methods
		OV<SError>						connectInput(const I<CAudioProcessor>& audioProcessor,
												const SAudioProcessingFormat& audioProcessingFormat);
		TNArray<CString>				getSetupDescription(const CString& indent);

		SAudioSourceStatus				performInto(CAudioFrames& audioFrames);

		TArray<SAudioProcessingSetup>	getInputSetups() const;

		TArray<SAudioProcessingSetup>	getOutputSetups() const;

	// Properties
	private:
		CAudioInterleaverInternals*	mInternals;
};
