//----------------------------------------------------------------------------------------------------------------------
//	SMedia.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CData.h"
#include "TimeAndDate.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SMedia

struct SMedia {
	// Packet - Opaque data for compressed audio or video frames
	struct Packet {
				// Lifecycle methods
				Packet(UInt32 duration, UInt32 byteCount) : mDuration(duration), mByteCount(byteCount) {}
				Packet(const Packet& other) : mDuration(other.mDuration), mByteCount(other.mByteCount) {}

				// Instance methods
		UInt32	getDuration() const
					{ return mDuration; }
		UInt32	getByteCount() const
					{ return mByteCount; }

		// Properties
		private:
			UInt32	mDuration;	// Duration is contextual - typically frame count for audio and time for video
			UInt32	mByteCount;
	};

	// Packet and Location - Packet with corresponding absolute position in the containing opaque data.
	struct PacketAndLocation {
								// Lifecycle methods
								PacketAndLocation(Packet packet, UInt64 byteOffset) :
									mPacket(packet), mByteOffset(byteOffset)
									{}
								PacketAndLocation(UInt32 duration, UInt32 byteCount, UInt64 byteOffset) :
									mPacket(Packet(duration, byteCount)), mByteOffset(byteOffset)
									{}

								// Instance methods
				const	Packet&	getPacket() const
									{ return mPacket; }
						UInt64	getByteOffset() const
									{ return mByteOffset; }

								// Class methods
		static			UInt64	getTotalByteCount(const TArray<PacketAndLocation>& packetAndLocations)
									{
										// Setup
										UInt64	byteCount = 0;

										// Iterate
										for (TIteratorD<PacketAndLocation> iterator = packetAndLocations.getIterator();
												iterator.hasValue(); iterator.advance())
											// Update byte count
											byteCount += iterator->mPacket.getByteCount();

										return byteCount;
									}

		// Properties
		private:
			Packet	mPacket;
			UInt64	mByteOffset;
	};

	// Packet Data
	struct PacketData {
						// Lifecycle methods
						PacketData(UInt32 duration, const CData& data) : mDuration(duration), mData(data) {}
						PacketData(const PacketData& other) : mDuration(other.mDuration), mData(other.mData) {}

						// Instance methods
				UInt32	getDuration() const
							{ return mDuration; }
		const	CData&	getData() const
							{ return mData; }

		// Properties
		private:
			UInt32	mDuration;	// Duration is contextual - typically frame count for audio and time for video
			CData	mData;
	};

	// SegmentInfo
	struct SegmentInfo {
								// Lifecycle methods
								SegmentInfo(UniversalTimeInterval duration, UInt32 bitrate) :
									mDuration(duration), mBitrate(bitrate)
									{}
								SegmentInfo(UniversalTimeInterval duration, UInt64 byteCount) :
									mDuration(duration),
											mBitrate((UInt32) ((Float32) byteCount * 8 / (Float32) duration))
									{}

								// Instance methods
		UniversalTimeInterval	getDuration() const
									{ return mDuration; }
		UInt32					getBitrate() const
									{ return mBitrate; }

		// Properties
		private:
			UniversalTimeInterval	mDuration;
			UInt32					mBitrate;
	};

	// SourceInfo
	struct SourceInfo {
									// Lifecycle methods
									SourceInfo(UniversalTimeInterval timeInterval) : mTimeInterval(timeInterval) {}
									SourceInfo(const SourceInfo& other) : mTimeInterval(other.mTimeInterval) {}

									// Instance methods
			UniversalTimeInterval	getTimeInterval() const
										{ return mTimeInterval; }

		// Properties
		private:
			UniversalTimeInterval	mTimeInterval;
	};
};
