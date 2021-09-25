//----------------------------------------------------------------------------------------------------------------------
//	CVideoTrack.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CVideoTrack.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CVideoTrackInternals

class CVideoTrackInternals : public TReferenceCountable<CVideoTrackInternals> {
	public:
		CVideoTrackInternals(UInt32 index, const SVideoStorageFormat& videoStorageFormat,
				const I<CCodec::DecodeInfo>& decodeInfo) :
			TReferenceCountable(), mIndex(index), mVideoStorageFormat(videoStorageFormat), mDecodeInfo(decodeInfo)
			{}

		UInt32					mIndex;
		SVideoStorageFormat		mVideoStorageFormat;
		I<CCodec::DecodeInfo>	mDecodeInfo;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CVideoTrack

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CVideoTrack::CVideoTrack(const Info& info, const SVideoStorageFormat& videoStorageFormat,
		const I<CCodec::DecodeInfo>& decodeInfo) : CMediaTrack(info)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new CVideoTrackInternals(0, videoStorageFormat, decodeInfo);
}

//----------------------------------------------------------------------------------------------------------------------
CVideoTrack::CVideoTrack(UInt32 index, const Info& info, const SVideoStorageFormat& videoStorageFormat,
		const I<CCodec::DecodeInfo>& decodeInfo) : CMediaTrack(info)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new CVideoTrackInternals(index, videoStorageFormat, decodeInfo);
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

//----------------------------------------------------------------------------------------------------------------------
const I<CCodec::DecodeInfo>& CVideoTrack::getDecodeInfo() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mDecodeInfo;
}
