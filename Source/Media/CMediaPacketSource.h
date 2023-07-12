//----------------------------------------------------------------------------------------------------------------------
//	CMediaPacketSource.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDataSource.h"
#include "SMedia.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaPacketSource

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
		virtual										~CMediaPacketSource() {}

													// Instance methods
		virtual	UInt32								seekToDuration(UInt32 duration) = 0;
		virtual	void								seekToPacket(UInt32 packetIndex) = 0;
				UInt32								seekToKeyframe(UInt32 initialFrameIndex,
															const TNumberArray<UInt32>& keyframeIndexes);

		virtual	TVResult<DataInfo>					readNext() = 0;
		virtual	TVResult<TArray<SMedia::Packet> >	readNextInto(CData& data,
															const OV<UInt32>& maxPacketCount = OV<UInt32>()) = 0;

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
		UInt32									seekToDuration(UInt32 duration);
		void									seekToPacket(UInt32 packetIndex)
													{ AssertFailUnimplemented(); }

		TVResult<CMediaPacketSource::DataInfo>	readNext();
		TVResult<TArray<SMedia::Packet> >		readNextInto(CData& data,
														const OV<UInt32>& maxPacketCount = OV<UInt32>());

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
		UInt32									seekToDuration(UInt32 duration);
		void									seekToPacket(UInt32 packetIndex);

		TVResult<CMediaPacketSource::DataInfo>	readNext();
		TVResult<TArray<SMedia::Packet> >		readNextInto(CData& data,
														const OV<UInt32>& maxPacketCount = OV<UInt32>());

	// Properties
	private:
		Internals*	mInternals;
};
