//----------------------------------------------------------------------------------------------------------------------
//	CAtomReader.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CByteReader.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CAtomReader

class CAtomReader : public CByteReader {
	// Atom
	public:
		struct Atom {
			// Lifecycle methods
			Atom(OSType type, UInt64 payloadPos, UInt64 payloadByteCount) :
				mType(type), mPayloadPos(payloadPos), mPayloadByteCount(payloadByteCount)
				{}
			Atom(const Atom& other) :
				mType(other.mType), mPayloadPos(other.mPayloadPos), mPayloadByteCount(other.mPayloadByteCount)
				{}

			// Properties
			OSType	mType;
			UInt64	mPayloadPos;
			UInt64	mPayloadByteCount;
		};

	// AtomGroup
	public:
		struct ContainerAtom {
										// Lifecycle methods
										ContainerAtom(const TArray<Atom>& atoms) : mAtoms(atoms) {}
										ContainerAtom(const ContainerAtom& other) : mAtoms(other.mAtoms) {}

										// Instance methods
					TIteratorD<Atom>	getIterator() const
											{ return mAtoms.getIterator(); }
					OR<Atom>			getAtom(OSType type) const
											{ return mAtoms.getFirst((TArray<Atom>::IsMatchProc) compareType, &type); }

										// Class methods
			static	bool				compareType(const Atom& atom, OSType* type)
											{ return atom.mType == *type; }

			// Properties
			TArray<Atom>	mAtoms;
		};

	// Methods
	public:
								// Lifecycle methods
								CAtomReader(const I<CRandomAccessDataSource>& randomAccessDataSource) :
									CByteReader(randomAccessDataSource, true)
									{}

								// Instance methods
		TVResult<Atom>			readAtom() const;
		TVResult<Atom>			readAtom(const Atom& atom, SInt64 offset) const;
		TVResult<CData>			readAtomPayload(const Atom& atom) const;
		TVResult<CData>			readAtomPayload(const OR<Atom>& atom) const;
		TVResult<ContainerAtom>	readContainerAtom() const;
		TVResult<ContainerAtom>	readContainerAtom(const Atom& atom) const;
		TVResult<ContainerAtom>	readContainerAtom(const OR<Atom>& atom) const;
		OV<SError>				seekToNextAtom(const Atom& atom) const;
};

