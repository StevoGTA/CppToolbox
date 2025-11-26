//----------------------------------------------------------------------------------------------------------------------
//	CAtomReader.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAtomReader.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CString	sErrorDomain(OSSTR("CAtomReader"));
static	SError	sNoAtomError(sErrorDomain, 1, CString(OSSTR("No Atom")));

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAtomReader

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
TVResult<CAtomReader::Atom> CAtomReader::readAtom() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read byte count as UInt32
	TVResult<UInt32>	byteCount32 = readUInt32();
	ReturnValueIfResultError(byteCount32, TVResult<CAtomReader::Atom>(byteCount32.getError()));

	// Read type as OSType
	TVResult<OSType>	type = readOSType();
	ReturnValueIfResultError(type, TVResult<CAtomReader::Atom>(type.getError()));

	// Do we need to read byte count as UInt64?
	UInt64	payloadByteCount;
	if (*byteCount32 == 1) {
		// Yes
		TVResult<UInt64>	byteCount64 = readUInt64();
		ReturnValueIfResultError(byteCount64, TVResult<CAtomReader::Atom>(byteCount64.getError()));
		payloadByteCount = *byteCount64 - 16;
	} else
		// No
		payloadByteCount = *byteCount32 - 8;

	return TVResult<Atom>(Atom(*type, getPos(), payloadByteCount));
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CAtomReader::Atom> CAtomReader::readAtom(const Atom& atom, SInt64 offset) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Move to
	OV<SError>	error = setPos(kPositionFromBeginning, atom.getPayloadPosition() + offset);
	ReturnValueIfError(error, TVResult<CAtomReader::Atom>(*error));

	return readAtom();
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CData> CAtomReader::readAtomPayload(const Atom& atom) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Move to
	OV<SError>	error = setPos(kPositionFromBeginning, atom.getPayloadPosition());
	ReturnValueIfError(error, TVResult<CData>(*error));

	return readData(atom.getPayloadByteCount());
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CData> CAtomReader::readAtomPayload(const OR<Atom>& atom) const
//----------------------------------------------------------------------------------------------------------------------
{
	return atom.hasReference() ? readAtomPayload(*atom) : TVResult<CData>(sNoAtomError);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CAtomReader::ContainerAtom> CAtomReader::readContainerAtom() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read Atoms
	TNArray<Atom>	atoms;
	while (getPos() < getByteCount()) {
		// Get atom info
		TVResult<Atom>	childAtom = readAtom();
		ReturnValueIfResultError(childAtom, TVResult<ContainerAtom>(childAtom.getError()));

		// Check for terminator atom
		if ((childAtom->getType() == 0) || (childAtom->getPayloadByteCount() == 0))
			// Done
			break;

		// Add to array
		atoms += *childAtom;

		// Seek
		OV<SError>	error = seekToNextAtom(*childAtom);
		ReturnValueIfError(error, TVResult<ContainerAtom>(*error));
	}

	return TVResult<ContainerAtom>(ContainerAtom(atoms));
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CAtomReader::ContainerAtom> CAtomReader::readContainerAtom(const Atom& atom) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Move to
	OV<SError>	error = setPos(kPositionFromBeginning, atom.getPayloadPosition());
	ReturnValueIfError(error, TVResult<ContainerAtom>(*error));

	// Read Atoms
	TNArray<Atom>	atoms;
	while (getPos() < (atom.getPayloadPosition() + atom.getPayloadByteCount())) {
		// Get atom info
		TVResult<Atom>	childAtom = readAtom();
		ReturnValueIfResultError(childAtom, TVResult<ContainerAtom>(childAtom.getError()));

		// Check for terminator atom
		if ((childAtom->getType() == 0) || (childAtom->getPayloadByteCount() == 0))
			// Done
			break;

		// Add to array
		atoms += *childAtom;

		// Seek
		error = seekToNextAtom(*childAtom);
		ReturnValueIfError(error, TVResult<ContainerAtom>(*error));
	}

	return TVResult<ContainerAtom>(ContainerAtom(atoms));
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CAtomReader::ContainerAtom> CAtomReader::readContainerAtom(const OR<Atom>& atom) const
//----------------------------------------------------------------------------------------------------------------------
{
	return atom.hasReference() ? readContainerAtom(*atom) : TVResult<ContainerAtom>(sNoAtomError);
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CAtomReader::seekToNextAtom(const Atom& atom) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if can set pos
	if ((atom.getPayloadPosition() + atom.getPayloadByteCount()) <= getByteCount())
		// Set pos
		return setPos(kPositionFromBeginning, atom.getPayloadPosition() + atom.getPayloadByteCount());
	else
		// Nope
		return OV<SError>(SError::mEndOfData);
}
