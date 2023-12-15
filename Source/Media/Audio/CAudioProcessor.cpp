//----------------------------------------------------------------------------------------------------------------------
//	CAudioProcessor.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAudioProcessor.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioProcessor::Internals

class CAudioProcessor::Internals {
	public:
		OV<I<CAudioProcessor> >	mAudioProcessor;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioProcessor

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAudioProcessor::CAudioProcessor()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals();
}

//----------------------------------------------------------------------------------------------------------------------
CAudioProcessor::~CAudioProcessor()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CAudioProcessor::connectInput(const I<CAudioProcessor>& audioProcessor,
		const SAudio::ProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mAudioProcessor.setValue(I<CAudioProcessor>(audioProcessor));

	// Note
	setInputFormat(audioProcessingFormat);

	return audioProcessor->setOutputFormat(audioProcessingFormat);
}

//----------------------------------------------------------------------------------------------------------------------
TArray<CString> CAudioProcessor::getSetupDescription(const CString& indent)
//----------------------------------------------------------------------------------------------------------------------
{
	return (*mInternals->mAudioProcessor)->getSetupDescription(indent);
}

//----------------------------------------------------------------------------------------------------------------------
CAudioFrames::Requirements CAudioProcessor::queryRequirements() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	AssertFailIf(!mInternals->mAudioProcessor.hasValue())

	return (*mInternals->mAudioProcessor)->queryRequirements();
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioProcessor::setSourceWindow(UniversalTimeInterval startTimeInterval,
		const OV<UniversalTimeInterval>& durationTimeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check for instance
	if (mInternals->mAudioProcessor.hasValue())
		// Set source window
		(*mInternals->mAudioProcessor)->setSourceWindow(startTimeInterval, durationTimeInterval);
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioProcessor::seek(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check for instance
	if (mInternals->mAudioProcessor.hasValue())
		// Seek
		(*mInternals->mAudioProcessor)->seek(timeInterval);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<SMedia::SourceInfo> CAudioProcessor::performInto(CAudioFrames& audioFrames)
//----------------------------------------------------------------------------------------------------------------------
{
	return (*mInternals->mAudioProcessor)->performInto(audioFrames);
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioProcessor::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check for instance
	if (mInternals->mAudioProcessor.hasValue())
		// Reset
		(*mInternals->mAudioProcessor)->reset();
}
