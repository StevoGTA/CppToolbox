//----------------------------------------------------------------------------------------------------------------------
//	CAudioProcessor.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAudioProcessor.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioProcessorInternals

class CAudioProcessorInternals {
	public:
		CAudioProcessorInternals() : mAudioProcessor(nil) {}
		~CAudioProcessorInternals()
			{ Delete(mAudioProcessor); }

		I<CAudioProcessor>*	mAudioProcessor;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioProcessor

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAudioProcessor::CAudioProcessor()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CAudioProcessorInternals();
}

//----------------------------------------------------------------------------------------------------------------------
CAudioProcessor::~CAudioProcessor()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CAudioProcessor::connectInput(const I<CAudioProcessor>& audioProcessor,
		const SAudioProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mAudioProcessor = new I<CAudioProcessor>(audioProcessor);

	// Setup
	audioProcessor->setOutputFormat(audioProcessingFormat);

	return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
SAudioReadStatus CAudioProcessor::perform(const SMediaPosition& mediaPosition, CAudioFrames& audioFrames)
//----------------------------------------------------------------------------------------------------------------------
{
	return (*mInternals->mAudioProcessor)->perform(mediaPosition, audioFrames);
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CAudioProcessor::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check for instance
	if (mInternals->mAudioProcessor != nil)
		// Reset
		return (*mInternals->mAudioProcessor)->reset();
	else
		// No Audio Processor
		return OI<SError>();
}
