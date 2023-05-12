//----------------------------------------------------------------------------------------------------------------------
//	SMediaPacket.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "SMediaPacket.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaPacketSource

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
UInt32 CMediaPacketSource::seekToKeyframe(UInt32 initialFrameIndex, const TNumberArray<UInt32>& keyframeIndexes)
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate through keyframes to find target one
	UInt32	packetIndex = 0;
	for (TIteratorM<TNumber<UInt32>, UInt32> iterator = keyframeIndexes.getIterator(); iterator.hasValue();
			iterator.advance()) {
		// Check this keyframe
		if (*iterator <= initialFrameIndex)
			// Is the same or before initial frame index
			packetIndex = *iterator;
		else
			// Is after initial frame index, done.
			break;
	}

	// Seek to packet
	seekToPacket(packetIndex);

	return packetIndex;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSeekableUniformMediaPacketSource::Internals

class CSeekableUniformMediaPacketSource::Internals {
	public:
		Internals(const I<CRandomAccessDataSource>& randomAccessDataSource, UInt64 byteOffset, UInt64 byteCount,
				UInt32 bytesPerPacket, UInt32 durationPerPacket) :
			mRandomAccessDataSource(randomAccessDataSource), mByteOffset(byteOffset),
					mPacketCount((UInt32) (byteCount / (UInt64) bytesPerPacket)), mBytesPerPacket(bytesPerPacket),
					mDurationPerPacket(durationPerPacket), mNextPacketIndex(0)
			{}

