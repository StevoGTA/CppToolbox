//----------------------------------------------------------------------------------------------------------------------
//	CCoreAudioAudioConverter.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioConverter.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CCoreAudioAudioConverter

class CCoreAudioAudioConverter : public CAudioConverter {
	// Classes
	private:
		class Internals;

	public:
										// Lifecycle methods
										CCoreAudioAudioConverter();
										~CCoreAudioAudioConverter();

										// CAudioProcessor methods
		OV<SError>						connectInput(const I<CAudioProcessor>& audioProcessor,
												const SAudioProcessingFormat& audioProcessingFormat);
		TArray<CString>					getSetupDescription(const CString& indent);

		SAudioSourceStatus				performInto(CAudioFrames& audioFrames);

		void							reset();

		TArray<SAudioProcessingSetup>	getInputSetups() const;

										// CAudioConverter methods
		bool							supportsNoninterleaved() const
											{ return true; }

	// Properties
	private:
		Internals*	mInternals;
};
