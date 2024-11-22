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
OV<SError> CFile::rename(const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	// Compose new filesystem path
	CFilesystemPath	filesystemPath = getFilesystemPath().deletingLastComponent().appendingComponent(string);

	// Rename
	if (::rename(*getFilesystemPath().getString().getUTF8String(), *filesystemPath.getString().getUTF8String()) == 0) {
		// Success
		update(filesystemPath);

		return OV<SError>();
	} else
		// Error
		CFileReportErrorAndReturnError(SErrorFromPOSIXerror(errno), "renaming file");
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
		CFileReportErrorAndReturnValue(SErrorFromPOSIXerror(errno), "getting byte count", 0);
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
		CFileReportErrorAndReturnError(SErrorFromPOSIXerror(errno), "removing file");
}

//----------------------------------------------------------------------------------------------------------------------
bool CFile::doesExist() const
//----------------------------------------------------------------------------------------------------------------------
{
	return ::access(*getFilesystemPath().getString().getUTF8String(), F_OK) != -1;
}

//----------------------------------------------------------------------------------------------------------------------
bool CFile::getLocked() const
//----------------------------------------------------------------------------------------------------------------------
{
	struct	stat	statInfo;

	return (::stat(*getFilesystemPath().getString().getUTF8String(), &statInfo) == 0) &&
			((statInfo.st_flags & UF_IMMUTABLE) != 0);
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFile::setLocked(bool lockFile) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get flags
	struct	stat	statInfo;
	if (::stat(*getFilesystemPath().getString().getUTF8String(), &statInfo) != 0)
		// Error
		CFileReportErrorAndReturnError(SErrorFromPOSIXerror(errno), "getting flags when setting locked");

	// Update flags
	statInfo.st_flags = lockFile ? (statInfo.st_flags | UF_IMMUTABLE) : (statInfo.st_flags & ~UF_IMMUTABLE);
	if (::chflags(*getFilesystemPath().getString().getUTF8String(), statInfo.st_flags) != 0)
		// Error
		CFileReportErrorAndReturnError(SErrorFromPOSIXerror(errno), "setting locked");

	return OV<SError>();
}

#if defined(TARGET_OS_MACOS) || defined(TARGET_OS_LINUX)
//----------------------------------------------------------------------------------------------------------------------
UInt16 CFile::getPermissions() const
//----------------------------------------------------------------------------------------------------------------------
{
	struct	stat	statInfo;

	return (::stat(*getFilesystemPath().getString().getUTF8String(), &statInfo) == 0) ?
			statInfo.st_mode : 0;
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
		CFileReportErrorAndReturnError(SErrorFromPOSIXerror(errno), "setting permissions");
}
#endif
