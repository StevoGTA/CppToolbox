//----------------------------------------------------------------------------------------------------------------------
//	CChunkReader.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CChunkReader.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CChunkReaderInternals

class CChunkReaderInternals {
	public:
		CChunkReaderInternals(CChunkReader::Format format) : mFormat(format) {}

		CChunkReader::Format	mFormat;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CChunkReader

// MARK:  Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CChunkReader::CChunkReader(const I<CRandomAccessDataSource>& randomAccessDataSource, Format format) :
		CByteReader(randomAccessDataSource, format == kFormat32BitBigEndian)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CChunkReaderInternals(format);
}

//----------------------------------------------------------------------------------------------------------------------
CChunkReader::~CChunkReader()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
TVResult<CChunkReader::ChunkInfo> CChunkReader::readChunkInfo() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check format
	switch (mInternals->mFormat) {
		case kFormat32BitBigEndian:
		case kFormat32BitLittleEndian: {
			// Read 32-bit header
			TVResult<OSType>	id = readOSType();
			ReturnValueIfResultError(id, TVResult<CChunkReader::ChunkInfo>(id.getError()));

			TVResult<UInt32>	_byteCount = readUInt32();
			ReturnValueIfResultError(_byteCount, TVResult<CChunkReader::ChunkInfo>(_byteCount.getError()));
			UInt32	byteCount = *_byteCount;

			UInt64	nextChunkPos = getPos() + byteCount;
			if ((nextChunkPos & 1) != 0)
				// Align
				nextChunkPos += 1;

			return TVResult<ChunkInfo>(ChunkInfo(*id, byteCount, getPos(), nextChunkPos));
			}

		case kFormat64BitLittleEndian: {
			// Read 64-bit header
			TVResult<CUUID>	uuid = readUUID();
			ReturnValueIfResultError(uuid, TVResult<CChunkReader::ChunkInfo>(uuid.getError()));

			TVResult<UInt64>	_byteCount = readUInt64();
			ReturnValueIfResultError(_byteCount, TVResult<CChunkReader::ChunkInfo>(_byteCount.getError()));
			UInt64	byteCount = *_byteCount - sizeof(CUUID::Bytes) - sizeof(UInt64);

			UInt64	nextChunkPos = getPos() + byteCount;
			if ((nextChunkPos & 7) != 0)
				// Align
				nextChunkPos += 7 - (nextChunkPos & 7);

			return TVResult<ChunkInfo>(ChunkInfo(*uuid, byteCount, getPos(), nextChunkPos));
			}

#if defined(TARGET_OS_WINDOWS)
		default:
			return TVResult<CChunkReader::ChunkInfo>(SError::mUnimplemented);
#endif
	}
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CChunkReader::Chunk> CChunkReader::readChunk() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check format
	switch (mInternals->mFormat) {
		case kFormat32BitBigEndian:
		case kFormat32BitLittleEndian: {
			// Read 32-bit header
			TVResult<OSType>	id = readOSType();
			ReturnValueIfResultError(id, TVResult<CChunkReader::Chunk>(id.getError()));

			TVResult<UInt32>	_byteCount = readUInt32();
			ReturnValueIfResultError(_byteCount, TVResult<CChunkReader::Chunk>(_byteCount.getError()));
			UInt32	byteCount = *_byteCount;

			UInt64	nextChunkPos = getPos() + byteCount;
			if ((nextChunkPos & 1) != 0)
				// Align
				nextChunkPos += 1;

			TVResult<CData>	data = CByteReader::readData(byteCount);
			ReturnValueIfResultError(data, TVResult<CChunkReader::Chunk>(_byteCount.getError()));

			return TVResult<Chunk>(Chunk(*id, *data, nextChunkPos));
			}

		case kFormat64BitLittleEndian: {
			// Read 64-bit header
			TVResult<CUUID>	uuid = readUUID();
			ReturnValueIfResultError(uuid, TVResult<CChunkReader::Chunk>(uuid.getError()));

			TVResult<UInt64>	_byteCount = readUInt64();
			ReturnValueIfResultError(_byteCount, TVResult<CChunkReader::Chunk>(_byteCount.getError()));
			UInt64	byteCount = *_byteCount - sizeof(CUUID::Bytes) - sizeof(UInt64);

			UInt64	nextChunkPos = getPos() + byteCount;
			if ((nextChunkPos & 8) != 0)
				// Align
				nextChunkPos += 7 - (nextChunkPos & 8);

			TVResult<CData>	data = CByteReader::readData(byteCount);
			ReturnValueIfResultError(data, TVResult<CChunkReader::Chunk>(_byteCount.getError()));

			return TVResult<Chunk>(Chunk(*uuid, *data, nextChunkPos));
			}

#if defined(TARGET_OS_WINDOWS)
		default:
			return TVResult<CChunkReader::Chunk>(SError::mUnimplemented);
#endif
	}
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CData> CChunkReader::readPayload(const ChunkInfo& chunkInfo, const OV<UInt64>& byteCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Move to begining of chunk
	OV<SError>	error = setPos(kPositionFromBeginning, chunkInfo.getThisChunkPos());
	ReturnValueIfError(error, TVResult<CData>(*error));

	return CByteReader::readData(
			byteCount.hasValue() ? std::min<UInt64>(*byteCount, chunkInfo.getByteCount()) : chunkInfo.getByteCount());
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CChunkReader::seekToNext(const ChunkInfo& chunkInfo) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check situation
	if (chunkInfo.getNextChunkPos() < getByteCount())
		// Set pos
		return setPos(kPositionFromBeginning, chunkInfo.getNextChunkPos());
	else
		// End of data
		return OV<SError>(SError::mEndOfData);
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CChunkReader::seekToNext(const Chunk& chunk) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check situation
	if (chunk.getNextChunkPos() < getByteCount())
		// Set pos
		return setPos(kPositionFromBeginning, chunk.getNextChunkPos());
	else
		// End of data
		return OV<SError>(SError::mEndOfData);
}
