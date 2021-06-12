//----------------------------------------------------------------------------------------------------------------------
//	CAtomReader.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CByteReader.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAtomReader

class CAtomReader : public CByteReader {
	// AtomInfo
	public:
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

	// AtomGroup
	public:
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
	public:
							// Lifecycle methods
							CAtomReader(const I<CSeekableDataSource>& seekableDataSource) :
								CByteReader(seekableDataSource, true)
								{}

							// Instance methods
		TIResult<AtomInfo>	readAtomInfo() const;
		TIResult<AtomInfo>	readAtomInfo(const AtomInfo& atomInfo, SInt64 offset) const;
		TIResult<CData>		readAtomPayload(const AtomInfo& atomInfo) const;
		TIResult<CData>		readAtomPayload(const OR<AtomInfo>& atomInfo) const;
		TIResult<CData>		readAtomPayload(const AtomInfo& atomInfo, SInt64 offset) const;
		TIResult<AtomGroup>	readAtomGroup(const AtomInfo& groupAtomInfo) const;
		TIResult<AtomGroup>	readAtomGroup(const OR<AtomInfo>& atomInfo) const;
		OI<SError>			seekToNextAtom(const AtomInfo& atomInfo) const;
};
