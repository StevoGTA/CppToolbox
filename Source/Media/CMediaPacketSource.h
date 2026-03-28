//----------------------------------------------------------------------------------------------------------------------
//	CMediaPacketSource.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDataSource.h"
#include "SMedia.h"
#include "Tuple.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaPacketSource

class CMediaPacketSource {
	// MediaPacketsAndBuffer
	public:
		struct MediaPacketsAndBuffer : public TV2<TArray<SMedia::Packet>, TBuffer<UInt8> > {
			// Methods
			public:
												// Lifecycle methods
												MediaPacketsAndBuffer(const TArray<SMedia::Packet>& mediaPackets,
														const TBuffer<UInt8>& buffer) :
													TV2<TArray<SMedia::Packet>, TBuffer<UInt8>>(mediaPackets, buffer)
													{}

												// Instance methods
				const	TArray<SMedia::Packet>&	getMediaPackets() const
													{ return getA(); }
				const	TBuffer<UInt8>&			getBuffer() const
													{ return getB(); }
		};

	// Methods
	public:
												// Lifecycle methods
		virtual									~CMediaPacketSource() {}

												// Instance methods
		virtual	UInt32							seekToDuration(UInt32 duration) = 0;
		virtual	void							seekToPacket(UInt32 packetIndex) = 0;
				UInt32							seekToKeyframe(UInt32 initialFrameIndex,
														const TNumberArray<UInt32>& keyframeIndexes);

		virtual	TVResult<SMedia::PacketData>	readNext() = 0;
		virtual	TVResult<MediaPacketsAndBuffer>	readNext(UInt64 maxByteCount) = 0;
		virtual	TVResult<SMedia::Packet>		readNextInto(TBuffer<UInt8>& buffer) = 0;

	protected:
												// Lifecycle methods
												CMediaPacketSource() {}
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSeekableUniformMediaPacketSource

class CSeekableUniformMediaPacketSource : public CMediaPacketSource {
	// Classes
	private:
		class Internals;

	// Methods
	public:
										// Lifecycle methods
										CSeekableUniformMediaPacketSource(
												const I<CRandomAccessDataSource>& randomAccessDataSource,
												UInt64 byteOffset, UInt64 byteCount, UInt32 bytesPerPacket,
												UInt32 durationPerPacket);
										~CSeekableUniformMediaPacketSource();

										// CMediaPacketSource methods
		UInt32							seekToDuration(UInt32 duration);
		void							seekToPacket(UInt32 packetIndex)
											{ AssertFailUnimplemented(); }

		TVResult<SMedia::PacketData>	readNext();
		TVResult<MediaPacketsAndBuffer>	readNext(UInt64 maxByteCount);
		TVResult<SMedia::Packet>		readNextInto(TBuffer<UInt8>& buffer);

	// Properties
	private:
		Internals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSeekableVaryingMediaPacketSource

class CSeekableVaryingMediaPacketSource : public CMediaPacketSource {
	// Classes
	private:
		class Internals;

	// Methods
	public:
										// Lifecycle methods
										CSeekableVaryingMediaPacketSource(
												const I<CRandomAccessDataSource>& randomAccessDataSource,
												const TArray<SMedia::PacketAndLocation>& mediaPacketAndLocations);
										~CSeekableVaryingMediaPacketSource();

										// CMediaPacketSource methods
		UInt32							seekToDuration(UInt32 duration);
		void							seekToPacket(UInt32 packetIndex);

		TVResult<SMedia::PacketData>	readNext();
		TVResult<MediaPacketsAndBuffer>	readNext(UInt64 maxByteCount);
		TVResult<SMedia::Packet>		readNextInto(TBuffer<UInt8>& buffer);

	// Properties
	private:
		Internals*	mInternals;
};
