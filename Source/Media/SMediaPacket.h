//----------------------------------------------------------------------------------------------------------------------
//	SMediaPacket.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDataSource.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SMediaPacket - Opaque data for compressed audio or video frames

struct SMediaPacket {
	// Lifecycle methods
	SMediaPacket(UInt32 duration, UInt32 byteCount) : mDuration(duration), mByteCount(byteCount) {}
	SMediaPacket(const SMediaPacket& other) : mDuration(other.mDuration), mByteCount(other.mByteCount) {}

	// Properties
	UInt32	mDuration;	// Duration is contextual - typically frame count for audio and time for video
	UInt32	mByteCount;
};


//----------------------------------------------------------------------------------------------------------------------
// MARK: - SMediaPacketAndLocation - SMediaPacket with corresponding absolute position in the containing opaque data.

struct SMediaPacketAndLocation {
					// Lifecycle methods
					SMediaPacketAndLocation(SMediaPacket mediaPacket, UInt64 byteOffset) :
						mMediaPacket(mediaPacket), mByteOffset(byteOffset)
						{}

					// Class methods
	static	UInt64	getTotalByteCount(const TArray<SMediaPacketAndLocation>& mediaPacketAndLocations)
						{
							// Setup
							UInt64	byteCount = 0;

							// Iterate
							for (TIteratorD<SMediaPacketAndLocation> iterator = mediaPacketAndLocations.getIterator();
									iterator.hasValue(); iterator.advance())
								// Update byte count
								byteCount += iterator->mMediaPacket.mByteCount;

							return byteCount;
						}

	// Properties
	SMediaPacket	mMediaPacket;
	UInt64			mByteOffset;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMediaPacketSource

class CMediaPacketSource {
	// DataInfo
	public:
		struct DataInfo {
								// Methods
								DataInfo(const CData& data, UInt32 duration) : mData(data), mDuration(duration) {}
								DataInfo(const DataInfo& other) : mData(other.mData), mDuration(other.mDuration) {}

				const	CData&	getData() const
									{ return mData; }
						UInt32	getDuration() const
									{ return mDuration; }

			// Properties
			private:
				CData	mData;
				UInt32	mDuration;
		};

	// Methods
	public:
												// Lifecycle methods
		virtual									~CMediaPacketSource() {}

												// Instance methods
		virtual	UInt32							seekToDuration(UInt32 duration) = 0;
		virtual	void							seekToPacket(UInt32 packetIndex) = 0;

		virtual	TIResult<DataInfo>				getNextPacket() = 0;
		virtual	TIResult<TArray<SMediaPacket> >	getMediaPackets(CData& data) = 0;

	protected:
												// Lifecycle methods
												CMediaPacketSource() {}
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSeekableUniformMediaPacketSource

class CSeekableUniformMediaPacketSourceInternals;
class CSeekableUniformMediaPacketSource : public CMediaPacketSource {
	// Methods
	public:
												// Lifecycle methods
												CSeekableUniformMediaPacketSource(
														const I<CSeekableDataSource>& seekableDataSource,
														UInt64 byteOffset, UInt64 byteCount, UInt32 bytesPerPacket,
														UInt32 durationPerPacket);
												~CSeekableUniformMediaPacketSource();

												// CMediaPacketSource methods
		UInt32									seekToDuration(UInt32 duration);
		void									seekToPacket(UInt32 packetIndex)
													{ AssertFailUnimplemented(); }

		TIResult<CMediaPacketSource::DataInfo>	getNextPacket();
		TIResult<TArray<SMediaPacket> >			getMediaPackets(CData& data)
													{
														AssertFailUnimplemented();

														return TIResult<TArray<SMediaPacket> >(AssertFailedError);
													}

	// Properties
	private:
		CSeekableUniformMediaPacketSourceInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSeekableVaryingMediaPacketSource

class CSeekableVaryingMediaPacketSourceInternals;
class CSeekableVaryingMediaPacketSource : public CMediaPacketSource {
	// Methods
	public:
												// Lifecycle methods
												CSeekableVaryingMediaPacketSource(
														const I<CSeekableDataSource>& seekableDataSource,
														const TArray<SMediaPacketAndLocation>& mediaPacketAndLocations);
												~CSeekableVaryingMediaPacketSource();

												// CMediaPacketSource methods
		UInt32									seekToDuration(UInt32 duration);
		void									seekToPacket(UInt32 packetIndex);

		TIResult<CMediaPacketSource::DataInfo>	getNextPacket();
		TIResult<TArray<SMediaPacket> >			getMediaPackets(CData& data);

	// Properties
	private:
		CSeekableVaryingMediaPacketSourceInternals*	mInternals;
};
