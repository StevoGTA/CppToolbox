//----------------------------------------------------------------------------------------------------------------------
//	CSecretRabbitCodeAudioConverter.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioProcessor.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSecretRabbitCodeAudioConverter

class CSecretRabbitCodeAudioConverterInternals;
class CSecretRabbitCodeAudioConverter : public CAudioConverter {
	public:
							// Lifecycle methods
							CSecretRabbitCodeAudioConverter();
							~CSecretRabbitCodeAudioConverter();

							// CAudioProcessor methods
		OV<SError>			connectInput(const I<CAudioProcessor>& audioProcessor,
									const SAudioProcessingFormat& audioProcessingFormat);
		TNArray<CString>	getSetupDescription(const CString& indent);

		SAudioSourceStatus	performInto(CAudioFrames& audioFrames);
		void				reset();

	// Properties
	private:
		CSecretRabbitCodeAudioConverterInternals*	mInternals;
};
