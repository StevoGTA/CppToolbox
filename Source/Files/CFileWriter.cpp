//----------------------------------------------------------------------------------------------------------------------
//	CFileWriter.cpp			©2023 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFileWriter.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CFileWriter

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFileWriter::write(const CFile& file, const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFileWriter	fileWriter(file);

	// Open
	OV<SError>	error = fileWriter.open(false, false, true);
	ReturnErrorIfError(error);

	// Write
	error = fileWriter.write(data);
	ReturnErrorIfError(error);

	return fileWriter.close();
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAtomFileWriter

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAtomFileWriter::CAtomFileWriter(const CFile& file) : CFileWriter(file)
//----------------------------------------------------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------------------------------------------------
CAtomFileWriter::CAtomFileWriter(const CAtomFileWriter& other) : CFileWriter(other)
//----------------------------------------------------------------------------------------------------------------------
{
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CAtomFileWriter::writeAtom(OSType id, const CData& payload)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	OV<SError>	error;

	// Write header
	error = write(EndianU32_NtoB(sizeof(UInt32) + sizeof(OSType) + (UInt32) payload.getByteCount()));
	ReturnErrorIfError(error);

	error = write(EndianU32_NtoB(id));
	ReturnErrorIfError(error);

	// Write payload
	error = write(payload);
	ReturnErrorIfError(error);

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CAtomFileWriter::writeAtomHeader(OSType id, UInt64 payloadByteCount, bool force64BitByteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	OV<SError>	error;

	// Check payloadByteCount
	UInt64	atomByteCount = payloadByteCount + sizeof(UInt32) + sizeof(OSType);
	if (!force64BitByteCount && (atomByteCount < 0x100000000)) {
		// Atom can use 32-bit byte count
		error = write(EndianU32_NtoB((UInt32) atomByteCount));
		ReturnErrorIfError(error);

		// Write id
		error = write(EndianU32_NtoB(id));
		ReturnErrorIfError(error);
	} else {
		// Atom is too big to use 32-bit byte count or forcing to use large byte count
		error = write(EndianU32_NtoB(1));
		ReturnErrorIfError(error);

		// Write id
		error = write(EndianU32_NtoB(id));
		ReturnErrorIfError(error);

		// Write byte count
		error = write(EndianU64_NtoB(payloadByteCount + sizeof(UInt64) + sizeof(OSType)));
		ReturnErrorIfError(error);
	}

	return OV<SError>();
}
