//----------------------------------------------------------------------------------------------------------------------
//	CCoreAudioAudioConverter.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioProcessor.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CCoreAudioAudioConverter

class CCoreAudioAudioConverterInternals;
class CCoreAudioAudioConverter : public CAudioConverter {
	public:
							// Lifecycle methods
							CCoreAudioAudioConverter();
							~CCoreAudioAudioConverter();

							// CAudioProcessor methods
		OV<SError>			connectInput(const I<CAudioProcessor>& audioProcessor,
									const SAudioProcessingFormat& audioProcessingFormat);
		TNArray<CString>	getSetupDescription(const CString& indent);

		SAudioSourceStatus	performInto(CAudioFrames& audioFrames);

		void				reset();

	// Properties
	private:
		CCoreAudioAudioConverterInternals*	mInternals;
};
