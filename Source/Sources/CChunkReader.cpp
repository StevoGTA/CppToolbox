//----------------------------------------------------------------------------------------------------------------------
//	CChunkReader.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CChunkReader.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CChunkReader

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CChunkReader::seekToNextChunk(const ChunkInfo& chunkInfo) const
//----------------------------------------------------------------------------------------------------------------------
{
	return setPos(kPositionFromBeginning, chunkInfo.mNextChunkPos);
}

//----------------------------------------------------------------------------------------------------------------------
TIResult<CData> CChunkReader::readData(const ChunkInfo& chunkInfo) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Move to begining of chunk
	OI<SError>	error = setPos(kPositionFromBeginning, chunkInfo.mThisChunkPos);
	ReturnValueIfError(error, TIResult<CData>(*error));

	return CByteReader::readData(chunkInfo.mByteCount);
}

//----------------------------------------------------------------------------------------------------------------------
TIResult<CChunkReader::ChunkInfo> CChunkReader::readChunkInfo() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt64	pos = getPos();

	// Read chunk header
	OSType	id;
	UInt64	size;
	if (true) {
		// Read 32-bit header
		TVResult<OSType>	_id = readOSType();
		ReturnValueIfResultError(_id, TIResult<CChunkReader::ChunkInfo>(_id.getError()));

		TVResult<UInt32>	_size = readUInt32();
		ReturnValueIfResultError(_size, TIResult<CChunkReader::ChunkInfo>(_size.getError()));

		id = _id.getValue();
		size = _size.getValue();
	}

	return TIResult<ChunkInfo>(ChunkInfo(id, size, pos, getPos() + size + (size % 1)));
}
