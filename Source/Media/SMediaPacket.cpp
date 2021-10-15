//----------------------------------------------------------------------------------------------------------------------
//	SMediaPacket.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "SMediaPacket.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSeekableUniformMediaPacketSourceInternals

class CSeekableUniformMediaPacketSourceInternals {
	public:
		CSeekableUniformMediaPacketSourceInternals(const I<CSeekableDataSource>& seekableDataSource,
				UInt64 byteOffset, UInt64 byteCount, UInt32 bytesPerPacket, UInt32 durationPerPacket) :
			mSeekableDataSource(seekableDataSource), mByteOffset(byteOffset),
					mPacketCount((UInt32) (byteCount / (UInt64) bytesPerPacket)), mBytesPerPacket(bytesPerPacket),
					mDurationPerPacket(durationPerPacket), mNextPacketIndex(0)
			{}

		I<CSeekableDataSource>	mSeekableDataSource;
		UInt64					mByteOffset;
		UInt32					mPacketCount;
		UInt32					mBytesPerPacket;
		UInt32					mDurationPerPacket;
		UInt32					mNextPacketIndex;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSeekableUniformMediaPacketSource

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CSeekableUniformMediaPacketSource::CSeekableUniformMediaPacketSource(const I<CSeekableDataSource>& seekableDataSource,
		UInt64 byteOffset, UInt64 byteCount, UInt32 bytesPerPacket, UInt32 durationPerPacket)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals =
			new CSeekableUniformMediaPacketSourceInternals(seekableDataSource, byteOffset, byteCount, bytesPerPacket,
					durationPerPacket);
}

//----------------------------------------------------------------------------------------------------------------------
CSeekableUniformMediaPacketSource::~CSeekableUniformMediaPacketSource()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CMediaPacketSource methods

//----------------------------------------------------------------------------------------------------------------------
UInt32 CSeekableUniformMediaPacketSource::seekToDuration(UInt32 duration)
//----------------------------------------------------------------------------------------------------------------------
{
	// Update
	mInternals->mNextPacketIndex = duration / mInternals->mDurationPerPacket;

	return duration % mInternals->mDurationPerPacket;
}

//----------------------------------------------------------------------------------------------------------------------
TIResult<CMediaPacketSource::DataInfo> CSeekableUniformMediaPacketSource::getNextPacket()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if can read next packet
	if (mInternals->mNextPacketIndex < mInternals->mPacketCount) {
		// Copy packet data
		CData		data((CData::Size) mInternals->mBytesPerPacket);
		UInt64		byteOffset = mInternals->mByteOffset + mInternals->mNextPacketIndex * mInternals->mBytesPerPacket;
		OI<SError>	error =
							mInternals->mSeekableDataSource->readData(byteOffset, data.getMutableBytePtr(),
									mInternals->mBytesPerPacket);
		ReturnValueIfError(error, TIResult<CMediaPacketSource::DataInfo>(*error));

		// Update
		mInternals->mNextPacketIndex++;

		return TIResult<CMediaPacketSource::DataInfo>(
				CMediaPacketSource::DataInfo(data, mInternals->mDurationPerPacket));
	} else
		// End of data
		return TIResult<CMediaPacketSource::DataInfo>(SError::mEndOfData);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSeekableVaryingMediaPacketSourceInternals

class CSeekableVaryingMediaPacketSourceInternals {
	public:
		CSeekableVaryingMediaPacketSourceInternals(const I<CSeekableDataSource>& seekableDataSource,
				const TArray<SMediaPacketAndLocation>& mediaPacketAndLocations) :
			mSeekableDataSource(seekableDataSource), mMediaPacketAndLocations(mediaPacketAndLocations),
					mNextPacketIndex(0)
			{}

		I<CSeekableDataSource>			mSeekableDataSource;
		TArray<SMediaPacketAndLocation>	mMediaPacketAndLocations;
		UInt32							mNextPacketIndex;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSeekableVaryingMediaPacketSource

// Lifecycle methods
//----------------------------------------------------------------------------------------------------------------------
CSeekableVaryingMediaPacketSource::CSeekableVaryingMediaPacketSource(const I<CSeekableDataSource>& seekableDataSource,
		const TArray<SMediaPacketAndLocation>& mediaPacketAndLocations)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CSeekableVaryingMediaPacketSourceInternals(seekableDataSource, mediaPacketAndLocations);
}