		I<CRandomAccessDataSource>	mRandomAccessDataSource;
		UInt64						mByteOffset;
		UInt32						mPacketCount;
		UInt32						mBytesPerPacket;
		UInt32						mDurationPerPacket;
		UInt32						mNextPacketIndex;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSeekableUniformMediaPacketSource

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CSeekableUniformMediaPacketSource::CSeekableUniformMediaPacketSource(
		const I<CRandomAccessDataSource>& randomAccessDataSource, UInt64 byteOffset, UInt64 byteCount,
		UInt32 bytesPerPacket, UInt32 durationPerPacket)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(randomAccessDataSource, byteOffset, byteCount, bytesPerPacket, durationPerPacket);
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
TVResult<CMediaPacketSource::DataInfo> CSeekableUniformMediaPacketSource::readNext()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if can read next packet
	if (mInternals->mNextPacketIndex < mInternals->mPacketCount) {
		// Copy packet data
		CData		data((CData::ByteCount) mInternals->mBytesPerPacket);
		UInt64		byteOffset = mInternals->mByteOffset + mInternals->mNextPacketIndex * mInternals->mBytesPerPacket;
		OV<SError>	error =
							mInternals->mRandomAccessDataSource->readData(byteOffset, data.getMutableBytePtr(),
									mInternals->mBytesPerPacket);
		ReturnValueIfError(error, TVResult<CMediaPacketSource::DataInfo>(*error));

		// Update
		mInternals->mNextPacketIndex++;

		return TVResult<CMediaPacketSource::DataInfo>(
				CMediaPacketSource::DataInfo(data, mInternals->mDurationPerPacket));
	} else
		// End of data
		return TVResult<CMediaPacketSource::DataInfo>(SError::mEndOfData);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<TArray<SMediaPacket> > CSeekableUniformMediaPacketSource::readNextInto(CData& data,
		const OV<UInt32>& maxPacketCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Add packets
	UInt32					dataByteCountRemaining = (UInt32) data.getByteCount();
	UInt8*					packetDataPtr = (UInt8*) data.getMutableBytePtr();
	UInt32					packetCount = maxPacketCount.hasValue() ? *maxPacketCount : ~0;
	TNArray<SMediaPacket>	mediaPackets;
	while ((packetCount > 0) && (mInternals->mNextPacketIndex < mInternals->mPacketCount)) {
		// Check if have space
		if (mInternals->mBytesPerPacket <= dataByteCountRemaining) {
			// Copy packet data
			UInt64		byteOffset =
								mInternals->mByteOffset + mInternals->mNextPacketIndex * mInternals->mBytesPerPacket;
			OV<SError>	error =
								mInternals->mRandomAccessDataSource->readData(byteOffset, packetDataPtr,
											mInternals->mBytesPerPacket);
			ReturnValueIfError(error, TVResult<TArray<SMediaPacket> >(*error));

			// Update
			mInternals->mNextPacketIndex++;

			// Update info
			dataByteCountRemaining -= mInternals->mBytesPerPacket;
			packetDataPtr += mInternals->mBytesPerPacket;
			packetCount--;
			mediaPackets += SMediaPacket(mInternals->mDurationPerPacket, mInternals->mBytesPerPacket);
		} else
			// No more space
			break;
	}

	return !mediaPackets.isEmpty() ?
			TVResult<TArray<SMediaPacket> >(mediaPackets) : TVResult<TArray<SMediaPacket> >(SError::mEndOfData);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSeekableVaryingMediaPacketSource::Internals

class CSeekableVaryingMediaPacketSource::Internals {
	public:
		Internals(const I<CRandomAccessDataSource>& randomAccessDataSource,
				const TArray<SMediaPacketAndLocation>& mediaPacketAndLocations) :
			mRandomAccessDataSource(randomAccessDataSource), mMediaPacketAndLocations(mediaPacketAndLocations),
					mNextPacketIndex(0)
			{}

		I<CRandomAccessDataSource>		mRandomAccessDataSource;
		TArray<SMediaPacketAndLocation>	mMediaPacketAndLocations;
		UInt32							mNextPacketIndex;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSeekableVaryingMediaPacketSource

// Lifecycle methods
//----------------------------------------------------------------------------------------------------------------------
CSeekableVaryingMediaPacketSource::CSeekableVaryingMediaPacketSource(
		const I<CRandomAccessDataSource>& randomAccessDataSource,
		const TArray<SMediaPacketAndLocation>& mediaPacketAndLocations)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(randomAccessDataSource, mediaPacketAndLocations);
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
TVResult<CMediaPacketSource::DataInfo> CSeekableVaryingMediaPacketSource::readNext()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if can read next packet
	if (mInternals->mNextPacketIndex < mInternals->mMediaPacketAndLocations.getCount()) {
		// Setup
		SMediaPacketAndLocation&	mediaPacketAndLocation =
											mInternals->mMediaPacketAndLocations.getAt(mInternals->mNextPacketIndex);

		// Copy packet data
		CData		data((CData::ByteCount) mediaPacketAndLocation.mMediaPacket.mByteCount);
		OV<SError>	error =
							mInternals->mRandomAccessDataSource->readData(mediaPacketAndLocation.mByteOffset,
									data.getMutableBytePtr(), mediaPacketAndLocation.mMediaPacket.mByteCount);
		ReturnValueIfError(error, TVResult<CMediaPacketSource::DataInfo>(*error));

		// Update
		mInternals->mNextPacketIndex++;

		return TVResult<CMediaPacketSource::DataInfo>(
				CMediaPacketSource::DataInfo(data, mediaPacketAndLocation.mMediaPacket.mDuration));
	} else
		// End of data
		return TVResult<CMediaPacketSource::DataInfo>(SError::mEndOfData);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<TArray<SMediaPacket> > CSeekableVaryingMediaPacketSource::readNextInto(CData& data,
		const OV<UInt32>& maxPacketCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Add packets
	UInt32					dataByteCountRemaining = (UInt32) data.getByteCount();
	UInt8*					packetDataPtr = (UInt8*) data.getMutableBytePtr();
	UInt32					packetCount = maxPacketCount.hasValue() ? *maxPacketCount : ~0;
	TNArray<SMediaPacket>	mediaPackets;
	while ((packetCount > 0) && (mInternals->mNextPacketIndex < mInternals->mMediaPacketAndLocations.getCount())) {
		// Setup
		SMediaPacketAndLocation&	mediaPacketAndLocation =
											mInternals->mMediaPacketAndLocations.getAt(mInternals->mNextPacketIndex);

		// Check if have space
		if (mediaPacketAndLocation.mMediaPacket.mByteCount <= dataByteCountRemaining) {
			// Copy packet data
			OV<SError>	error =
								mInternals->mRandomAccessDataSource->readData(mediaPacketAndLocation.mByteOffset,
										packetDataPtr, mediaPacketAndLocation.mMediaPacket.mByteCount);
			ReturnValueIfError(error, TVResult<TArray<SMediaPacket> >(*error));

			// Update
			mInternals->mNextPacketIndex++;

			// Update info
			dataByteCountRemaining -= mediaPacketAndLocation.mMediaPacket.mByteCount;
			packetDataPtr += mediaPacketAndLocation.mMediaPacket.mByteCount;
			packetCount--;
			mediaPackets += mediaPacketAndLocation.mMediaPacket;
		} else
			// No more space
			break;
	}

	return !mediaPackets.isEmpty() ?
			TVResult<TArray<SMediaPacket> >(mediaPackets) : TVResult<TArray<SMediaPacket> >(SError::mEndOfData);
}
