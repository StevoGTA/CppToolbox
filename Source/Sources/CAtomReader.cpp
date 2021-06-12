//----------------------------------------------------------------------------------------------------------------------
//	CAtomReader.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CAtomReader.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CString	sAtomMediaReaderErrorDomain(OSSTR("CAtomReader"));
static	SError	sNoAtomInfoError(sAtomMediaReaderErrorDomain, 1, CString(OSSTR("No Atom Info")));

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAtomReader

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
TIResult<CAtomReader::AtomInfo> CAtomReader::readAtomInfo() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read size as UInt32
	TVResult<UInt32>	size32 = readUInt32();
	ReturnValueIfError(size32.getError(), TIResult<CAtomReader::AtomInfo>(*size32.getError()));

	// Read type as OSType
	TVResult<OSType>	type = readOSType();
	ReturnValueIfError(type.getError(), TIResult<CAtomReader::AtomInfo>(*type.getError()));

	// Do we need to read large size?
	UInt64	payloadSize;
	if (*size32.getValue() == 1) {
		// Yes
		TVResult<UInt64>	size64 = readUInt64();
		ReturnValueIfError(size64.getError(), TIResult<CAtomReader::AtomInfo>(*size64.getError()));

		payloadSize = *size64.getValue() - 12;
	} else
		// No
		payloadSize = *size32.getValue() - 8;

	return TIResult<AtomInfo>(AtomInfo(*type.getValue(), getPos(), payloadSize));
}

//----------------------------------------------------------------------------------------------------------------------
TIResult<CAtomReader::AtomInfo> CAtomReader::readAtomInfo(const AtomInfo& atomInfo, SInt64 offset) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Move to
	OI<SError>	error = setPos(kPositionFromBeginning, atomInfo.mPayloadPos + offset);
	ReturnValueIfError(error, TIResult<CAtomReader::AtomInfo>(*error));

	return readAtomInfo();
}

//----------------------------------------------------------------------------------------------------------------------
TIResult<CData> CAtomReader::readAtomPayload(const AtomInfo& atomInfo) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Move to
	OI<SError>	error = setPos(kPositionFromBeginning, atomInfo.mPayloadPos);
	ReturnValueIfError(error, TIResult<CData>(*error));

	return readData(atomInfo.mPayloadSize);
}

//----------------------------------------------------------------------------------------------------------------------
TIResult<CData> CAtomReader::readAtomPayload(const OR<AtomInfo>& atomInfo) const
//----------------------------------------------------------------------------------------------------------------------
{
	return atomInfo.hasReference() ? readAtomPayload(*atomInfo) : TIResult<CData>(sNoAtomInfoError);
}

//----------------------------------------------------------------------------------------------------------------------
TIResult<CData> CAtomReader::readAtomPayload(const AtomInfo& atomInfo, SInt64 offset) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Retrieve child AtomInfo
	TIResult<AtomInfo>	childAtomInfo = readAtomInfo(atomInfo, offset);
	ReturnValueIfError(childAtomInfo.getError(), TIResult<CData>(*childAtomInfo.getError()));

	return readAtomPayload(*childAtomInfo.getValue());
}

//----------------------------------------------------------------------------------------------------------------------
TIResult<CAtomReader::AtomGroup> CAtomReader::readAtomGroup(const AtomInfo& groupAtomInfo) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Move to
	OI<SError>	error = setPos(kPositionFromBeginning, groupAtomInfo.mPayloadPos);
	ReturnValueIfError(error, TIResult<AtomGroup>(*error));

	// Read Atom
	TNArray<AtomInfo>	atomInfos;
	while (getPos() - (groupAtomInfo.mPayloadPos + groupAtomInfo.mPayloadSize)) {
		// Get atom info
		TIResult<AtomInfo>	atomInfo = readAtomInfo();
		ReturnValueIfError(atomInfo.getError(), TIResult<AtomGroup>(*atomInfo.getError()));

		// Add to array
		atomInfos += *atomInfo.getValue();

		// Seek
		error = seekToNextAtom(*atomInfo.getValue());
		ReturnValueIfError(error, TIResult<AtomGroup>(*error));
	}

	return TIResult<AtomGroup>(AtomGroup(atomInfos));
}

//----------------------------------------------------------------------------------------------------------------------
TIResult<CAtomReader::AtomGroup> CAtomReader::readAtomGroup(const OR<AtomInfo>& atomInfo) const
//----------------------------------------------------------------------------------------------------------------------
{
	return atomInfo.hasReference() ? readAtomGroup(*atomInfo) : TIResult<AtomGroup>(sNoAtomInfoError);
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CAtomReader::seekToNextAtom(const AtomInfo& atomInfo) const
//----------------------------------------------------------------------------------------------------------------------
{
	return setPos(kPositionFromBeginning, atomInfo.mPayloadPos + atomInfo.mPayloadSize);
}
