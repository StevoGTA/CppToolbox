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
	return mByteParceller.setPos(CDataSource::kPositionFromBeginning, 0);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
OI<CChunkMediaSource::ChunkInfo> CChunkMediaSource::getChunkInfo(OI<SError>& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SInt64	pos = mByteParceller.getPos();

	// Read chunk header
	OSType	id;
	UInt64	size;
	if (true) {
		// Read 32-bit header
		OV<OSType>	_id = mByteParceller.readOSType(outError);
		ReturnValueIfError(outError, OI<ChunkInfo>());
		id = *_id;

		OV<UInt32>	_size = mByteParceller.readUInt32(outError);
		ReturnValueIfError(outError, OI<ChunkInfo>());
		size = *_size;
	}

	return OI<ChunkInfo>(new ChunkInfo(id, size, pos, mByteParceller.getPos() + size + (size % 1)));
}

//----------------------------------------------------------------------------------------------------------------------
OI<CData> CChunkMediaSource::getChunk(const ChunkInfo& chunkInfo, OI<SError>& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Move to begining of chunk
	outError = mByteParceller.setPos(CDataSource::kPositionFromBeginning, chunkInfo.mThisChunkPos);
	ReturnValueIfError(outError, OI<CData>());

	return mByteParceller.readData(chunkInfo.mSize, outError);
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CChunkMediaSource::seekToNextChunk(const ChunkInfo& chunkInfo) const
//----------------------------------------------------------------------------------------------------------------------
{
	return mByteParceller.setPos(CDataSource::kPositionFromBeginning, chunkInfo.mNextChunkPos);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAtomMediaSource

// CMediaSource methods

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CAtomMediaSource::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	return mByteParceller.setPos(CDataSource::kPositionFromBeginning, 0);
}

// Instance methods

//----------------------------------------------------------------------------------------------------------------------
OI<CAtomMediaSource::AtomInfo> CAtomMediaSource::getAtomInfo(OI<SError>& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read size as UInt32
	OV<UInt32>	size32 = mByteParceller.readUInt32(outError);
	ReturnValueIfError(outError, OI<AtomInfo>());

	// Read type as OSType
	OV<OSType>	type = mByteParceller.readOSType(outError);
	ReturnValueIfError(outError, OI<AtomInfo>());

	// Do we need to read large size?
	UInt64	payloadSize;
	if (*size32 == 1) {
		// Yes
		OV<UInt64>	size64 = mByteParceller.readUInt64(outError);
		ReturnValueIfError(outError, OI<AtomInfo>());

		payloadSize = *size64 - 12;
	} else
		// No
		payloadSize = *size32 - 8;

	return OI<AtomInfo>(new AtomInfo(*type, mByteParceller.getPos(), payloadSize));
}

//----------------------------------------------------------------------------------------------------------------------
OI<CData> CAtomMediaSource::getAtomPayload(const AtomInfo& atomInfo, OI<SError>& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Move to
	outError = mByteParceller.setPos(CDataSource::kPositionFromBeginning, atomInfo.mPayloadPos);
	ReturnValueIfError(outError, OI<CData>());

	return mByteParceller.readData(atomInfo.mPayloadSize, outError);
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
	outError = mByteParceller.setPos(CDataSource::kPositionFromBeginning, atomInfo.mPayloadPos + offset);
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
	outError = mByteParceller.setPos(CDataSource::kPositionFromBeginning, groupAtomInfo.mPayloadPos);
	ReturnValueIfError(outError, OI<AtomGroup>());

	// Read Atom
	TNArray<AtomInfo>	atomInfos;
	while (mByteParceller.getPos() - (groupAtomInfo.mPayloadPos + groupAtomInfo.mPayloadSize)) {
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
	return mByteParceller.setPos(CDataSource::kPositionFromBeginning, atomInfo.mPayloadPos + atomInfo.mPayloadSize);
}
