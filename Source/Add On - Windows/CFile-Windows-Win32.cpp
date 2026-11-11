//----------------------------------------------------------------------------------------------------------------------
//	CFile-Windows-Win32.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFile.h"

#include "CLogServices.h"
#include "SError-Windows.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Macros

#define	CFileReportError(error, message)																		\
				{																								\
					CLogServices::logError(error, message,														\
							CString(__FILE__, sizeof(__FILE__), CString::kEncodingUTF8),						\
							CString(__func__, sizeof(__func__), CString::kEncodingUTF8), __LINE__);				\
					CLogServices::logError(																		\
							CString::mSpaceX4 + CString(OSSTR("File: ")) + getFilesystemPath().getString());	\
				}
#define	CFileReportErrorAndReturnError(error, message)															\
				{																								\
					CLogServices::logError(error, message,														\
							CString(__FILE__, sizeof(__FILE__), CString::kEncodingUTF8),						\
							CString(__func__, sizeof(__func__), CString::kEncodingUTF8), __LINE__);				\
					CLogServices::logError(																		\
							CString::mSpaceX4 + CString(OSSTR("File: ")) + getFilesystemPath().getString());	\
																												\
					return OV<SError>(error);																				\
				}
#define	CFileReportErrorAndReturnValue(error, message, value)													\
				{																								\
					CLogServices::logError(error, message,														\
							CString(__FILE__, sizeof(__FILE__), CString::kEncodingUTF8),						\
							CString(__func__, sizeof(__func__), CString::kEncodingUTF8), __LINE__);				\
					CLogServices::logError(																		\
							CString::mSpaceX4 + CString(OSSTR("File: ")) + getFilesystemPath().getString());	\
																												\
					return value;																				\
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
	else
		// Error
		CFileReportErrorAndReturnError(SErrorFromWindowsGetLastError(), CString(OSSTR("renaming file")));
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
	} else
		// Error
		CFileReportErrorAndReturnValue(SErrorFromWindowsGetLastError(), CString(OSSTR("GetFileAttributexEx")), 0);
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFile::remove() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove
	if (::DeleteFile(getFilesystemPath().getString().getOSString()))
		// Success
		return OV<SError>();
	else
		// Error
		CFileReportErrorAndReturnError(SErrorFromWindowsGetLastError(), CString(OSSTR("removing file")));
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

	return (attributes != INVALID_FILE_ATTRIBUTES) && ((attributes & FILE_ATTRIBUTE_READONLY) != 0);
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFile::setLocked(bool lockFile) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get attributes
	DWORD	attributes = ::GetFileAttributes(getFilesystemPath().getString().getOSString());
	if (attributes == INVALID_FILE_ATTRIBUTES)
		// Error
		CFileReportErrorAndReturnError(SErrorFromWindowsGetLastError(), CString(OSSTR("setting file locked")));

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
	else
		// Error
		CFileReportErrorAndReturnError(SErrorFromWindowsGetLastError(), CString(OSSTR("setting file locked")));
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
	} else
		// Error
		CFileReportErrorAndReturnValue(SErrorFromWindowsGetLastError(), CString(OSSTR("GetFileAttributexEx")), 0);
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
	} else
		// Error
		CFileReportErrorAndReturnValue(SErrorFromWindowsGetLastError(), CString(OSSTR("GetFileAttributexEx")), 0);
}

//----------------------------------------------------------------------------------------------------------------------
OV<CData> CFile::getExtendedAttributeAsData(const CString& name) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Open the alternate data stream
	HANDLE	handle =
					::CreateFileW((getFilesystemPath().getString() + CString(OSSTR(":")) + name).getOSString(),
							GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE)
		// Not present or can't be read
		return OV<CData>();

	// Read
	OV<CData>		data;
	LARGE_INTEGER	byteCount;
	if (::GetFileSizeEx(handle, &byteCount)) {
		// Read
		CData	buffer((CData::ByteCount) byteCount.QuadPart, 0);
		DWORD	bytesRead;
		if (::ReadFile(handle, *buffer.getMutableBuffer(0, byteCount.QuadPart), (DWORD) byteCount.QuadPart,
				&bytesRead, NULL) && (bytesRead == byteCount.QuadPart))
			// Success
			data.setValue(buffer);
	}

	// Cleanup
	::CloseHandle(handle);

	return data;
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFile::setExtendedAttribute(const CString& name, const CData& data) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Open the alternate data stream
	HANDLE	handle =
					::CreateFileW((getFilesystemPath().getString() + CString(OSSTR(":")) + name).getOSString(),
							GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (handle == INVALID_HANDLE_VALUE)
		// Error
		CFileReportErrorAndReturnError(SErrorFromWindowsGetLastError(),
				CString(OSSTR("opening extended attribute")));

	// Write
	DWORD	bytesWritten;
	if (!::WriteFile(handle, *data.getUInt8Buffer(), (DWORD) data.getByteCount(), &bytesWritten, NULL)) {
		// Error
		SError	error = SErrorFromWindowsGetLastError();
		::CloseHandle(handle);
		CFileReportErrorAndReturnError(error, CString(OSSTR("writing extended attribute")));
	}

	// Cleanup
	::CloseHandle(handle);

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFile::removeExtendedAttribute(const CString& name) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove the alternate data stream
	if (!::DeleteFileW((getFilesystemPath().getString() + CString(OSSTR(":")) + name).getOSString()) &&
			(::GetLastError() != ERROR_FILE_NOT_FOUND))
		// Error
		CFileReportErrorAndReturnError(SErrorFromWindowsGetLastError(),
				CString(OSSTR("removing extended attribute")));

	return OV<SError>();
}
