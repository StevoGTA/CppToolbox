//----------------------------------------------------------------------------------------------------------------------
//	CFolder-POSIX.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFolder.h"

#include <sys/stat.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: Macros

#define	CFolderReportErrorAndReturnError(error, message)									\
				{																			\
					CLogServices::logError(error, message, __FILE__, __func__, __LINE__);	\
					logAsError(CString::mSpaceX4);											\
																							\
					return error;															\
				}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFolder

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
UError CFolder::rename(const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	// Compose new filesystem path
	CFilesystemPath	filesystemPath = getFilesystemPath().deletingLastComponent().appendingComponent(string);

	// Rename
	if (::rename(*getFilesystemPath().getString().getCString(kStringEncodingUTF8),
			*filesystemPath.getString().getCString(kStringEncodingUTF8)) == 0) {
		// Success
		update(filesystemPath);

		return kNoError;
	} else
		// Error
		CFolderReportErrorAndReturnError(MAKE_UError(kPOSIXErrorDomain, errno), "renaming");
}

//----------------------------------------------------------------------------------------------------------------------
UError CFolder::create() const
//----------------------------------------------------------------------------------------------------------------------
{
	if (::mkdir(*getFilesystemPath().getString().getCString(kStringEncodingUTF8), 0777) == 0)
		// Success
		return kNoError;
	else
		// Error
		CFolderReportErrorAndReturnError(MAKE_UError(kPOSIXErrorDomain, errno), "creating");
}

//----------------------------------------------------------------------------------------------------------------------
UError CFolder::remove() const
//----------------------------------------------------------------------------------------------------------------------
{
	if (::unlink(*getFilesystemPath().getString().getCString(kStringEncodingUTF8)) == 0)
		// Success
		return kNoError;
	else
		// Error
		CFolderReportErrorAndReturnError(MAKE_UError(kPOSIXErrorDomain, errno), "removing");
}

//----------------------------------------------------------------------------------------------------------------------
bool CFolder::doesExist() const
//----------------------------------------------------------------------------------------------------------------------
{
	struct	stat	statInfo;

	return (::stat(*getFilesystemPath().getString().getCString(kStringEncodingUTF8), &statInfo) == 0) &&
			S_ISDIR(statInfo.st_mode);
}
