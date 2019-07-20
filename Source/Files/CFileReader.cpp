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

//----------------------------------------------------------------------------------------------------------------------
CString CFileReader::readStringToEOL(UError& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CString	outString;

	bool	foundEnd = false;
	while (!foundEnd) {
		// First, read as much as we can
		UInt64	bytesRead = std::min<UInt64>(1024, getFile().getSize() - getPos());
		if (bytesRead == 0) {
			// EOF
			outError = kFileEOFError;

			return outString;
		}

		char	buffer[bytesRead + 1];
		outError = readData(buffer, bytesRead);
		if (outError != kNoError)
			// Error
			return CString::mEmpty;

		// Add NULL to end of end
		buffer[bytesRead] = 0;

		// Did we actually read anything?
		if (bytesRead > 0) {
			// Prepare for resetting file position
			SInt32	delta = (SInt32) -bytesRead;

			// Go through destBuffer, searching for \r and \n
			char*	p = buffer;
			while ((bytesRead > 0) && (*p != '\r') && (*p != '\n')) {
				p++;
				bytesRead--;
				delta++;
			}

			// Did we find any end of line chars?
			if (bytesRead > 0) {
				// Yes
				foundEnd = true;

				// End string
				*p = 0;
				p++;
				delta++;

				// Skip the rest of the end of line chars we find
				while ((bytesRead > 0) && ((*p == '\r') || (*p == '\n'))) {
					p++;
					bytesRead--;
					delta++;
				}
			}

			// Reset the file's position to the beginning of the next line
			outError = setPos(kFileReaderPositionModeFromCurrent, delta);
			ReturnValueIfError(outError, outString);

			// Append the chars we found to the return string
			outString += CString(buffer);
		} else
			foundEnd = true;
	}

	return outString;
}
