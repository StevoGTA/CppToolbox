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
		OI<SError>			connectInput(const I<CAudioProcessor>& audioProcessor,
									const SAudioProcessingFormat& audioProcessingFormat);

		SAudioReadStatus	perform(const SMediaPosition& mediaPosition, CAudioFrames& audioFrames);
		OI<SError>			reset();

	// Properties
	private:
		CCoreAudioAudioConverterInternals*	mInternals;
};
