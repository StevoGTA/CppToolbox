//----------------------------------------------------------------------------------------------------------------------
//	CFile-POSIX.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFile.h"

#include "CLogServices.h"
#include "SError-POSIX.h"

#include <sys/stat.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: Macros

#define	CFileReportErrorAndReturnError(error, message)															\
				{																								\
					CLogServices::logError(error, message,														\
							CString(__FILE__, sizeof(__FILE__), CString::kEncodingUTF8),						\
							CString(__func__, sizeof(__func__), CString::kEncodingUTF8), __LINE__);				\
					CLogServices::logError(																		\
							CString::mSpaceX4 + CString(OSSTR("File: ")) + getFilesystemPath().getString());	\
																												\
					return OV<SError>(error);																	\
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
OV<SError> CFile::rename(const CString& name)
//----------------------------------------------------------------------------------------------------------------------
{
	// Compose new filesystem path
	CFilesystemPath	filesystemPath = getFilesystemPath().deletingLastComponent().appendingComponent(name);

	// Rename
	if (::rename(*getFilesystemPath().getString().getUTF8String(), *filesystemPath.getString().getUTF8String()) == 0) {
		// Success
		update(filesystemPath);

		return OV<SError>();
	} else
		// Error
		CFileReportErrorAndReturnError(SErrorFromPOSIXerror(errno), CString(OSSTR("renaming file")));
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CFile::getByteCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get size
	struct	stat	statInfo;
	if (::stat(*getFilesystemPath().getString().getUTF8String(), &statInfo) == 0)
		// Success
		return statInfo.st_size;
	else
		// Error
		CFileReportErrorAndReturnValue(SErrorFromPOSIXerror(errno), CString(OSSTR("getting byte count")), 0);
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFile::remove() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove
	if (::unlink(*getFilesystemPath().getString().getUTF8String()) == 0)
		// Success
		return OV<SError>();
	else
		// Error
		CFileReportErrorAndReturnError(SErrorFromPOSIXerror(errno), CString(OSSTR("removing file")));
}

//----------------------------------------------------------------------------------------------------------------------
bool CFile::doesExist() const
//----------------------------------------------------------------------------------------------------------------------
{
	return ::access(*getFilesystemPath().getString().getUTF8String(), F_OK) != -1;
}

#if defined(TARGET_OS_MACOS) || defined(TARGET_OS_LINUX)
//----------------------------------------------------------------------------------------------------------------------
UInt16 CFile::getPermissions() const
//----------------------------------------------------------------------------------------------------------------------
{
	struct	stat	statInfo;

	return (::stat(*getFilesystemPath().getString().getUTF8String(), &statInfo) == 0) ? statInfo.st_mode : 0;
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFile::setPermissions(UInt16 permissions) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Set permissions
	if (::chmod(*getFilesystemPath().getString().getUTF8String(), permissions) == 0)
		// Succes
		return OV<SError>();
	else
		// Error
		CFileReportErrorAndReturnError(SErrorFromPOSIXerror(errno), CString(OSSTR("setting permissions")));
}
#endif
