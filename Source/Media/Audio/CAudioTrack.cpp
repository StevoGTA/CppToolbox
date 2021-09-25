//----------------------------------------------------------------------------------------------------------------------
//	CAudioTrack.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAudioTrack.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioTrackInternals

class CAudioTrackInternals : public TReferenceCountable<CAudioTrackInternals> {
	public:
		CAudioTrackInternals(UInt32 index, const SAudioStorageFormat& audioStorageFormat,
				const I<CCodec::DecodeInfo>& decodeInfo) :
			TReferenceCountable(), mIndex(index), mAudioStorageFormat(audioStorageFormat), mDecodeInfo(decodeInfo)
			{}

		UInt32					mIndex;
		SAudioStorageFormat		mAudioStorageFormat;
		I<CCodec::DecodeInfo>	mDecodeInfo;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioTrack

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAudioTrack::CAudioTrack(const Info& info, const SAudioStorageFormat& audioStorageFormat,
		const I<CCodec::DecodeInfo>& decodeInfo) : CMediaTrack(info)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new CAudioTrackInternals(0, audioStorageFormat, decodeInfo);
}

//----------------------------------------------------------------------------------------------------------------------
CAudioTrack::CAudioTrack(UInt32 index, const Info& info, const SAudioStorageFormat& audioStorageFormat,
		const I<CCodec::DecodeInfo>& decodeInfo) : CMediaTrack(info)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new CAudioTrackInternals(index, audioStorageFormat, decodeInfo);
}

//----------------------------------------------------------------------------------------------------------------------
CAudioTrack::CAudioTrack(const CAudioTrack& other) : CMediaTrack(other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CAudioTrack::~CAudioTrack()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
const SAudioStorageFormat& CAudioTrack::getAudioStorageFormat() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mAudioStorageFormat;
}

//----------------------------------------------------------------------------------------------------------------------
const I<CCodec::DecodeInfo>& CAudioTrack::getDecodeInfo() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mDecodeInfo;
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CMediaTrack::Info CAudioTrack::composeInfo(const SAudioStorageFormat& audioStorageFormat, UInt64 frameCount,
		UInt64 byteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UniversalTimeInterval	duration =
									(UniversalTimeInterval) frameCount /
											(UniversalTimeInterval) audioStorageFormat.getSampleRate();

	return Info(duration, (UInt32) (((UniversalTimeInterval) byteCount * 8) / duration));
}

//----------------------------------------------------------------------------------------------------------------------
CString CAudioTrack::getStringFromDB(Float32 db, Float32 muteDB)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value
	if (db == muteDB)
		// Silence
		return CString(OSSTR("-\u221EdB"));
	else if (db < 1.0)
		// Negative
		return CString(db, 0, 1) + CString(OSSTR("dB"));
	else
		// Positive
		return CString(OSSTR("+")) + CString(db, 0, 1) + CString(OSSTR("dB"));
}
