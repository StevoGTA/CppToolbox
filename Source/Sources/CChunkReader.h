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
							ChunkInfo(OSType id, const ChunkInfo& other) :
								mID(id), mByteCount(other.mByteCount), mThisChunkPos(other.mThisChunkPos),
										mNextChunkPos(other.mNextChunkPos)
								{}
							ChunkInfo(const CUUID& uuid, UInt64 byteCount, UInt64 thisChunkPos, UInt64 nextChunkPos) :
								mUUID(uuid), mByteCount(byteCount), mThisChunkPos(thisChunkPos),
										mNextChunkPos(nextChunkPos)
								{}
							ChunkInfo(const ChunkInfo& other) :
								mID(other.mID), mUUID(other.mUUID), mByteCount(other.mByteCount),
										mThisChunkPos(other.mThisChunkPos), mNextChunkPos(other.mNextChunkPos)
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
				OV<CUUID>	mUUID;
				UInt64		mByteCount;
				UInt64		mThisChunkPos;
				UInt64		mNextChunkPos;
		};

	// Chunk
	public:
		struct Chunk {
							// Lifecycle methods
							Chunk(OSType id, const CData& payloadData, UInt64 nextChunkPos) :
								mID(id), mPayloadData(payloadData), mNextChunkPos(nextChunkPos)
								{}
							Chunk(const CUUID& uuid, const CData& payloadData, UInt64 nextChunkPos) :
								mUUID(uuid), mPayloadData(payloadData), mNextChunkPos(nextChunkPos)
								{}
							Chunk(const Chunk& other) :
								mID(other.mID), mUUID(other.mUUID), mPayloadData(other.mPayloadData),
										mNextChunkPos(other.mNextChunkPos)
								{}

							// Instance methods
					bool	hasID() const { return mID.hasValue(); }
					OSType	getID() const { return *mID; }
			const	CUUID&	getUUID() const { return *mUUID; }
			const	CData&	getPayloadData() const { return mPayloadData; }
					UInt64	getNextChunkPos() const { return mNextChunkPos; }

			// Properties
			private:
				OV<OSType>	mID;
				OV<CUUID>	mUUID;
				CData		mPayloadData;
				UInt64		mNextChunkPos;
		};

	// Format
	public:
		enum Format {
			kFormat32BitBigEndian,
			kFormat32BitLittleEndian,
			kFormat64BitLittleEndian,
		};

	// Methods
	public:
							// Lifecycle methods
							CChunkReader(const I<CRandomAccessDataSource>& randomAccessDataSource, Format format);
							~CChunkReader();

							// Instance methods
		TVResult<ChunkInfo>	readChunkInfo() const;
		TVResult<Chunk>		readChunk() const;
		TVResult<CData>		readPayload(const ChunkInfo& chunkInfo, const OV<UInt64>& byteCount = OV<UInt64>()) const;
		OV<SError>			seekToNext(const ChunkInfo& chunkInfo) const;
		OV<SError>			seekToNext(const Chunk& chunk) const;

	// Properties
	private:
		CChunkReaderInternals*	mInternals;
};
