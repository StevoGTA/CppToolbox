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
UInt32 CMediaDestination::CMediaDestination::getAudioTrackCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mAudioProcessors.getKeyCount();
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CMediaDestination::CMediaDestination::getVideoTrackCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mVideoProcessors.getKeyCount();
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaDestination::setSourceWindow(UniversalTimeInterval startTimeInterval,
		const OV<UniversalTimeInterval>& durationTimeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all audio tracks
	for (UInt32 i = 0; i < getAudioTrackCount(); i++)
		// Set window
		(*mInternals->mAudioProcessors[i])->setSourceWindow(startTimeInterval, durationTimeInterval);

	// Iterate all video tracks
	for (UInt32 i = 0; i < getVideoTrackCount(); i++)
		// Seek
		(*mInternals->mVideoProcessors[i])->setSourceWindow(startTimeInterval, durationTimeInterval);
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaDestination::seek(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all audio tracks
	for (UInt32 i = 0; i < getAudioTrackCount(); i++)
		// Seek
		(*mInternals->mAudioProcessors[i])->seek(timeInterval);

	// Iterate all video tracks
	for (UInt32 i = 0; i < getVideoTrackCount(); i++)
		// Seek
		(*mInternals->mVideoProcessors[i])->seek(timeInterval);
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaDestination::add(const I<CAudioProcessor>& audioProcessor, UInt32 trackIndex)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mAudioProcessors.set(trackIndex, audioProcessor);
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
OR<I<CVideoProcessor> > CMediaDestination::CMediaDestination::getVideoProcessor(UInt32 trackIndex) const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mVideoProcessors[trackIndex];
}
