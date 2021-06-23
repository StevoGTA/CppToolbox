//----------------------------------------------------------------------------------------------------------------------
//	SMediaPacket.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "SMediaPacket.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CPacketMediaReaderInternals

class CPacketMediaReaderInternals {
	public:
		CPacketMediaReaderInternals(const I<CSeekableDataSource>& seekableDataSource,
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
// MARK: - CPacketMediaReader

// MARK: Properties

SError	CPacketMediaReader::mBufferTooSmall(CString(OSSTR("CPacketMediaReader")), 1,
				CString(OSSTR("Buffer too small")));

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CPacketMediaReader::CPacketMediaReader(const I<CSeekableDataSource>& seekableDataSource,
		const TArray<SMediaPacketAndLocation>& mediaPacketAndLocations) : CMediaReader()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CPacketMediaReaderInternals(seekableDataSource, mediaPacketAndLocations);
}

//----------------------------------------------------------------------------------------------------------------------
CPacketMediaReader::~CPacketMediaReader()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CMediaReader methods

//----------------------------------------------------------------------------------------------------------------------
Float32 CPacketMediaReader::getPercenConsumed() const
//----------------------------------------------------------------------------------------------------------------------
{
	return (Float32) mInternals->mNextPacketIndex / (Float32) mInternals->mMediaPacketAndLocations.getCount();
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CPacketMediaReader::set(const SMediaPosition& mediaPosition,
		const SAudioProcessingFormat& audioProcessingFormat)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get new sample position
	UInt64	frameIndex = mediaPosition.getFrameIndex(audioProcessingFormat.getSampleRate());

	// Find packet index
	mInternals->mNextPacketIndex = 0;
	for (TIteratorD<SMediaPacketAndLocation> iterator = mInternals->mMediaPacketAndLocations.getIterator();
			iterator.hasValue(); iterator.advance()) {
		// Setup
		const	SMediaPacket& mediaPacket = iterator->mMediaPacket;
		if (frameIndex > mediaPacket.mDuration) {
			// Advance another packet
			mInternals->mNextPacketIndex++;
			frameIndex -= mediaPacket.mDuration;
		} else
			// Done
			break;
	}

	return OI<SError>();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
TIResult<SMediaPacket> CPacketMediaReader::readPacket(void* buffer, UInt64 bufferAvailableByteCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if still have available packets
	if (mInternals->mNextPacketIndex < mInternals->mMediaPacketAndLocations.getCount()) {
		// Setup
		SMediaPacketAndLocation&	mediaPacketAndLocation =
											mInternals->mMediaPacketAndLocations.getAt(mInternals->mNextPacketIndex);

		// Check if have space
		if (mediaPacketAndLocation.mMediaPacket.mByteCount <= bufferAvailableByteCount) {
			// Copy packet data
			OI<SError>	error =
								mInternals->mSeekableDataSource->readData(mediaPacketAndLocation.mPos, buffer,
										mediaPacketAndLocation.mMediaPacket.mByteCount);
			ReturnValueIfError(error, TIResult<SMediaPacket>(*error));

			// Update
			mInternals->mNextPacketIndex++;

			return TIResult<SMediaPacket>(mediaPacketAndLocation.mMediaPacket);
		} else
			// Not enough space
			return TIResult<SMediaPacket>(mBufferTooSmall);
	} else
		// No more data
		return TIResult<SMediaPacket>(SError::mEndOfData);
}

//----------------------------------------------------------------------------------------------------------------------
TIResult<TArray<SMediaPacket> > CPacketMediaReader::readPackets(CData& data) const
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
								mInternals->mSeekableDataSource->readData(mediaPacketAndLocation.mPos, packetDataPtr,
										mediaPacketAndLocation.mMediaPacket.mByteCount);
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

	return TIResult<TArray<SMediaPacket> >(mediaPackets);
}
