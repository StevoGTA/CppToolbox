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
	// Compose new filesystem path
	CFilesystemPath	filesystemPath = getFilesystemPath().deletingLastComponent().appendingComponent(string);

	// Rename
	if (::MoveFile(getFilesystemPath().getString().getOSString(), filesystemPath.getString().getOSString()))
		// Success
		return OV<SError>();
	else {
		// Error
		OV<SError>	error(SErrorFromWindowsGetLastError());
		CLogServices::logError(*error, "renaming file", __FILE__, __func__, __LINE__);

		return error;
	}
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
		LARGE_INTEGER	byteCount = {0};
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
	// Get attributes
	DWORD	attributes = ::GetFileAttributes(getFilesystemPath().getString().getOSString());

	return (attributes & FILE_ATTRIBUTE_READONLY) != 0;
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFile::setLocked(bool lockFile) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get attributes
	DWORD	attributes = ::GetFileAttributes(getFilesystemPath().getString().getOSString());
	if (attributes == INVALID_FILE_ATTRIBUTES) {
		// Error
		OV<SError>	error(SErrorFromWindowsGetLastError());
		CLogServices::logError(*error, "setting file locked", __FILE__, __func__, __LINE__);

		return error;
	}

	// Update
	if (lockFile)
		// Locking
		attributes |= FILE_ATTRIBUTE_READONLY;
	else
		// Unlocking
		attributes &= ~FILE_ATTRIBUTE_READONLY;

	// Set attributes
	if (::SetFileAttributes(getFilesystemPath().getString().getOSString(), attributes))
		// Success
		return OV<SError>();
	else {
		OV<SError>	error(SErrorFromWindowsGetLastError());
		CLogServices::logError(*error, "setting file locked", __FILE__, __func__, __LINE__);

		return error;
	}
}

//----------------------------------------------------------------------------------------------------------------------
UniversalTime CFile::getCreationUniversalTime() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Query info
	WIN32_FILE_ATTRIBUTE_DATA	fileAttributeData;
	if (::GetFileAttributesEx(getFilesystemPath().getString().getOSString(), GetFileExInfoStandard,
			&fileAttributeData)) {
		// Handle results
		ULARGE_INTEGER	dateTime = {0};
		dateTime.LowPart = fileAttributeData.ftCreationTime.dwLowDateTime;
		dateTime.HighPart = fileAttributeData.ftCreationTime.dwHighDateTime;

		return (UniversalTime) dateTime.QuadPart / (UniversalTime) 10000000ULL - 12622780800LL;
	} else {
		// Error
		CFileReportError(SErrorFromWindowsGetLastError(), "GetFileAttributexEx");

		return 0;
	}
}

//----------------------------------------------------------------------------------------------------------------------
UniversalTime CFile::getModificationUniversalTime() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Query info
	WIN32_FILE_ATTRIBUTE_DATA	fileAttributeData;
	if (::GetFileAttributesEx(getFilesystemPath().getString().getOSString(), GetFileExInfoStandard,
			&fileAttributeData)) {
		// Handle results
		ULARGE_INTEGER	dateTime = {0};
		dateTime.LowPart = fileAttributeData.ftLastWriteTime.dwLowDateTime;
		dateTime.HighPart = fileAttributeData.ftLastWriteTime.dwHighDateTime;

		return (UniversalTime)dateTime.QuadPart / (UniversalTime)10000000ULL - 12622780800LL;
	} else {
		// Error
		CFileReportError(SErrorFromWindowsGetLastError(), "GetFileAttributexEx");

		return 0;
	}
}
