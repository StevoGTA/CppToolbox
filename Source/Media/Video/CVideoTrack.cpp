//----------------------------------------------------------------------------------------------------------------------
//	CVideoTrack.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CVideoTrack.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CVideoTrackInternals

class CVideoTrackInternals : public TReferenceCountable<CVideoTrackInternals> {
	public:
		CVideoTrackInternals(UInt32 index, const SVideoStorageFormat& videoStorageFormat) :
			TReferenceCountable(), mIndex(index), mVideoStorageFormat(videoStorageFormat)
			{}

		UInt32				mIndex;
		SVideoStorageFormat	mVideoStorageFormat;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CVideoTrack

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CVideoTrack::CVideoTrack(const Info& info, const SVideoStorageFormat& videoStorageFormat) : CMediaTrack(info)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new CVideoTrackInternals(0, videoStorageFormat);
}

//----------------------------------------------------------------------------------------------------------------------
CVideoTrack::CVideoTrack(UInt32 index, const Info& info, const SVideoStorageFormat& videoStorageFormat) :
		CMediaTrack(info)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new CVideoTrackInternals(index, videoStorageFormat);
}

//----------------------------------------------------------------------------------------------------------------------
CVideoTrack::CVideoTrack(const CVideoTrack& other) : CMediaTrack(other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CVideoTrack::~CVideoTrack()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
const SVideoStorageFormat& CVideoTrack::getVideoStorageFormat() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mVideoStorageFormat;
}
