//----------------------------------------------------------------------------------------------------------------------
//	CMediaPacketSource.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CMediaPacketSource.h"

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
TVResult<TArray<SMedia::Packet> > CSeekableUniformMediaPacketSource::readNextInto(CData& data,
		const OV<UInt32>& maxPacketCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Add packets
	UInt32					dataByteCountRemaining = (UInt32) data.getByteCount();
	UInt8*					packetDataPtr = (UInt8*) data.getMutableBytePtr();
	UInt32					packetCount = maxPacketCount.hasValue() ? *maxPacketCount : ~0;
	TNArray<SMedia::Packet>	mediaPackets;
	while ((packetCount > 0) && (mInternals->mNextPacketIndex < mInternals->mPacketCount)) {
		// Check if have space
		if (mInternals->mBytesPerPacket <= dataByteCountRemaining) {
			// Copy packet data
			UInt64		byteOffset =
								mInternals->mByteOffset + mInternals->mNextPacketIndex * mInternals->mBytesPerPacket;
			OV<SError>	error =
								mInternals->mRandomAccessDataSource->readData(byteOffset, packetDataPtr,
											mInternals->mBytesPerPacket);
			ReturnValueIfError(error, TVResult<TArray<SMedia::Packet> >(*error));

			// Update
			mInternals->mNextPacketIndex++;

			// Update info
			dataByteCountRemaining -= mInternals->mBytesPerPacket;
			packetDataPtr += mInternals->mBytesPerPacket;
			packetCount--;
			mediaPackets += SMedia::Packet(mInternals->mDurationPerPacket, mInternals->mBytesPerPacket);
		} else
			// No more space
			break;
	}

	return !mediaPackets.isEmpty() ?
			TVResult<TArray<SMedia::Packet> >(mediaPackets) : TVResult<TArray<SMedia::Packet> >(SError::mEndOfData);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSeekableVaryingMediaPacketSource::Internals

class CSeekableVaryingMediaPacketSource::Internals {
	public:
		Internals(const I<CRandomAccessDataSource>& randomAccessDataSource,
				const TArray<SMedia::PacketAndLocation>& mediaPacketAndLocations) :
			mRandomAccessDataSource(randomAccessDataSource), mMediaPacketAndLocations(mediaPacketAndLocations),
					mNextPacketIndex(0)
			{}

		I<CRandomAccessDataSource>		mRandomAccessDataSource;
		TArray<SMedia::PacketAndLocation>	mMediaPacketAndLocations;
		UInt32							mNextPacketIndex;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSeekableVaryingMediaPacketSource

// Lifecycle methods
//----------------------------------------------------------------------------------------------------------------------
CSeekableVaryingMediaPacketSource::CSeekableVaryingMediaPacketSource(
		const I<CRandomAccessDataSource>& randomAccessDataSource,
		const TArray<SMedia::PacketAndLocation>& mediaPacketAndLocations)
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
	for (TIteratorD<SMedia::PacketAndLocation> iterator = mInternals->mMediaPacketAndLocations.getIterator();
			iterator.hasValue(); iterator.advance()) {
		// Check if can advance past this packet
		if (duration >= iterator->getPacket().getDuration()) {
			// Advance
			mInternals->mNextPacketIndex++;
			duration -= iterator->getPacket().getDuration();
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
		SMedia::PacketAndLocation&	mediaPacketAndLocation =
											mInternals->mMediaPacketAndLocations.getAt(mInternals->mNextPacketIndex);

		// Copy packet data
		CData		data((CData::ByteCount) mediaPacketAndLocation.getPacket().getByteCount());
		OV<SError>	error =
							mInternals->mRandomAccessDataSource->readData(mediaPacketAndLocation.getByteOffset(),
									data.getMutableBytePtr(), mediaPacketAndLocation.getPacket().getByteCount());
		ReturnValueIfError(error, TVResult<CMediaPacketSource::DataInfo>(*error));

		// Update
		mInternals->mNextPacketIndex++;

		return TVResult<CMediaPacketSource::DataInfo>(
				CMediaPacketSource::DataInfo(data, mediaPacketAndLocation.getPacket().getDuration()));
	} else
		// End of data
		return TVResult<CMediaPacketSource::DataInfo>(SError::mEndOfData);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<TArray<SMedia::Packet> > CSeekableVaryingMediaPacketSource::readNextInto(CData& data,
		const OV<UInt32>& maxPacketCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Add packets
	UInt32					dataByteCountRemaining = (UInt32) data.getByteCount();
	UInt8*					packetDataPtr = (UInt8*) data.getMutableBytePtr();
	UInt32					packetCount = maxPacketCount.hasValue() ? *maxPacketCount : ~0;
	TNArray<SMedia::Packet>	mediaPackets;
	while ((packetCount > 0) && (mInternals->mNextPacketIndex < mInternals->mMediaPacketAndLocations.getCount())) {
		// Setup
		SMedia::PacketAndLocation&	mediaPacketAndLocation =
											mInternals->mMediaPacketAndLocations.getAt(mInternals->mNextPacketIndex);

		// Check if have space
		if (mediaPacketAndLocation.getPacket().getByteCount() <= dataByteCountRemaining) {
			// Copy packet data
			OV<SError>	error =
								mInternals->mRandomAccessDataSource->readData(mediaPacketAndLocation.getByteOffset(),
										packetDataPtr, mediaPacketAndLocation.getPacket().getByteCount());
			ReturnValueIfError(error, TVResult<TArray<SMedia::Packet> >(*error));

			// Update
			mInternals->mNextPacketIndex++;

			// Update info
			dataByteCountRemaining -= mediaPacketAndLocation.getPacket().getByteCount();
			packetDataPtr += mediaPacketAndLocation.getPacket().getByteCount();
			packetCount--;
			mediaPackets += mediaPacketAndLocation.getPacket();
		} else
			// No more space
			break;
	}

	return !mediaPackets.isEmpty() ?
			TVResult<TArray<SMedia::Packet> >(mediaPackets) : TVResult<TArray<SMedia::Packet> >(SError::mEndOfData);
}
