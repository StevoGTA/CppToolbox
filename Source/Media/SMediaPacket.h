//----------------------------------------------------------------------------------------------------------------------
//	SMediaPacket.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CCodec.h"
#include "CDataSource.h"
#include "SAudioFormats.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SMediaPacket

// Opaque data for compressed audio or video frames
struct SMediaPacket {
	// Lifecycle methods
	SMediaPacket(UInt32 duration, UInt32 byteCount) : mDuration(duration), mByteCount(byteCount) {}
	SMediaPacket(const SMediaPacket& other) : mDuration(other.mDuration), mByteCount(other.mByteCount) {}

	// Properties
	UInt32	mDuration;	// Duration is contextual - typically frame count for audio and time for video
	UInt32	mByteCount;
};


//----------------------------------------------------------------------------------------------------------------------
// MARK: - SMediaPacketAndLocation

// SMediaPacket with corresponding absolute position in the containing opaque data.
struct SMediaPacketAndLocation {
	// Lifecycle methods
	SMediaPacketAndLocation(SMediaPacket mediaPacket, UInt64 pos) : mMediaPacket(mediaPacket), mPos(pos) {}

	// Properties
	SMediaPacket	mMediaPacket;
	UInt64			mPos;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: CPacketMediaReader

class CPacketMediaReaderInternals;
class CPacketMediaReader : public CMediaReader {
	// MediaPacketDataInfo
	public:
		struct MediaPacketDataInfo {
								// Methods
								MediaPacketDataInfo(const CData& data, UInt32 duration) :
									mData(data), mDuration(duration)
									{}
								MediaPacketDataInfo(const MediaPacketDataInfo& other) :
									mData(other.mData), mDuration(other.mDuration)
									{}

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
										CPacketMediaReader(const I<CSeekableDataSource>& seekableDataSource,
												const TArray<SMediaPacketAndLocation>& mediaPacketAndLocations);
										~CPacketMediaReader();

										// CMediaReader methods
		Float32							getPercentConsumed() const;
		OI<SError>						set(UInt64 frameIndex);

										// Instance methods
		TIResult<TArray<SMediaPacket> >	readMediaPackets(CData& data) const;
		TIResult<MediaPacketDataInfo>	readNextMediaPacketDataInfo() const;
		TVResult<UInt32>				readNextMediaPacket(void* buffer) const;

	// Properties
	public:
		static	SError					mBufferTooSmall;

	private:
		CPacketMediaReaderInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CPacketsDecodeInfo

class CPacketsDecodeInfo : public CCodec::DecodeInfo {
	// Methods
	public:
								// Lifecycle methods
								CPacketsDecodeInfo(const TArray<SMediaPacketAndLocation>& mediaPacketAndLocations) :
									DecodeInfo(), mMediaPacketAndLocations(mediaPacketAndLocations)
									{}
								CPacketsDecodeInfo(const SMediaPacket& mediaPacket, UInt64 startByteOffset,
										UInt32 mediaPacketCount) :
									DecodeInfo(), mMediaPacketAndLocations(TNArray<SMediaPacketAndLocation>())
									{
										// Compose media packet and locations
										TNArray<SMediaPacketAndLocation>	mediaPacketAndLocations;
										for (UInt32 i = 0; i < mediaPacketCount;
												i++, startByteOffset += mediaPacket.mByteCount)
											// Add media packet and location
											mediaPacketAndLocations +=
													SMediaPacketAndLocation(mediaPacket, startByteOffset);
										mMediaPacketAndLocations = mediaPacketAndLocations;
									}

								// DecodeInfo methods
		I<CMediaReader>			createMediaReader(const I<CSeekableDataSource>& seekableDataSource) const
									{ return I<CMediaReader>(
											new CPacketMediaReader(seekableDataSource, mMediaPacketAndLocations)); }

								// Instance methods
		I<CPacketMediaReader>	createPacketMediaReader(const I<CSeekableDataSource>& seekableDataSource) const
									{ return I<CPacketMediaReader>(
											new CPacketMediaReader(seekableDataSource, mMediaPacketAndLocations)); }

	// Properties
	private:
		TArray<SMediaPacketAndLocation>	mMediaPacketAndLocations;
};
