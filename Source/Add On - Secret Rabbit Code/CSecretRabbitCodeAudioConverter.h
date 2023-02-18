//----------------------------------------------------------------------------------------------------------------------
//	CSecretRabbitCodeAudioConverter.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioConverter.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSecretRabbitCodeAudioConverter

class CSecretRabbitCodeAudioConverterInternals;
class CSecretRabbitCodeAudioConverter : public CAudioConverter {
	public:
										// Lifecycle methods
										CSecretRabbitCodeAudioConverter();
										~CSecretRabbitCodeAudioConverter();

										// CAudioProcessor methods
		OV<SError>						connectInput(const I<CAudioProcessor>& audioProcessor,
												const SAudioProcessingFormat& audioProcessingFormat);
		TArray<CString>					getSetupDescription(const CString& indent);
	
		SAudioSourceStatus				performInto(CAudioFrames& audioFrames);
		void							reset();

		TArray<SAudioProcessingSetup>	getInputSetups() const;

										// CAudioConverter methods
		bool							supportsNoninterleaved() const
											{ return false; }

	// Properties
	private:
		CSecretRabbitCodeAudioConverterInternals*	mInternals;
};
