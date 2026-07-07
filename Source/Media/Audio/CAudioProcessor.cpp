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

	// Check if need to perform a seek to stay in the specified media segment
	if (performSeek)
		// Seek
		seek(mInternals->mMediaSegment->getStartTimeInterval());
}

//----------------------------------------------------------------------------------------------------------------------
void CAudioSource::seek(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if have a media segment
	if (mInternals->mMediaSegment.hasValue()) {
		// Bound to the media segment
		timeInterval = std::max<UniversalTimeInterval>(timeInterval, mInternals->mMediaSegment->getStartTimeInterval());
		timeInterval = std::min<UniversalTimeInterval>(timeInterval, mInternals->mMediaSegment->getEndTimeInterval());
	}

	// Update
	mInternals->mCurrentTimeInterval = timeInterval;
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
		// We track position and segment bounds as Float64 time (UniversalTimeInterval) by design - converting to/from
		//	whole frames is expected to introduce only sub-sample error.  That assumption ONLY holds if every
		//	time->frame conversion ROUNDS to the nearest frame.  Truncating (a plain (UInt32) cast of
		//	durationRemaining * sampleRate) is NOT a tiny error - it is a systematic floor bias of up to one full
		//	frame, and it caused two subtle, audible defects when exporting a specified portion:
		//
		//		1. The segment came out one frame short (e.g. a 10.0s @ 44100Hz portion produced 440999 frames instead
		//			of the expected 441000) because the final fractional frame of the remaining duration was floored
		//			away.
		//		2. A full-magnitude frame leaked out at the very end.  Downstream processors (e.g. a fade-out whose
		//			window ends exactly at the segment end) intentionally pass through, UNCHANGED, any frame that falls
		//			outside their window - handling samples after a fade is the pipeline builder's responsibility (so
		//			that multiple fades can be composed).  The fade decides "inside the window" with a clean Float64
		//			comparison, while this routine decided the source's final frame with a floor.  Those two boundaries
		//			disagreed by a frame at the seam, so a boundary frame could be emitted by the source yet land just
		//			past the fade-out window and pass straight through at full level.
		//
		//	Rounding BOTH the segment end and the current position to the nearest frame and subtracting yields a single,
		//	exact integer frame boundary that the whole pipeline agrees on: the frame count is exactly correct and the
		//	last delivered frame is guaranteed to fall strictly inside (never at/after) the segment end.  The current
		//	position is itself a Float64 accumulation of frameCount / sampleRate, but its drift stays far below half a
		//	frame, so rounding recovers the exact cumulative frame index.
		SInt64	endFrame = (SInt64) (mInternals->mMediaSegment->getEndTimeInterval() * sampleRate + 0.5);
		SInt64	currentFrame = (SInt64) (mInternals->mCurrentTimeInterval * sampleRate + 0.5);

		return (currentFrame >= endFrame) ?
				TVResult<UInt32>(SError::mEndOfData) : TVResult<UInt32>((UInt32) (endFrame - currentFrame));
	} else
		// No segment - no limit
		return TVResult<UInt32>((UInt32) ~0);
}
