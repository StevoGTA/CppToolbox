//----------------------------------------------------------------------------------------------------------------------
//	CChunkReader.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CByteReader.h"
#include "CUUID.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CChunkReader

class CChunkReaderInternals;
class CChunkReader : public CByteReader {
	// ChunkInfo
	public:
		struct ChunkInfo {
							// Lifecycle methods
							ChunkInfo(OSType id, UInt64 byteCount, UInt64 thisChunkPos, UInt64 nextChunkPos) :
								mID(id), mByteCount(byteCount), mThisChunkPos(thisChunkPos), mNextChunkPos(nextChunkPos)
								{}
							ChunkInfo(const CUUID& uuid, UInt64 byteCount, UInt64 thisChunkPos, UInt64 nextChunkPos) :
								mUUID(uuid), mByteCount(byteCount), mThisChunkPos(thisChunkPos),
										mNextChunkPos(nextChunkPos)
								{}
							ChunkInfo(const ChunkInfo& other) :
								mID(other.mID), mByteCount(other.mByteCount), mThisChunkPos(other.mThisChunkPos),
										mNextChunkPos(other.mNextChunkPos)
								{}

							// Instance methods
					bool	hasID() const { return mID.hasValue(); }
					OSType	getID() const { return *mID; }
			const	CUUID&	getUUID() const { return *mUUID; }
					UInt64	getByteCount() const { return mByteCount; }
					UInt64	getThisChunkPos() const { return mThisChunkPos; }
					UInt64	getNextChunkPos() const { return mNextChunkPos; }

			// Properties
			private:
				OV<OSType>	mID;
				OI<CUUID>	mUUID;
				UInt64		mByteCount;
				UInt64		mThisChunkPos;
				UInt64		mNextChunkPos;
		};

	// Methods
	public:
							// Lifecycle methods
							CChunkReader(const I<CSeekableDataSource>& seekableDataSource, bool isBigEndian);
							~CChunkReader();

							// Instance methods
		TIResult<ChunkInfo>	readChunkInfo() const;
		TIResult<CData>		readPayload(const ChunkInfo& chunkInfo) const;
		OI<SError>			seekToNext(const ChunkInfo& chunkInfo) const;

	// Properties
	private:
		CChunkReaderInternals*	mInternals;
};