//----------------------------------------------------------------------------------------------------------------------
CSeekableVaryingMediaPacketSource::~CSeekableVaryingMediaPacketSource()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CMediaPacketSource methods

//----------------------------------------------------------------------------------------------------------------------
UInt32 CSeekableVaryingMediaPacketSource::seekToDuration(UInt32 duration)
//----------------------------------------------------------------------------------------------------------------------
{
	// Find packet index
	mInternals->mNextPacketIndex = 0;
	for (TIteratorD<SMediaPacketAndLocation> iterator = mInternals->mMediaPacketAndLocations.getIterator();
			iterator.hasValue(); iterator.advance()) {
		// Check if can advance past this packet
		if (duration >= iterator->mMediaPacket.mDuration) {
			// Advance
			mInternals->mNextPacketIndex++;
			duration -= iterator->mMediaPacket.mDuration;
		} else
			// No
			return duration;
	}

	return 0;
}

//----------------------------------------------------------------------------------------------------------------------
void CSeekableVaryingMediaPacketSource::seekToPacket(UInt32 packetIndex)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mNextPacketIndex = packetIndex;
}

//----------------------------------------------------------------------------------------------------------------------
TIResult<CMediaPacketSource::DataInfo> CSeekableVaryingMediaPacketSource::getNextPacket()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if can read next packet
	if (mInternals->mNextPacketIndex < mInternals->mMediaPacketAndLocations.getCount()) {
		// Setup
		SMediaPacketAndLocation&	mediaPacketAndLocation =
											mInternals->mMediaPacketAndLocations.getAt(mInternals->mNextPacketIndex);

		// Copy packet data
		CData		data((CData::Size) mediaPacketAndLocation.mMediaPacket.mByteCount);
		OI<SError>	error =
							mInternals->mSeekableDataSource->readData(mediaPacketAndLocation.mByteOffset,
									data.getMutableBytePtr(), mediaPacketAndLocation.mMediaPacket.mByteCount);
		ReturnValueIfError(error, TIResult<CMediaPacketSource::DataInfo>(*error));

		// Update
		mInternals->mNextPacketIndex++;

		return TIResult<CMediaPacketSource::DataInfo>(
				CMediaPacketSource::DataInfo(data, mediaPacketAndLocation.mMediaPacket.mDuration));
	} else
		// End of data
		return TIResult<CMediaPacketSource::DataInfo>(SError::mEndOfData);
}

//----------------------------------------------------------------------------------------------------------------------
TIResult<TArray<SMediaPacket> > CSeekableVaryingMediaPacketSource::getMediaPackets(CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	// Add packets
	UInt32					available = (UInt32) data.getSize();
	UInt8*					packetDataPtr = (UInt8*) data.getMutableBytePtr();
	TNArray<SMediaPacket>	mediaPackets;
	while (mInternals->mNextPacketIndex < mInternals->mMediaPacketAndLocations.getCount()) {
		// Setup
		SMediaPacketAndLocation&	mediaPacketAndLocation =
											mInternals->mMediaPacketAndLocations.getAt(mInternals->mNextPacketIndex);

		// Check if have space
		if (mediaPacketAndLocation.mMediaPacket.mByteCount <= available) {
			// Copy packet data
			OI<SError>	error =
								mInternals->mSeekableDataSource->readData(mediaPacketAndLocation.mByteOffset,
										packetDataPtr, mediaPacketAndLocation.mMediaPacket.mByteCount);
			ReturnValueIfError(error, TIResult<TArray<SMediaPacket> >(*error));

			// Update
			mInternals->mNextPacketIndex++;
		} else
			// No more space
			break;

		// Update info
		mediaPackets += mediaPacketAndLocation.mMediaPacket;

		available -= mediaPacketAndLocation.mMediaPacket.mByteCount;
		packetDataPtr += mediaPacketAndLocation.mMediaPacket.mByteCount;
	}

	return !mediaPackets.isEmpty() ?
			TIResult<TArray<SMediaPacket> >(mediaPackets) : TIResult<TArray<SMediaPacket> >(SError::mEndOfData);
}
