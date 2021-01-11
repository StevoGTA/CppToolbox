//----------------------------------------------------------------------------------------------------------------------
//	CMediaSource.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CArray.h"
#include "CAudioTrack.h"
#include "CByteParceller.h"
#include "SError.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMediaSource

class CMediaSource {
	// Methods
	public:
									// Lifecycle methods
									CMediaSource(const CByteParceller& byteParceller) : mByteParceller(byteParceller) {}
		virtual						~CMediaSource() {}

									// Instance methods
		virtual	OI<SError>			loadTracks() = 0;
		virtual	TArray<CAudioTrack>	getAudioTracks() { return TNArray<CAudioTrack>(); }

	protected:
									// Subclass methods
				OI<SError>			reset()
										{ return mByteParceller.setPos(CDataSource::kPositionFromBeginning, 0); }

	// Properties
	protected:
		const	CByteParceller&	mByteParceller;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CChunkMediaSource

class CChunkMediaSourceInternals;
class CChunkMediaSource : public CMediaSource {
	// Types
	public:
		enum Type {
			kBigEndian,
			kLittleEndian,
		};

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
						CChunkMediaSource(const CByteParceller& byteParceller, Type type);
						~CChunkMediaSource();

						// Instance methods
		OI<ChunkInfo>	getChunkInfo(OI<SError>& outError) const;
		OI<CData>		getChunk(const ChunkInfo& chunkInfo, OI<SError>& outError) const;
		OI<SError>		seekToNextChunk(const ChunkInfo& chunkInfo) const;

	// Properties
	private:
		CChunkMediaSourceInternals*	mInternals;
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
						CAtomMediaSource(const CByteParceller& byteParceller);

						// Instance methods
		OI<AtomInfo>	getAtomInfo(OI<SError>& outError) const;
		OI<CData>		getAtomPayload(const AtomInfo& atomInfo, OI<SError>& outError) const;
		OI<AtomGroup>	getAtomGroup(const AtomInfo& atomInfo, OI<SError>& outError) const;
		OI<SError>		seekToNextAtom(const AtomInfo& atomInfo) const;
};
