//----------------------------------------------------------------------------------------------------------------------
//	CVideoProcessor.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CVideoProcessor.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CVideoProcessorInternals

class CVideoProcessorInternals {
	public:
		CVideoProcessorInternals() : mVideoProcessor(nil) {}
		~CVideoProcessorInternals()
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
	mInternals = new CVideoProcessorInternals();
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
void CVideoProcessor::setSourceWindow(UniversalTimeInterval startTimeInterval,
		const OV<UniversalTimeInterval>& durationTimeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check for instance
	if (mInternals->mVideoProcessor != nil)
		// Set source window
		(*mInternals->mVideoProcessor)->setSourceWindow(startTimeInterval, durationTimeInterval);
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
