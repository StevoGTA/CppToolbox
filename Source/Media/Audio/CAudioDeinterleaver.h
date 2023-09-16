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
												const SAudio::ProcessingFormat& audioProcessingFormat);
		TArray<CString>					getSetupDescription(const CString& indent);

		TVResult<SMedia::SourceInfo>	performInto(CAudioFrames& audioFrames);

		TArray<SAudio::ProcessingSetup>	getInputSetups() const;

		TArray<SAudio::ProcessingSetup>	getOutputSetups() const;

	// Properties
	private:
		Internals*	mInternals;
};
