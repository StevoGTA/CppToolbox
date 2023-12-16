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

	// Segment
	public:
		struct Segment {
			// Methods
			public:
													// Lifecycle methods
													Segment() : mStartTimeInterval(0.0) {}
													Segment(UniversalTimeInterval startTimeInterval,
															const OV<UniversalTimeInterval>& durationTimeInterval) :
														mStartTimeInterval(startTimeInterval),
																mDurationTimeInterval(durationTimeInterval)
														{}
													Segment(const Segment& other) :
														mStartTimeInterval(other.mStartTimeInterval),
																mDurationTimeInterval(other.mDurationTimeInterval)
														{}

													// Instance methods
						UniversalTimeInterval		getStartTimeInterval() const
														{ return mStartTimeInterval; }
				const	OV<UniversalTimeInterval>&	getDurationTimeInterval() const
														{ return mDurationTimeInterval; }

						OV<UniversalTimeInterval>	getEndTimeInterval() const
														{ return mDurationTimeInterval.hasValue() ?
																OV<UniversalTimeInterval>(
																		mStartTimeInterval + *mDurationTimeInterval) :
																OV<UniversalTimeInterval>(); }

						Segment&					operator=(const Segment& other)
														{
															// Store
															mStartTimeInterval = other.mStartTimeInterval;
															mDurationTimeInterval = mDurationTimeInterval;

															return *this;
														}

			// Properties
			private:
				UniversalTimeInterval		mStartTimeInterval;
				OV<UniversalTimeInterval>	mDurationTimeInterval;
		};

	// SegmentInfo
	public:
		struct SegmentInfo {
			// Methods
			public:
										// Lifecycle methods
										SegmentInfo(UniversalTimeInterval duration, UInt32 bitrate) :
											mDuration(duration), mBitrate(bitrate)
											{}
										SegmentInfo(UniversalTimeInterval duration, UInt64 byteCount) :
											mDuration(duration),
													mBitrate((UInt32) ((Float32) byteCount * 8 / (Float32) duration))
											{}
										SegmentInfo(const SegmentInfo& other) :
											mDuration(other.mDuration), mBitrate(other.mBitrate)
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
};
