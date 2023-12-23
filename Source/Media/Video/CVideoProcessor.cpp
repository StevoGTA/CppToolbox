//----------------------------------------------------------------------------------------------------------------------
//	CVideoProcessor.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CVideoProcessor.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CVideoProcessor::Internals

class CVideoProcessor::Internals {
	public:
		Internals() : mVideoProcessor(nil) {}
		~Internals()
			{ Delete(mVideoProcessor); }

		I<CVideoProcessor>*	mVideoProcessor;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CVideoProcessor

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CVideoProcessor::CVideoProcessor()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals();
}

//----------------------------------------------------------------------------------------------------------------------
CVideoProcessor::~CVideoProcessor()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CVideoProcessor::connectInput(const I<CVideoProcessor>& videoProcessor)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mVideoProcessor = new I<CVideoProcessor>(videoProcessor);

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
TArray<CString> CVideoProcessor::getSetupDescription(const CString& indent)
//----------------------------------------------------------------------------------------------------------------------
{
	return (*mInternals->mVideoProcessor)->getSetupDescription(indent);
}

//----------------------------------------------------------------------------------------------------------------------
void CVideoProcessor::setMediaSegment(const OV<SMedia::Segment>& mediaSegment)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check for instance
	if (mInternals->mVideoProcessor != nil)
		// Set source window
		(*mInternals->mVideoProcessor)->setMediaSegment(mediaSegment);
}

//----------------------------------------------------------------------------------------------------------------------
void CVideoProcessor::seek(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check for instance
	if (mInternals->mVideoProcessor != nil)
		// Seek
		(*mInternals->mVideoProcessor)->seek(timeInterval);
}

//----------------------------------------------------------------------------------------------------------------------
CVideoProcessor::PerformResult CVideoProcessor::perform()
//----------------------------------------------------------------------------------------------------------------------
{
	return (*mInternals->mVideoProcessor)->perform();
}

//----------------------------------------------------------------------------------------------------------------------
void CVideoProcessor::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check for instance
	if (mInternals->mVideoProcessor != nil)
		// Reset
		(*mInternals->mVideoProcessor)->reset();
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CVideoSource::Internals

class CVideoSource::Internals {
	public:
		Internals() : mCurrentTimeInterval(0.0) {}

		OV<SMedia::Segment>		mMediaSegment;
		UniversalTimeInterval	mCurrentTimeInterval;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CVideoSource

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CVideoSource::CVideoSource()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals();
}

//----------------------------------------------------------------------------------------------------------------------
CVideoSource::~CVideoSource()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CVideoProcessor methods

//----------------------------------------------------------------------------------------------------------------------
void CVideoSource::setMediaSegment(const OV<SMedia::Segment>& mediaSegment)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	bool	performSeek = mediaSegment.hasValue() && !mediaSegment->contains(mInternals->mCurrentTimeInterval);

	// Store
	mInternals->mMediaSegment = mediaSegment;

	// Do super
	CVideoProcessor::setMediaSegment(mediaSegment);

	// Check if need seek
	if (performSeek)
		// Seek
		seek(mInternals->mMediaSegment->getStartTimeInterval());
}

//----------------------------------------------------------------------------------------------------------------------
void CVideoSource::seek(UniversalTimeInterval timeInterval)
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
void CVideoSource::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Reset
	mInternals->mCurrentTimeInterval =
			mInternals->mMediaSegment.hasValue() ? mInternals->mMediaSegment->getStartTimeInterval() : 0.0;
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
UniversalTimeInterval CVideoSource::getCurrentTimeInterval() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mCurrentTimeInterval;
}

//----------------------------------------------------------------------------------------------------------------------
void CVideoSource::setCurrentTimeInterval(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mCurrentTimeInterval = timeInterval;
}
