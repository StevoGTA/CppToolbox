//----------------------------------------------------------------------------------------------------------------------
//	CFile-Windows-Win32.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFile.h"

#include "SError-Windows.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Macros

#define	CFileReportError(error, message)													\
				{																			\
					CLogServices::logError(error, message, __FILE__, __func__, __LINE__);	\
					logAsError(CString::mSpaceX4);											\
				}
#define	CFileReportErrorAndReturnError(error, message)										\
				{																			\
					CLogServices::logError(error, message, __FILE__, __func__, __LINE__);	\
					logAsError(CString::mSpaceX4);											\
																							\
					return OV<SError>(error);												\
				}
#define	CFileReportErrorAndReturnValue(error, message, value)								\
				{																			\
					CLogServices::logError(error, message, __FILE__, __func__, __LINE__);	\
					logAsError(CString::mSpaceX4);											\
																							\
					return OV<SError>(value);												\
				}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFile

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFile::rename(const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CFile::getByteCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Query info
	WIN32_FILE_ATTRIBUTE_DATA	fileAttributeData;
	if (::GetFileAttributesEx(getFilesystemPath().getString().getOSString(), GetFileExInfoStandard,
			&fileAttributeData)) {
		// Handle results
		LARGE_INTEGER	byteCount;
		byteCount.HighPart = fileAttributeData.nFileSizeHigh;
		byteCount.LowPart = fileAttributeData.nFileSizeLow;

		return byteCount.QuadPart;
	} else {
		// Error
		CFileReportError(SErrorFromWindowsGetLastError(), "GetFileAttributexEx");

		return 0;
	}
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFile::remove() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove
	if (::DeleteFile(getFilesystemPath().getString().getOSString()))
		// Success
		return OV<SError>();
	else {
		// Error
		OV<SError>	error(SErrorFromWindowsGetLastError());
		CLogServices::logError(*error, "removing file", __FILE__, __func__, __LINE__);

		return error;
	}
}

//----------------------------------------------------------------------------------------------------------------------
bool CFile::doesExist() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get attributes
	DWORD	attributes = ::GetFileAttributes(getFilesystemPath().getString().getOSString());

	return attributes != INVALID_FILE_ATTRIBUTES;
}

//----------------------------------------------------------------------------------------------------------------------
bool CFile::isHidden() const
//----------------------------------------------------------------------------------------------------------------------
{
	return getName().hasPrefix(CString(OSSTR(".")));
}

//----------------------------------------------------------------------------------------------------------------------
bool CFile::getLocked() const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return false;
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFile::setLocked(bool lockFile) const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
UniversalTime CFile::getCreationUniversalTime() const
//----------------------------------------------------------------------------------------------------------------------
{
return 0;
}

//----------------------------------------------------------------------------------------------------------------------
UniversalTime CFile::getModificationUniversalTime() const
{
return 0;
}
