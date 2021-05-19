//----------------------------------------------------------------------------------------------------------------------
//	CCodec.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CArray.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CCodec

class CCodec {
	// Structs
	public:
		// Opaque data for compressed audio or video frames
		struct Packet {
			// Lifecycle methods
			Packet(UInt32 duration, UInt32 byteCount) : mDuration(duration), mByteCount(byteCount) {}

			// Properties
			UInt32	mDuration;	// Duration is contextual - typically frame count for audio and time for video
			UInt32	mByteCount;
		};

		// Packet with corresponding absolute position in the containing opaque data.
		struct PacketAndLocation {
			// Lifecycle methods
			PacketAndLocation(Packet packet, SInt64 pos) : mPacket(packet), mPos(pos) {}

			// Properties
			Packet	mPacket;
			SInt64	mPos;
		};

	// Classes
	public:
		class DecodeInfo {
			// Methods
			public:
						// Lifecycle methods
						DecodeInfo() {}
				virtual	~DecodeInfo() {}
		};

		class DataDecodeInfo : public DecodeInfo {
			// Methods
			public:
						// Lifecycle methods
						DataDecodeInfo(SInt64 startOffset, SInt64 size) :
							DecodeInfo(), mStartOffset(startOffset), mSize(size)
							{}

						// Instance methods
				SInt64	getStartOffset() const
							{ return mStartOffset; }
				SInt64	getSize() const
							{ return mSize; }

			// Properties
			private:
				SInt64	mStartOffset;
				SInt64	mSize;
		};

		class PacketsDecodeInfo : public DecodeInfo {
			// Methods
			public:
													// Lifecycle methods
													PacketsDecodeInfo(
															const TArray<PacketAndLocation>& packetAndLocations) :
														DecodeInfo(), mPacketAndLocations(packetAndLocations)
														{}

													// Instance methods
				const	TArray<PacketAndLocation>&	getPacketAndLocations() const
														{ return mPacketAndLocations; }

			// Properties
			private:
				TArray<PacketAndLocation>	mPacketAndLocations;
		};

		class EncodeSettings {
			// Methods
			public:
				// Lifecycle methods
				EncodeSettings() : mDummy(false) {}
				EncodeSettings(const EncodeSettings& other) : mDummy(other.mDummy) {}

			// Properties
			private:
				bool	mDummy;
		};

	// Methods
	public:
				// Lifecycle methods
				CCodec() {}
		virtual	~CCodec() {}
};
