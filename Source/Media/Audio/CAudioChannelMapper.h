//----------------------------------------------------------------------------------------------------------------------
//	CAudioChannelMapper.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioProcessor.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioChannelMapper

class CAudioChannelMapper : public CBasicAudioProcessor {
	// Classes
	private:
		class Internals;

	// Methods
	public:
												// Lifecycle methods
												CAudioChannelMapper();
												~CAudioChannelMapper();

												// CAudioProcessor methods
				OV<SError>						connectInput(const I<CAudioProcessor>& audioProcessor,
														const SAudio::ProcessingFormat& audioProcessingFormat);
				TArray<CString>					getSetupDescription(const CString& indent);

				SAudioSourceStatus				performInto(CAudioFrames& audioFrames);

				TArray<SAudio::ProcessingSetup>	getInputSetups() const;

				TArray<SAudio::ProcessingSetup>	getOutputSetups() const;

												// Class methods
		static	bool							canPerform(const SAudio::ChannelMap& fromAudioChannelMap,
														const SAudio::ChannelMap& toAudioChannelMap);

	// Properties
	private:
		Internals*	mInternals;
};
