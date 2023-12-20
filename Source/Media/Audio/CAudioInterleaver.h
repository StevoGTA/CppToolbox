//----------------------------------------------------------------------------------------------------------------------
//	CAudioInterleaver.h			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioProcessor.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioInterleaver

class CAudioInterleaver : public CBasicAudioProcessor {
	// Classes
	private:
		class Internals;

	// Methods
	public:
										// Lifecycle methods
										CAudioInterleaver();
										~CAudioInterleaver();

										// CAudioProcessor methods
		OV<SError>						connectInput(const I<CAudioProcessor>& audioProcessor,
												const SAudio::ProcessingFormat& audioProcessingFormat);
		TArray<CString>					getSetupDescription(const CString& indent);

		TVResult<SourceInfo>			performInto(CAudioFrames& audioFrames);

		TArray<SAudio::ProcessingSetup>	getInputSetups() const;

		TArray<SAudio::ProcessingSetup>	getOutputSetups() const;

	// Properties
	private:
		Internals*	mInternals;
};
