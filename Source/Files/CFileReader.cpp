//----------------------------------------------------------------------------------------------------------------------
//	CFileReader.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFileReader.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CFileReader

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CData CFileReader::readData(UInt64 byteCount, UError& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	CData	data((CDataSize) byteCount);
	outError = readData(data.getMutableBytePtr(), byteCount);

	return (outError == kNoError) ? data : CData::mEmpty;
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CData CFileReader::readData(const CFile& file, UError& outError)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFileReader	fileReader(file);

	// Open
	outError = fileReader.open();
	ReturnValueIfError(outError, CData::mEmpty);

	// Read
	CData	data = fileReader.readData(file.getSize(), outError);
	ReturnValueIfError(outError, CData::mEmpty);

	// Close
	outError = fileReader.close();
	ReturnValueIfError(outError, CData::mEmpty);

	return data;
}
