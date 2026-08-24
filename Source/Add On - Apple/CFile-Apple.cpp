//----------------------------------------------------------------------------------------------------------------------
//	CFile-Apple.cpp			©2019 Stevo Brock	All rights reserved.
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

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFile

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
bool CFile::getLocked() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
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
		CFileReportErrorAndReturnError(SErrorFromPOSIXerror(errno),
				CString(OSSTR("getting flags when setting locked")));

	// Update flags
	statInfo.st_flags = lockFile ? (statInfo.st_flags | UF_IMMUTABLE) : (statInfo.st_flags & ~UF_IMMUTABLE);
	if (::chflags(*getFilesystemPath().getString().getUTF8String(), statInfo.st_flags) != 0)
		// Error
		CFileReportErrorAndReturnError(SErrorFromPOSIXerror(errno), CString(OSSTR("setting locked")));

	return OV<SError>();
}
