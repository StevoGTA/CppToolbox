//----------------------------------------------------------------------------------------------------------------------
//	CAtomReader.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAtomReader.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CString	sAtomMediaReaderErrorDomain(OSSTR("CAtomReader"));
static	SError	sNoAtomError(sAtomMediaReaderErrorDomain, 1, CString(OSSTR("No Atom")));

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAtomReader

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
TIResult<CAtomReader::Atom> CAtomReader::readAtom() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read byte count as UInt32
	TVResult<UInt32>	byteCount32 = readUInt32();
	ReturnValueIfResultError(byteCount32, TIResult<CAtomReader::Atom>(byteCount32.getError()));

	// Read type as OSType
	TVResult<OSType>	type = readOSType();
	ReturnValueIfResultError(type, TIResult<CAtomReader::Atom>(type.getError()));

	// Do we need to read byte count as UInt64?
	UInt64	payloadByteCount;
	if (byteCount32.getValue() == 1) {
		// Yes
		TVResult<UInt64>	byteCount64 = readUInt64();
		ReturnValueIfResultError(byteCount64, TIResult<CAtomReader::Atom>(byteCount64.getError()));
		payloadByteCount = byteCount64.getValue() - 16;
	} else
		// No
		payloadByteCount = byteCount32.getValue() - 8;

	return TIResult<Atom>(Atom(type.getValue(), getPos(), payloadByteCount));
}

//----------------------------------------------------------------------------------------------------------------------
TIResult<CAtomReader::Atom> CAtomReader::readAtom(const Atom& atom, SInt64 offset) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Move to
	OI<SError>	error = setPos(kPositionFromBeginning, atom.mPayloadPos + offset);
	ReturnValueIfError(error, TIResult<CAtomReader::Atom>(*error));

	return readAtom();
}

//----------------------------------------------------------------------------------------------------------------------
TIResult<CData> CAtomReader::readAtomPayload(const Atom& atom) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Move to
	OI<SError>	error = setPos(kPositionFromBeginning, atom.mPayloadPos);
	ReturnValueIfError(error, TIResult<CData>(*error));

	return readData(atom.mPayloadByteCount);
}

//----------------------------------------------------------------------------------------------------------------------
TIResult<CData> CAtomReader::readAtomPayload(const OR<Atom>& atom) const
//----------------------------------------------------------------------------------------------------------------------
{
	return atom.hasReference() ? readAtomPayload(*atom) : TIResult<CData>(sNoAtomError);
}

//----------------------------------------------------------------------------------------------------------------------
TIResult<CData> CAtomReader::readAtomPayload(const Atom& atom, SInt64 offset) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Retrieve child Atom
	TIResult<Atom>	childAtom = readAtom(atom, offset);
	ReturnValueIfResultError(childAtom, TIResult<CData>(childAtom.getError()));

	return readAtomPayload(childAtom.getValue());
}

//----------------------------------------------------------------------------------------------------------------------
TIResult<CAtomReader::ContainerAtom> CAtomReader::readContainerAtom(const Atom& atom) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Move to
	OI<SError>	error = setPos(kPositionFromBeginning, atom.mPayloadPos);
	ReturnValueIfError(error, TIResult<ContainerAtom>(*error));

	// Read Atom
	TNArray<Atom>	atoms;
	while (getPos() - (atom.mPayloadPos + atom.mPayloadByteCount)) {
		// Get atom info
		TIResult<Atom>	childAtom = readAtom();
		ReturnValueIfResultError(childAtom, TIResult<ContainerAtom>(childAtom.getError()));

		// Add to array
		atoms += childAtom.getValue();

		// Seek
		error = seekToNextAtom(childAtom.getValue());
		ReturnValueIfError(error, TIResult<ContainerAtom>(*error));
	}

	return TIResult<ContainerAtom>(ContainerAtom(atoms));
}

//----------------------------------------------------------------------------------------------------------------------
TIResult<CAtomReader::ContainerAtom> CAtomReader::readContainerAtom(const OR<Atom>& atom) const
//----------------------------------------------------------------------------------------------------------------------
{
	return atom.hasReference() ? readContainerAtom(*atom) : TIResult<ContainerAtom>(sNoAtomError);
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CAtomReader::seekToNextAtom(const Atom& atom) const
//----------------------------------------------------------------------------------------------------------------------
{
	return setPos(kPositionFromBeginning, atom.mPayloadPos + atom.mPayloadByteCount);
}
