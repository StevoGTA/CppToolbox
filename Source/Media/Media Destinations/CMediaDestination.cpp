//----------------------------------------------------------------------------------------------------------------------
//	CMediaDestination.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CMediaDestination.h"

#include "CDictionary.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaDestination::Internals

class CMediaDestination::Internals {
	public:
		Internals(const CString& name) : mName(name) {}

		CString														mName;
		TNKeyConvertibleDictionary<UInt32, I<CAudioDestination> >	mAudioDestinationByTrack;
		TNKeyConvertibleDictionary<UInt32, I<CVideoDestination> >	mVideoDestinationByTrack;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMediaDestination

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CMediaDestination::CMediaDestination(const CString& name)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(name);
}

//----------------------------------------------------------------------------------------------------------------------
CMediaDestination::~CMediaDestination()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
const CString& CMediaDestination::getName() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mName;
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CMediaDestination::CMediaDestination::getAudioTrackCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mAudioDestinationByTrack.getKeyCount();
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaDestination::add(const I<CAudioDestination>& audioDestination, UInt32 trackIndex)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mAudioDestinationByTrack.set(trackIndex, audioDestination);
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CMediaDestination::CMediaDestination::getVideoTrackCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mVideoDestinationByTrack.getKeyCount();
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaDestination::add(const I<CVideoDestination>& videoDestination, UInt32 trackIndex)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mVideoDestinationByTrack.set(trackIndex, videoDestination);
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaDestination::setSourceWindow(UniversalTimeInterval startTimeInterval,
		const OV<UniversalTimeInterval>& durationTimeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all audio tracks
	for (UInt32 i = 0; i < getAudioTrackCount(); i++)
		// Set window
		(*mInternals->mAudioDestinationByTrack[i])->setSourceWindow(startTimeInterval, durationTimeInterval);

	// Iterate all video tracks
	for (UInt32 i = 0; i < getVideoTrackCount(); i++)
		// Seek
		(*mInternals->mVideoDestinationByTrack[i])->setSourceWindow(startTimeInterval, durationTimeInterval);
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaDestination::seek(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all audio tracks
	for (UInt32 i = 0; i < getAudioTrackCount(); i++)
		// Seek
		(*mInternals->mAudioDestinationByTrack[i])->seek(timeInterval);

	// Iterate all video tracks
	for (UInt32 i = 0; i < getVideoTrackCount(); i++)
		// Seek
		(*mInternals->mVideoDestinationByTrack[i])->seek(timeInterval);
}

//----------------------------------------------------------------------------------------------------------------------
OR<I<CAudioDestination> > CMediaDestination::CMediaDestination::getAudioDestination(UInt32 trackIndex) const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mAudioDestinationByTrack[trackIndex];
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaDestination::removeAllAudioDestinations()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mAudioDestinationByTrack.removeAll();
}

//----------------------------------------------------------------------------------------------------------------------
OR<I<CVideoDestination> > CMediaDestination::CMediaDestination::getVideoDestination(UInt32 trackIndex) const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mVideoDestinationByTrack[trackIndex];
}

//----------------------------------------------------------------------------------------------------------------------
void CMediaDestination::removeAllVideoDestinations()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mVideoDestinationByTrack.removeAll();
}
