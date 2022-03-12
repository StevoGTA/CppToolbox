//----------------------------------------------------------------------------------------------------------------------
//	CChunkReader.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CByteReader.h"
#include "CUUID.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CChunkReader

class CChunkReader : public CByteReader {
	// ChunkInfo
	public:
		struct ChunkInfo {
			// Methods
			ChunkInfo(OSType id, UInt64 byteCount, UInt64 thisChunkPos, UInt64 nextChunkPos) :
				mID(id), mByteCount(byteCount), mThisChunkPos(thisChunkPos), mNextChunkPos(nextChunkPos)
				{}
			ChunkInfo(const CUUID& uuid, UInt64 byteCount, UInt64 thisChunkPos, UInt64 nextChunkPos) :
				mUUID(uuid), mByteCount(byteCount), mThisChunkPos(thisChunkPos), mNextChunkPos(nextChunkPos)
				{}
			ChunkInfo(const ChunkInfo& other) :
				mID(other.mID), mByteCount(other.mByteCount), mThisChunkPos(other.mThisChunkPos),
						mNextChunkPos(other.mNextChunkPos)
				{}

			// Properties
			OV<OSType>	mID;
			OI<CUUID>	mUUID;
			UInt64		mByteCount;
			UInt64		mThisChunkPos;
			UInt64		mNextChunkPos;
		};

	// Methods
	public:
							// Lifecycle methods
							CChunkReader(const I<CSeekableDataSource>& seekableDataSource, bool isBigEndian) :
								CByteReader(seekableDataSource, isBigEndian)
								{}

							// Instance methods
		TIResult<ChunkInfo>	readChunkInfo() const;
		TIResult<CData>		readPayload(const ChunkInfo& chunkInfo) const;
		OI<SError>			seekToNext(const ChunkInfo& chunkInfo) const;
};
