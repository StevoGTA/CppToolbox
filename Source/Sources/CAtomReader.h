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
			// Methods
			public:
						// Lifecycle methods
						Atom(OSType type, UInt64 payloadPosition, UInt64 payloadByteCount) :
							mType(type), mPayloadPosition(payloadPosition), mPayloadByteCount(payloadByteCount)
							{}
						Atom(const Atom& other) :
							mType(other.mType), mPayloadPosition(other.mPayloadPosition),
									mPayloadByteCount(other.mPayloadByteCount)
							{}

						// Instance mehtods
				OSType	getType() const
							{ return mType; }
				UInt64	getPayloadPosition() const
							{ return mPayloadPosition; }
				UInt64	getPayloadByteCount() const
							{ return mPayloadByteCount; }

			// Properties
			private:
				OSType	mType;
				UInt64	mPayloadPosition;
				UInt64	mPayloadByteCount;
		};

	// AtomGroup
	public:
		struct ContainerAtom {
			// Methods
			public:
											// Lifecycle methods
											ContainerAtom(const TArray<Atom>& atoms) : mAtoms(atoms) {}
											ContainerAtom(const ContainerAtom& other) : mAtoms(other.mAtoms) {}

											// Instance methods
					TArray<Atom>::Iterator	getIterator() const
												{ return mAtoms.getIterator(); }
					OR<Atom>				getAtom(OSType type) const
												{ return mAtoms.getFirst((TArray<Atom>::IsMatchProc) compareType,
														&type); }

											// Class methods
			static	bool					compareType(const Atom& atom, OSType* type)
												{ return atom.getType() == *type; }

			// Properties
			private:
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

