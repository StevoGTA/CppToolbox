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
CChunkReader::CChunkReader(const I<CSeekableDataSource>& seekableDataSource, Format format) :
		CByteReader(seekableDataSource, format == kFormat32BitBigEndian)
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
TIResult<CChunkReader::ChunkInfo> CChunkReader::readChunkInfo() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check format
	switch (mInternals->mFormat) {
		case kFormat32BitBigEndian:
		case kFormat32BitLittleEndian: {
			// Read 32-bit header
			TVResult<OSType>	id = readOSType();
			ReturnValueIfResultError(id, TIResult<CChunkReader::ChunkInfo>(id.getError()));

			TVResult<UInt32>	_size = readUInt32();
			ReturnValueIfResultError(_size, TIResult<CChunkReader::ChunkInfo>(_size.getError()));
			UInt32				size = *_size;

			UInt64				nextChunkPos = getPos() + size;
			if ((nextChunkPos % 1) != 0)
				// Align
				nextChunkPos += 1;

			return TIResult<ChunkInfo>(ChunkInfo(*id, size, getPos(), nextChunkPos));
			}

		case kFormat64BitLittleEndian: {
			// Read 64-bit header
			TIResult<CUUID>	uuid = readUUID();
			ReturnValueIfResultError(uuid, TIResult<CChunkReader::ChunkInfo>(uuid.getError()));

			TVResult<UInt64>	_size = readUInt64();
			ReturnValueIfResultError(_size, TIResult<CChunkReader::ChunkInfo>(_size.getError()));
			UInt64				size = *_size - sizeof(CUUID::Bytes) - sizeof(UInt64);

			UInt64				nextChunkPos = getPos() + size;
			if ((nextChunkPos % 8) != 0)
				// Align
				nextChunkPos += 7 - (nextChunkPos % 8);

			return TIResult<ChunkInfo>(ChunkInfo(*uuid, size, getPos(), nextChunkPos));
			}

#if defined(TARGET_OS_WINDOWS)
		default:
			return TIResult<CChunkReader::ChunkInfo>(SError::mUnimplemented);
#endif
	}
}

//----------------------------------------------------------------------------------------------------------------------
TIResult<CData> CChunkReader::readPayload(const ChunkInfo& chunkInfo) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Move to begining of chunk
	OI<SError>	error = setPos(kPositionFromBeginning, chunkInfo.getThisChunkPos());
	ReturnValueIfError(error, TIResult<CData>(*error));

	return CByteReader::readData(chunkInfo.getByteCount());
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CChunkReader::seekToNext(const ChunkInfo& chunkInfo) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check situation
	if (chunkInfo.getNextChunkPos() < getByteCount())
		// Set pos
		return setPos(kPositionFromBeginning, chunkInfo.getNextChunkPos());
	else
		// End of data
		return OI<SError>(SError::mEndOfData);
}
