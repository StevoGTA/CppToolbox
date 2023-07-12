//----------------------------------------------------------------------------------------------------------------------
//	CSecretRabbitCodeAudioConverter.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioConverter.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSecretRabbitCodeAudioConverter

class CSecretRabbitCodeAudioConverter : public CAudioConverter {
	// Classes
	private:
		class Internals;

	// Methods
	public:
										// Lifecycle methods
										CSecretRabbitCodeAudioConverter();
										~CSecretRabbitCodeAudioConverter();

										// CAudioProcessor methods
		OV<SError>						connectInput(const I<CAudioProcessor>& audioProcessor,
												const SAudio::ProcessingFormat& audioProcessingFormat);
		TArray<CString>					getSetupDescription(const CString& indent);
	
		SAudioSourceStatus				performInto(CAudioFrames& audioFrames);
		void							reset();

		TArray<SAudio::ProcessingSetup>	getInputSetups() const;

										// CAudioConverter methods
		bool							supportsNoninterleaved() const
											{ return false; }

	// Properties
	private:
		Internals*	mInternals;
};
