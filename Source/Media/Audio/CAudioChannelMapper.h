//----------------------------------------------------------------------------------------------------------------------
//	CAudioChannelMapper.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CAudioProcessor.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioChannelMapper

class CAudioChannelMapperInternals;
class CAudioChannelMapper : public CBasicAudioProcessor {
	// Methods
	public:
												// Lifecycle methods
												CAudioChannelMapper();
												~CAudioChannelMapper();

												// Instance methods
				OI<SError>						connectInput(const I<CAudioProcessor>& audioProcessor,
														const SAudioProcessingFormat& audioProcessingFormat);

				SAudioSourceStatus				performInto(CAudioFrames& audioFrames);

												// Subclass methods
				TArray<SAudioProcessingSetup>	getOutputSetups() const;

												// Class methods
		static	bool							canPerform(EAudioChannelMap fromAudioChannelMap,
														EAudioChannelMap toAudioChannelMap);

	// Properties
	private:
		CAudioChannelMapperInternals*	mInternals;
};