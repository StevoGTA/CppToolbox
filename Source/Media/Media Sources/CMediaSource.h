//----------------------------------------------------------------------------------------------------------------------
//	CMediaSource.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CArray.h"
#include "CAudioTrack.h"
#include "CVideoTrack.h"
#include "CByteParceller.h"
#include "SError.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaSource

class CMediaSource {
	// Methods
	public:
									// Lifecycle methods
									CMediaSource() {}
		virtual						~CMediaSource() {}

									// Instance methods
		virtual	OI<SError>			loadTracks() = 0;
		virtual	TArray<CAudioTrack>	getAudioTracks() { return TNArray<CAudioTrack>(); }
		virtual	TArray<CVideoTrack>	getVideoTracks() { return TNArray<CVideoTrack>(); }

	protected:
									// Subclass methods
		virtual	OI<SError>			reset() = 0;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CChunkMediaSource

class CChunkMediaSource : public CMediaSource {
	// Structs
	protected:
		struct ChunkInfo {
			// Methods
			ChunkInfo(OSType id, UInt64 size, SInt64 thisChunkPos, SInt64 nextChunkPos) :
				mID(id), mSize(size), mThisChunkPos(thisChunkPos), mNextChunkPos(nextChunkPos)
				{}

			// Properties
			OSType	mID;
			UInt64	mSize;
			SInt64	mThisChunkPos;
			SInt64	mNextChunkPos;
		};

	// Methods
	protected:
						// Lifecycle methods
						CChunkMediaSource(const CByteParceller& byteParceller) : mByteParceller(byteParceller) {}

						// CMediaSource methods
		OI<SError>		reset();

						// Instance methods
		OI<ChunkInfo>	getChunkInfo(OI<SError>& outError) const;
		OI<CData>		getChunk(const ChunkInfo& chunkInfo, OI<SError>& outError) const;
		OI<SError>		seekToNextChunk(const ChunkInfo& chunkInfo) const;

	// Properties
	protected:
		CByteParceller	mByteParceller;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAtomMediaSource

class CAtomMediaSource : public CMediaSource {
	// Structs
	protected:
		struct AtomInfo {
			// Lifecycle methods
			AtomInfo(OSType type, SInt64 payloadPos, UInt64 payloadSize) :
				mType(type), mPayloadPos(payloadPos), mPayloadSize(payloadSize)
				{}
			AtomInfo(const AtomInfo& other) :
				mType(other.mType), mPayloadPos(other.mPayloadPos), mPayloadSize(other.mPayloadSize)
				{}

			// Properties
			OSType	mType;
			SInt64	mPayloadPos;
			UInt64	mPayloadSize;
		};

		struct AtomGroup {
											// Lifecycle methods
											AtomGroup(const TArray<AtomInfo>& atomInfos) : mAtomInfos(atomInfos) {}

											// Instance methods
					TIteratorD<AtomInfo>	getIterator() const
												{ return mAtomInfos.getIterator(); }
					OR<AtomInfo>			getAtomInfo(OSType type)
												{ return mAtomInfos.getFirst(compareType, &type); }

											// Class methods
			static	bool					compareType(const AtomInfo& atomInfo, void* type)
												{ return atomInfo.mType == *((OSType*) type); }

			// Properties
			TArray<AtomInfo>	mAtomInfos;
		};

	// Methods
	protected:
						// Lifecycle methods
						CAtomMediaSource(const CByteParceller& byteParceller) : mByteParceller(byteParceller) {}

						// CMediaSource methods
		OI<SError>		reset();

						// Instance methods
		OI<AtomInfo>	getAtomInfo(OI<SError>& outError) const;
		OI<AtomInfo>	getAtomInfo(const AtomInfo& atomInfo, SInt64 offset, OI<SError>& outError) const;
		OI<CData>		getAtomPayload(const AtomInfo& atomInfo, OI<SError>& outError) const;
		OI<CData>		getAtomPayload(const OR<AtomInfo>& atomInfo, OI<SError>& outError) const;
		OI<CData>		getAtomPayload(const AtomInfo& atomInfo, SInt64 offset, OI<SError>& outError) const;
		OI<AtomGroup>	getAtomGroup(const AtomInfo& atomInfo, OI<SError>& outError) const;
		OI<AtomGroup>	getAtomGroup(const OR<AtomInfo>& atomInfo, OI<SError>& outError) const;
		OI<SError>		seekToNextAtom(const AtomInfo& atomInfo) const;

	// Properties
	protected:
		CByteParceller	mByteParceller;
};
