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
void CAudioProcessor::setMediaSegment(const OV<SMedia::Segment>& mediaSegment)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check for instance
	if (mInternals->mAudioProcessor.hasValue())
		// Set source window
		(*mInternals->mAudioProcessor)->setMediaSegment(mediaSegment);
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
TVResult<CAudioProcessor::SourceInfo> CAudioProcessor::performInto(CAudioFrames& audioFrames)
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

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioSource::Internals

class CAudioSource::Internals {
	public:
		Internals() : mCurrentTimeInterval(0.0) {}

		OV<SMedia::Segment>		mMediaSegment;
		UniversalTimeInterval	mCurrentTimeInterval;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioSource

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAudioSource::CAudioSource()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals();
}

//----------------------------------------------------------------------------------------------------------------------
CAudioSource::~CAudioSource()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CAudioProcessor methods

//----------------------------------------------------------------------------------------------------------------------
void CAudioSource::setMediaSegment(const OV<SMedia::Segment>& mediaSegment)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	bool	performSeek = mediaSegment.hasValue() && !mediaSegment->contains(mInternals->mCurrentTimeInterval);

	// Store
	mInternals->mMediaSegment = mediaSegment;

	// Do super
	CAudioProcessor::setMediaSegment(mediaSegment);

	// Check if need seek
	if (performSeek)
		// Seek
		seek(mInternals->mMediaSegment->getStartTimeInterval());
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioSource::seek(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Bound the given time
	if (mInternals->mMediaSegment.hasValue()) {
		// Limit to within start and end
		timeInterval = std::max<UniversalTimeInterval>(timeInterval, mInternals->mMediaSegment->getStartTimeInterval());
		timeInterval = std::min<UniversalTimeInterval>(timeInterval, mInternals->mMediaSegment->getEndTimeInterval());
	}

	// Update
	mInternals->mCurrentTimeInterval = timeInterval;
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioSource::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Reset
	mInternals->mCurrentTimeInterval =
			mInternals->mMediaSegment.hasValue() ? mInternals->mMediaSegment->getStartTimeInterval() : 0.0;
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
UniversalTimeInterval CAudioSource::getCurrentTimeInterval() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mCurrentTimeInterval;
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioSource::setCurrentTimeInterval(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mCurrentTimeInterval = timeInterval;
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<UInt32> CAudioSource::calculateMaxFrames(Float32 sampleRate) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	if (mInternals->mMediaSegment.hasValue()) {
		// Have duration
		UniversalTimeInterval	durationRemaining =
										mInternals->mMediaSegment->getEndTimeInterval() -
												mInternals->mCurrentTimeInterval;
		if (durationRemaining <= 0.0)
			// Already done
			return TVResult<UInt32>(SError::mEndOfData);

		return TVResult<UInt32>((UInt32) (durationRemaining * sampleRate));
	} else
		// No duration
		return TVResult<UInt32>((UInt32) ~0);
}
