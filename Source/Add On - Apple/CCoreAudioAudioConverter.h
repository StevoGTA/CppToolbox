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
												const SAudio::ProcessingFormat& audioProcessingFormat);
		TArray<CString>					getSetupDescription(const CString& indent);

		TVResult<SourceInfo>			performInto(CAudioFrames& audioFrames);

		void							seek(UniversalTimeInterval timeInterval);

		TArray<SAudio::ProcessingSetup>	getInputSetups() const;

										// CAudioConverter methods
		bool							supportsNoninterleaved() const
											{ return true; }

	// Properties
	private:
		Internals*	mInternals;
};
