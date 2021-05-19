//----------------------------------------------------------------------------------------------------------------------
//	CMediaDestination.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CMediaDestination.h"

#include "CDictionary.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaDestinationInternals

class CMediaDestinationInternals {
	public:
		CMediaDestinationInternals() {}

		TKeyConvertibleDictionary<UInt32, I<CAudioProcessor> >	mAudioProcessors;
		TKeyConvertibleDictionary<UInt32, I<CVideoProcessor> >	mVideoProcessors;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMediaDestination

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CMediaDestination::CMediaDestination()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CMediaDestinationInternals();
}

//----------------------------------------------------------------------------------------------------------------------
CMediaDestination::~CMediaDestination()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CMediaDestination::add(const I<CAudioProcessor>& audioProcessor, UInt32 trackIndex)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mAudioProcessors.set(trackIndex, audioProcessor);
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CMediaDestination::CMediaDestination::getAudioTrackCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mAudioProcessors.getKeyCount();
}

//----------------------------------------------------------------------------------------------------------------------
OR<I<CAudioProcessor> > CMediaDestination::CMediaDestination::getAudioProcessor(UInt32 trackIndex) const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mAudioProcessors[trackIndex];
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaDestination::add(const I<CVideoProcessor>& videoProcessor, UInt32 trackIndex)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mVideoProcessors.set(trackIndex, videoProcessor);
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CMediaDestination::CMediaDestination::getVideoTrackCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mVideoProcessors.getKeyCount();
}

//----------------------------------------------------------------------------------------------------------------------
OR<I<CVideoProcessor> > CMediaDestination::CMediaDestination::getVideoProcessor(UInt32 trackIndex) const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mVideoProcessors[trackIndex];
}
