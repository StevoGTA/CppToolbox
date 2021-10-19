//----------------------------------------------------------------------------------------------------------------------
//	CChunkReader.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CByteReader.h"

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
			ChunkInfo(const ChunkInfo& other) :
				mID(other.mID), mByteCount(other.mByteCount), mThisChunkPos(other.mThisChunkPos),
						mNextChunkPos(other.mNextChunkPos)
				{}

			// Properties
			OSType	mID;
			UInt64	mByteCount;
			UInt64	mThisChunkPos;
			UInt64	mNextChunkPos;
		};

	// Methods
	public:
							// Lifecycle methods
							CChunkReader(const I<CSeekableDataSource>& seekableDataSource, bool isBigEndian) :
								CByteReader(seekableDataSource, isBigEndian)
								{}

							// Instance methods
		OI<SError>			seekToNextChunk(const ChunkInfo& chunkInfo) const;

		TIResult<CData>		readData(const ChunkInfo& chunkInfo) const;

		TIResult<ChunkInfo>	readChunkInfo() const;
};
