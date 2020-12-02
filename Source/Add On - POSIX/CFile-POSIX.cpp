//----------------------------------------------------------------------------------------------------------------------
//	CFile-POSIX.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFile.h"

#include "SError-POSIX.h"

#include <sys/stat.h>

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
OI<SError> CFile::rename(const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	// Compose new filesystem path
	CFilesystemPath	filesystemPath = getFilesystemPath().deletingLastComponent().appendingComponent(string);

	// Rename
	if (::rename(*getFilesystemPath().getString().getCString(kStringEncodingUTF8),
			*filesystemPath.getString().getCString(kStringEncodingUTF8)) == 0) {
		// Success
		update(filesystemPath);

		return OI<SError>();
	} else
		// Error
		CFileReportErrorAndReturnError(SErrorFromPOSIXerror(errno), "renaming file");
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CFile::getSize() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get size
	struct	stat	statInfo;
	if (::stat(*getFilesystemPath().getString().getCString(kStringEncodingUTF8), &statInfo) == 0)
		// Success
		return statInfo.st_size;
	else
		// Error
		CFileReportErrorAndReturnValue(SErrorFromPOSIXerror(errno), "getting size", 0);
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CFile::remove() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove
	if (::unlink(*getFilesystemPath().getString().getCString(kStringEncodingUTF8)) == 0)
		// Success
		return OI<SError>();
	else
		// Error
		CFileReportErrorAndReturnError(SErrorFromPOSIXerror(errno), "removing file");
}

//----------------------------------------------------------------------------------------------------------------------
bool CFile::doesExist() const
//----------------------------------------------------------------------------------------------------------------------
{
	return ::access(*getFilesystemPath().getString().getCString(kStringEncodingUTF8), F_OK) != -1;
}

//----------------------------------------------------------------------------------------------------------------------
bool CFile::getLocked() const
//----------------------------------------------------------------------------------------------------------------------
{
	struct	stat	statInfo;

	return (::stat(*getFilesystemPath().getString().getCString(kStringEncodingUTF8), &statInfo) == 0) &&
			((statInfo.st_flags & UF_IMMUTABLE) != 0);
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CFile::setLocked(bool lockFile) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get flags
	struct	stat	statInfo;
	if (::stat(*getFilesystemPath().getString().getCString(kStringEncodingUTF8), &statInfo) != 0)
		// Error
		CFileReportErrorAndReturnError(SErrorFromPOSIXerror(errno), "getting flags when setting locked");

	// Update flags
	statInfo.st_flags = lockFile ? (statInfo.st_flags | UF_IMMUTABLE) : (statInfo.st_flags & ~UF_IMMUTABLE);
	if (::chflags(*getFilesystemPath().getString().getCString(kStringEncodingUTF8), statInfo.st_flags) != 0)
		// Error
		CFileReportErrorAndReturnError(SErrorFromPOSIXerror(errno), "setting locked");

	return OI<SError>();
}

#if TARGET_OS_MACOS || TARGET_OS_LINUX
//----------------------------------------------------------------------------------------------------------------------
UInt16 CFile::getPermissions() const
//----------------------------------------------------------------------------------------------------------------------
{
	struct	stat	statInfo;

	return (::stat(*getFilesystemPath().getString().getCString(kStringEncodingUTF8), &statInfo) == 0) ?
			statInfo.st_mode : 0;
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CFile::setPermissions(UInt16 permissions) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Set permissions
	if (::chmod(*getFilesystemPath().getString().getCString(kStringEncodingUTF8), permissions) == 0)
		// Succes
		return OI<SError>();
	else
		// Error
		CFileReportErrorAndReturnError(SErrorFromPOSIXerror(errno), "setting permissions");
}
#endif
