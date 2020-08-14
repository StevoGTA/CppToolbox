//----------------------------------------------------------------------------------------------------------------------
//	CFile-Windows.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFile.h"

#undef Delete
#include <Windows.h>
#define Delete(x)		{ delete x; x = nil; }

//----------------------------------------------------------------------------------------------------------------------
// MARK: Macros

#define	CFileReportErrorAndReturnError(error, message)										\
				{																			\
					CLogServices::logError(error, message, __FILE__, __func__, __LINE__);	\
					logAsError(CString::mSpaceX4);											\
																							\
					return error;															\
				}
#define	CFileReportErrorAndReturnValue(error, message, value)								\
				{																			\
					CLogServices::logError(error, message, __FILE__, __func__, __LINE__);	\
					logAsError(CString::mSpaceX4);											\
																							\
					return value;															\
				}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFile

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
UError CFile::rename(const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return kNoError;
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CFile::getSize() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Query info
	WIN32_FILE_ATTRIBUTE_DATA	fileAttributeData;
	BOOL						result =
										GetFileAttributesEx(getFilesystemPath().getString().getOSString(),
												GetFileExInfoStandard, &fileAttributeData);
	AssertFailIf(!result);

	// Handle results
	LARGE_INTEGER	size;
	size.HighPart = fileAttributeData.nFileSizeHigh;
	size.LowPart = fileAttributeData.nFileSizeLow;

	return size.QuadPart;
}

//----------------------------------------------------------------------------------------------------------------------
UError CFile::remove() const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return kNoError;
}

//----------------------------------------------------------------------------------------------------------------------
bool CFile::doesExist() const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return false;
}

//----------------------------------------------------------------------------------------------------------------------
bool CFile::getLocked() const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return false;
}

//----------------------------------------------------------------------------------------------------------------------
UError CFile::setLocked(bool lockFile) const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return kNoError;
}
