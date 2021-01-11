//----------------------------------------------------------------------------------------------------------------------
//	CMediaSource.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CMediaSource.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CChunkMediaSourceInternals

class CChunkMediaSourceInternals {
	public:
		CChunkMediaSourceInternals(CChunkMediaSource::Type type) : mType(type) {}

		CChunkMediaSource::Type	mType;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CChunkMediaSource

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CChunkMediaSource::CChunkMediaSource(const CByteParceller& byteParceller, Type type) : CMediaSource(byteParceller)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CChunkMediaSourceInternals(type);
}

//----------------------------------------------------------------------------------------------------------------------
CChunkMediaSource::~CChunkMediaSource()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
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
		struct SHeader { OSType	mID; UInt32	mSize; } header;
		outError = mByteParceller.readData(&header, sizeof(SHeader));
		ReturnValueIfError(outError, OI<ChunkInfo>());

		id = EndianU32_BtoN(header.mID);
		size = (mInternals->mType == kBigEndian) ? EndianU32_BtoN(header.mSize) : EndianU32_LtoN(header.mSize);
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

// Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAtomMediaSource::CAtomMediaSource(const CByteParceller& byteParceller) : CMediaSource(byteParceller)
//----------------------------------------------------------------------------------------------------------------------
{
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
	if (EndianU32_BtoN(*size32) == 1) {
		// Yes
		OV<UInt64>	size64 = mByteParceller.readUInt64(outError);
		ReturnValueIfError(outError, OI<AtomInfo>());

		payloadSize = EndianU64_BtoN(*size64) - 12;
	} else
		// No
		payloadSize = EndianU32_BtoN(*size32) - 8;

	return OI<AtomInfo>(new AtomInfo(EndianU32_BtoN(*type), mByteParceller.getPos(), payloadSize));
}

//----------------------------------------------------------------------------------------------------------------------
OI<CData> CAtomMediaSource::getAtomPayload(const AtomInfo& atomInfo, OI<SError>& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Move to
	OI<SError>	error = mByteParceller.setPos(CDataSource::kPositionFromBeginning, atomInfo.mPayloadPos);
	ReturnValueIfError(outError, OI<CData>());

	return mByteParceller.readData(atomInfo.mPayloadSize, outError);
}

//----------------------------------------------------------------------------------------------------------------------
OI<CAtomMediaSource::AtomGroup> CAtomMediaSource::getAtomGroup(const AtomInfo& groupAtomInfo, OI<SError>& outError)
		const
//----------------------------------------------------------------------------------------------------------------------
{
	// Move to
	OI<SError>	error = mByteParceller.setPos(CDataSource::kPositionFromBeginning, groupAtomInfo.mPayloadPos);
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
OI<SError> CAtomMediaSource::seekToNextAtom(const AtomInfo& atomInfo) const
//----------------------------------------------------------------------------------------------------------------------
{
	return mByteParceller.setPos(CDataSource::kPositionFromBeginning, atomInfo.mPayloadPos + atomInfo.mPayloadSize);
}
