//----------------------------------------------------------------------------------------------------------------------
//	CChunkReader.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CChunkReader.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CChunkReaderInternals

class CChunkReaderInternals {
	public:
		CChunkReaderInternals()
			{}
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CChunkReader

// MARK:  Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CChunkReader::CChunkReader(const I<CSeekableDataSource>& seekableDataSource, bool isBigEndian) :
		CByteReader(seekableDataSource, isBigEndian)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CChunkReaderInternals();
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

	return TIResult<ChunkInfo>(ChunkInfo(id, size, getPos(), getPos() + size + (size % 1)));
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
