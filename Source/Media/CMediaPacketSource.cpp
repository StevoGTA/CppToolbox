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
	for (TNumberArray<UInt32>::Iterator iterator = keyframeIndexes.getIterator(); iterator; iterator++) {
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
TVResult<SMedia::PacketData> CSeekableUniformMediaPacketSource::readNext()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if can read next packet
	if (mInternals->mNextPacketIndex < mInternals->mPacketCount) {
		// Copy packet data
		UInt64			byteOffset =
								mInternals->mByteOffset + mInternals->mNextPacketIndex * mInternals->mBytesPerPacket;
		TVResult<CData>	data = mInternals->mRandomAccessDataSource->readData(byteOffset, mInternals->mBytesPerPacket);
		ReturnValueIfResultError(data, TVResult<SMedia::PacketData>(data.getError()));

		// Update
		mInternals->mNextPacketIndex++;

		return TVResult<SMedia::PacketData>(SMedia::PacketData(mInternals->mDurationPerPacket, *data));
	} else
		// End of data
		return TVResult<SMedia::PacketData>(SError::mEndOfData);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CMediaPacketSource::MediaPacketsAndBuffer> CSeekableUniformMediaPacketSource::readNext(UInt64 maxByteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Add packets
	UInt64					byteCountRemaining = maxByteCount;
	TBuffer<UInt8>			buffer(maxByteCount);
	UInt8*					packetDataPtr = *buffer;
	TNArray<SMedia::Packet>	mediaPackets;
	while (mInternals->mNextPacketIndex < mInternals->mPacketCount) {
		// Check if have space
		if (mInternals->mBytesPerPacket <= byteCountRemaining) {
			// Copy packet data
			UInt64		byteOffset =
								mInternals->mByteOffset + mInternals->mNextPacketIndex * mInternals->mBytesPerPacket;
			OV<SError>	error =
								mInternals->mRandomAccessDataSource->read(byteOffset, packetDataPtr,
											mInternals->mBytesPerPacket);
			ReturnValueIfError(error, TVResult<MediaPacketsAndBuffer>(*error));

			// Update
			mInternals->mNextPacketIndex++;

			// Update info
			byteCountRemaining -= mInternals->mBytesPerPacket;
			packetDataPtr += mInternals->mBytesPerPacket;
			mediaPackets += SMedia::Packet(mInternals->mDurationPerPacket, mInternals->mBytesPerPacket);
		} else
			// No more space
			break;
	}

	return !mediaPackets.isEmpty() ?
			TVResult<MediaPacketsAndBuffer>(
					MediaPacketsAndBuffer(mediaPackets, TBuffer<UInt8>(buffer, maxByteCount - byteCountRemaining))) :
			TVResult<MediaPacketsAndBuffer>(SError::mEndOfData);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<SMedia::Packet> CSeekableUniformMediaPacketSource::readNextInto(TBuffer<UInt8>& buffer)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if can read next packet
	if (mInternals->mNextPacketIndex < mInternals->mPacketCount) {
		// Copy packet data
		UInt64		byteOffset = mInternals->mByteOffset + mInternals->mNextPacketIndex * mInternals->mBytesPerPacket;
		OV<SError>	error = mInternals->mRandomAccessDataSource->read(byteOffset, *buffer, buffer.getByteCount());
		ReturnValueIfError(error, TVResult<SMedia::Packet>(*error));

		// Update
		mInternals->mNextPacketIndex++;

		return TVResult<SMedia::Packet>(SMedia::Packet(mInternals->mDurationPerPacket, mInternals->mBytesPerPacket));
	} else
		// End of data
		return TVResult<SMedia::Packet>(SError::mEndOfData);
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

		I<CRandomAccessDataSource>			mRandomAccessDataSource;
		TArray<SMedia::PacketAndLocation>	mMediaPacketAndLocations;
		UInt32								mNextPacketIndex;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSeekableVaryingMediaPacketSource

// MARK: Lifecycle methods

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
	for (TArray<SMedia::PacketAndLocation>::Iterator iterator = mInternals->mMediaPacketAndLocations.getIterator();
			iterator; iterator++) {
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
TVResult<SMedia::PacketData> CSeekableVaryingMediaPacketSource::readNext()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if can read next packet
	if (mInternals->mNextPacketIndex < mInternals->mMediaPacketAndLocations.getCount()) {
		// Setup
		SMedia::PacketAndLocation&	mediaPacketAndLocation =
											mInternals->mMediaPacketAndLocations.getAt(mInternals->mNextPacketIndex);

		// Copy packet data
		TVResult<CData>	data =
								mInternals->mRandomAccessDataSource->readData(mediaPacketAndLocation.getByteOffset(),
										mediaPacketAndLocation.getPacket().getByteCount());
		ReturnValueIfResultError(data, TVResult<SMedia::PacketData>(data.getError()));

		// Update
		mInternals->mNextPacketIndex++;

		return TVResult<SMedia::PacketData>(
				SMedia::PacketData(mediaPacketAndLocation.getPacket().getDuration(), *data));
	} else
		// End of data
		return TVResult<SMedia::PacketData>(SError::mEndOfData);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CMediaPacketSource::MediaPacketsAndBuffer> CSeekableVaryingMediaPacketSource::readNext(UInt64 maxByteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Add packets
	UInt64					byteCountRemaining = maxByteCount;
	TBuffer<UInt8>			buffer(maxByteCount);
	UInt8*					packetDataPtr = *buffer;
	TNArray<SMedia::Packet>	mediaPackets;
	while (mInternals->mNextPacketIndex < mInternals->mMediaPacketAndLocations.getCount()) {
		// Setup
		SMedia::PacketAndLocation&	mediaPacketAndLocation =
											mInternals->mMediaPacketAndLocations.getAt(mInternals->mNextPacketIndex);

		// Check if have space
		if (mediaPacketAndLocation.getPacket().getByteCount() <= byteCountRemaining) {
			// Copy packet data
			OV<SError>	error =
								mInternals->mRandomAccessDataSource->read(mediaPacketAndLocation.getByteOffset(),
										packetDataPtr, mediaPacketAndLocation.getPacket().getByteCount());
			ReturnValueIfError(error, TVResult<MediaPacketsAndBuffer>(*error));

			// Update
			mInternals->mNextPacketIndex++;

			// Update info
			byteCountRemaining -= mediaPacketAndLocation.getPacket().getByteCount();
			packetDataPtr += mediaPacketAndLocation.getPacket().getByteCount();
			mediaPackets += mediaPacketAndLocation.getPacket();
		} else
			// No more space
			break;
	}

	return !mediaPackets.isEmpty() ?
			TVResult<MediaPacketsAndBuffer>(
					MediaPacketsAndBuffer(mediaPackets, TBuffer<UInt8>(buffer, maxByteCount - byteCountRemaining))) :
			TVResult<MediaPacketsAndBuffer>(SError::mEndOfData);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<SMedia::Packet> CSeekableVaryingMediaPacketSource::readNextInto(TBuffer<UInt8>& buffer)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if can read next packet
	if (mInternals->mNextPacketIndex < mInternals->mMediaPacketAndLocations.getCount()) {
		// Setup
		SMedia::PacketAndLocation&	mediaPacketAndLocation =
											mInternals->mMediaPacketAndLocations.getAt(mInternals->mNextPacketIndex);

		// Copy packet data
		OV<SError>	error =
							mInternals->mRandomAccessDataSource->read(mediaPacketAndLocation.getByteOffset(),
									*buffer, buffer.getByteCount());
		ReturnValueIfError(error, TVResult<SMedia::Packet>(*error));

		// Update
		mInternals->mNextPacketIndex++;

		return TVResult<SMedia::Packet>(mediaPacketAndLocation.getPacket());
	} else
		// End of data
		return TVResult<SMedia::Packet>(SError::mEndOfData);
}
