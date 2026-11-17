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
										for (TArray<PacketAndLocation>::Iterator iterator =
														packetAndLocations.getIterator();
												iterator; iterator++)
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
						PacketData(UInt32 duration, const TBuffer<UInt8>& data) :
							mDuration(duration), mData(*data, data.getByteCount())
							{}
						PacketData(UInt32 duration, const CData& data) : mDuration(duration), mData(data) {}

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
																	Segment(
																			const SUniversalTimeIntervalRange&
																					timeIntervalRange,
																			UniversalTimeInterval inHandleTimeInterval,
																			UniversalTimeInterval outHandleTimeInterval,
																			UniversalTimeInterval
																					totalDurationTimeInterval) :
																		mTimeIntervalRange(
																				std::max(0.0,
																						timeIntervalRange.getStart() -
																								inHandleTimeInterval),
																				std::min(totalDurationTimeInterval,
																								timeIntervalRange
																												.getEnd() +
																										outHandleTimeInterval) -
																						std::max(0.0,
																								timeIntervalRange
																												.getStart() -
																										inHandleTimeInterval))
																		{}
				explicit											Segment(
																			const SUniversalTimeIntervalRange&
																					timeIntervalRange) :
																		mTimeIntervalRange(timeIntervalRange)
																		{}

																	// Instance methods
							const	SUniversalTimeIntervalRange&	getTimeIntervalRange() const
																		{ return mTimeIntervalRange; }
							UniversalTimeInterval					getMidDurationTimeInterval() const
																		{ return mTimeIntervalRange.getStart() +
																				mTimeIntervalRange.getLength() / 2.0; }

							bool									contains(UniversalTimeInterval timeInterval) const
																		{ return (timeInterval >=
																						mTimeIntervalRange.getStart()) &&
																				(timeInterval <
																						mTimeIntervalRange.getEnd()); }

			// Properties
			private:
				SUniversalTimeIntervalRange	mTimeIntervalRange;
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
