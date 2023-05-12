//----------------------------------------------------------------------------------------------------------------------
//	CAudioTrack.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAudioTrack.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAudioTrack::Internals

class CAudioTrack::Internals : public TReferenceCountable<Internals> {
	public:
		Internals(UInt32 index, const SAudioStorageFormat& audioStorageFormat) :
			TReferenceCountable(), mIndex(index), mAudioStorageFormat(audioStorageFormat)
			{}

		UInt32				mIndex;
		SAudioStorageFormat	mAudioStorageFormat;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAudioTrack

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAudioTrack::CAudioTrack(const Info& info, const SAudioStorageFormat& audioStorageFormat) : CMediaTrack(info)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new Internals(0, audioStorageFormat);
}

//----------------------------------------------------------------------------------------------------------------------
CAudioTrack::CAudioTrack(UInt32 index, const Info& info, const SAudioStorageFormat& audioStorageFormat) :
		CMediaTrack(info)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new Internals(index, audioStorageFormat);
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

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CMediaTrack::Info CAudioTrack::composeInfo(UniversalTimeInterval duration,
		const SAudioStorageFormat& audioStorageFormat, UInt32 bytesPerFrame)
//----------------------------------------------------------------------------------------------------------------------
{
	return Info(duration, (UInt32) (audioStorageFormat.getSampleRate() * (Float32) bytesPerFrame * 8.0f));
}

//----------------------------------------------------------------------------------------------------------------------
CMediaTrack::Info CAudioTrack::composeInfo(UniversalTimeInterval duration,
		const SAudioStorageFormat& audioStorageFormat, UInt32 framesPerPacket, UInt32 bytesPerPacket)
//----------------------------------------------------------------------------------------------------------------------
{
	return Info(duration,
			(UInt32)
					(audioStorageFormat.getSampleRate() / (Float32) framesPerPacket * (Float32) bytesPerPacket * 8.0f));
}

//----------------------------------------------------------------------------------------------------------------------
CMediaTrack::Info CAudioTrack::composeInfo(const SAudioStorageFormat& audioStorageFormat, UInt64 frameCount,
		UInt64 byteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	return CMediaTrack::composeInfo(
			(UniversalTimeInterval) frameCount / (UniversalTimeInterval) audioStorageFormat.getSampleRate(), byteCount);
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
