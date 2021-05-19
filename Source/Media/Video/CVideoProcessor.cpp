//----------------------------------------------------------------------------------------------------------------------
//	CVideoProcessor.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CVideoProcessor.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CVideoProcessorInternals

class CVideoProcessorInternals {
	public:
//		CVideoProcessorInternals() : mVideoProcessor(nil) {}
		CVideoProcessorInternals() {}
//		~CVideoProcessorInternals()
//			{ Delete(mVideoProcessor); }

//		I<CVideoProcessor>*	mVideoProcessor;
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

////----------------------------------------------------------------------------------------------------------------------
//OI<SError> CVideoProcessor::connectInput(const I<CVideoProcessor>& videoProcessor)
////----------------------------------------------------------------------------------------------------------------------
//{
//	// Store
//	mInternals->mVideoProcessor = new I<CVideoProcessor>(videoProcessor);
//
//	return OI<SError>();
//}

////----------------------------------------------------------------------------------------------------------------------
//SVideoReadStatus CVideoProcessor::perform(const SMediaPosition& mediaPosition, CVideoFrame& videoFrame)
////----------------------------------------------------------------------------------------------------------------------
//{
//	return (*mInternals->mVideoProcessor)->perform(mediaPosition, videoFrame);
//}

////----------------------------------------------------------------------------------------------------------------------
//OI<SError> CVideoProcessor::reset()
////----------------------------------------------------------------------------------------------------------------------
//{
//	// Check for instance
//	if (mInternals->mVideoProcessor != nil)
//		// Reset
//		return (*mInternals->mVideoProcessor)->reset();
//	else
//		// No Video Processor
//		return OI<SError>();
//}
