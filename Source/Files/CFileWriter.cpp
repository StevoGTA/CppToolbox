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
