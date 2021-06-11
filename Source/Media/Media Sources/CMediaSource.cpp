//----------------------------------------------------------------------------------------------------------------------
//	CMediaSource.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CMediaSource.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CString	sAtomMediaSourceErrorDomain(OSSTR("CCAtomMediaSource"));
static	SError	sNoAtomInfoError(sAtomMediaSourceErrorDomain, 1, CString(OSSTR("No Atom Info")));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CChunkMediaSource

// CMediaSource methods

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CChunkMediaSource::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	return mByteReader.setPos(CByteReader::kPositionFromBeginning, 0);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
OI<CChunkMediaSource::ChunkInfo> CChunkMediaSource::getChunkInfo(OI<SError>& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SInt64	pos = mByteReader.getPos();

	// Read chunk header
	OSType	id;
	UInt64	size;
	if (true) {
		// Read 32-bit header
		TVResult<OSType>	_id = mByteReader.readOSType();
		outError = _id.getError();
		ReturnValueIfError(outError, OI<ChunkInfo>());
		id = *_id.getValue();

		TVResult<UInt32>	_size = mByteReader.readUInt32();
		outError = _size.getError();
		ReturnValueIfError(outError, OI<ChunkInfo>());
		size = *_size.getValue();
	}

	return OI<ChunkInfo>(new ChunkInfo(id, size, pos, mByteReader.getPos() + size + (size % 1)));
}

//----------------------------------------------------------------------------------------------------------------------
OI<CData> CChunkMediaSource::getChunk(const ChunkInfo& chunkInfo, OI<SError>& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Move to begining of chunk
	outError = mByteReader.setPos(CByteReader::kPositionFromBeginning, chunkInfo.mThisChunkPos);
	ReturnValueIfError(outError, OI<CData>());

	TIResult<CData>	dataResult = mByteReader.readData(chunkInfo.mSize);
	outError = dataResult.getError();

	return dataResult.getValue();
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CChunkMediaSource::seekToNextChunk(const ChunkInfo& chunkInfo) const
//----------------------------------------------------------------------------------------------------------------------
{
	return mByteReader.setPos(CByteReader::kPositionFromBeginning, chunkInfo.mNextChunkPos);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAtomMediaSource

// CMediaSource methods

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CAtomMediaSource::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	return mByteReader.setPos(CByteReader::kPositionFromBeginning, 0);
}

// Instance methods

//----------------------------------------------------------------------------------------------------------------------
OI<CAtomMediaSource::AtomInfo> CAtomMediaSource::getAtomInfo(OI<SError>& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read size as UInt32
	TVResult<UInt32>	size32 = mByteReader.readUInt32();
	outError = size32.getError();
	ReturnValueIfError(outError, OI<AtomInfo>());

	// Read type as OSType
	TVResult<OSType>	type = mByteReader.readOSType();
	outError = type.getError();
	ReturnValueIfError(outError, OI<AtomInfo>());

	// Do we need to read large size?
	UInt64	payloadSize;
	if (*size32.getValue() == 1) {
		// Yes
		TVResult<UInt64>	size64 = mByteReader.readUInt64();
		outError = size64.getError();
		ReturnValueIfError(outError, OI<AtomInfo>());

		payloadSize = *size64.getValue() - 12;
	} else
		// No
		payloadSize = *size32.getValue() - 8;

	return OI<AtomInfo>(new AtomInfo(*type.getValue(), mByteReader.getPos(), payloadSize));
}

//----------------------------------------------------------------------------------------------------------------------
OI<CData> CAtomMediaSource::getAtomPayload(const AtomInfo& atomInfo, OI<SError>& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Move to
	outError = mByteReader.setPos(CByteReader::kPositionFromBeginning, atomInfo.mPayloadPos);
	ReturnValueIfError(outError, OI<CData>());

	TIResult<CData>	dataResult = mByteReader.readData(atomInfo.mPayloadSize);
	outError = dataResult.getError();

	return dataResult.getValue();
}

//----------------------------------------------------------------------------------------------------------------------
OI<CData> CAtomMediaSource::getAtomPayload(const OR<AtomInfo>& atomInfo, OI<SError>& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if have AtomInfo
	if (atomInfo.hasReference())
		// Have AtomInfo
		return getAtomPayload(*atomInfo, outError);
	else {
		// Don't have AtomInfo
		outError = OI<SError>(sNoAtomInfoError);

		return OI<CData>();
	}
}

//----------------------------------------------------------------------------------------------------------------------
OI<CData> CAtomMediaSource::getAtomPayload(const AtomInfo& atomInfo, SInt64 offset, OI<SError>& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Retrieve child AtomInfo
	OI<AtomInfo>	childAtomInfo = getAtomInfo(atomInfo, offset, outError);
	ReturnValueIfError(outError, OI<CData>());

	return getAtomPayload(*childAtomInfo, outError);
}

//----------------------------------------------------------------------------------------------------------------------
OI<CAtomMediaSource::AtomInfo> CAtomMediaSource::getAtomInfo(const AtomInfo& atomInfo, SInt64 offset,
		OI<SError>& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Move to
	outError = mByteReader.setPos(CByteReader::kPositionFromBeginning, atomInfo.mPayloadPos + offset);
	ReturnValueIfError(outError, OI<AtomInfo>());

	// Read
	return getAtomInfo(outError);
}

//----------------------------------------------------------------------------------------------------------------------
OI<CAtomMediaSource::AtomGroup> CAtomMediaSource::getAtomGroup(const AtomInfo& groupAtomInfo, OI<SError>& outError)
		const
//----------------------------------------------------------------------------------------------------------------------
{
	// Move to
	outError = mByteReader.setPos(CByteReader::kPositionFromBeginning, groupAtomInfo.mPayloadPos);
	ReturnValueIfError(outError, OI<AtomGroup>());

	// Read Atom
	TNArray<AtomInfo>	atomInfos;
	while (mByteReader.getPos() - (groupAtomInfo.mPayloadPos + groupAtomInfo.mPayloadSize)) {
		// Get atom info
		OI<AtomInfo>	atomInfo = getAtomInfo(outError);
		ReturnValueIfError(outError, OI<AtomGroup>());

		// Add to array
		atomInfos += *atomInfo;

		// Seek
		outError = seekToNextAtom(*atomInfo);
		ReturnValueIfError(outError, OI<AtomGroup>());
	}

	return OI<AtomGroup>(new AtomGroup(atomInfos));
}

//----------------------------------------------------------------------------------------------------------------------
OI<CAtomMediaSource::AtomGroup> CAtomMediaSource::getAtomGroup(const OR<AtomInfo>& atomInfo, OI<SError>& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if have AtomInfo
	if (atomInfo.hasReference())
		// Have AtomInfo
		return getAtomGroup(*atomInfo, outError);
	else {
		// Don't have AtomInfo
		outError = OI<SError>(sNoAtomInfoError);

		return OI<AtomGroup>();
	}
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CAtomMediaSource::seekToNextAtom(const AtomInfo& atomInfo) const
//----------------------------------------------------------------------------------------------------------------------
{
	return mByteReader.setPos(CByteReader::kPositionFromBeginning, atomInfo.mPayloadPos + atomInfo.mPayloadSize);
}
