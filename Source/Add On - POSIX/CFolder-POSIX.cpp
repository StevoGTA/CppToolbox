//----------------------------------------------------------------------------------------------------------------------
//	CFolder-POSIX.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFolder.h"

#include "SError-POSIX.h"

#include <sys/stat.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: Macros

#define	CFolderReportErrorAndReturnError(error, message)											\
				{																					\
					CLogServices::logError(error, message,											\
							CString(__FILE__, sizeof(__FILE__), CString::kEncodingUTF8),			\
							CString(__func__, sizeof(__func__), CString::kEncodingUTF8), __LINE__);	\
					logAsError(CString::mSpaceX4);													\
																									\
					return error;																	\
				}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFolder

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFolder::rename(const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	// Compose new filesystem path
	CFilesystemPath	filesystemPath = getFilesystemPath().deletingLastComponent().appendingComponent(string);

	// Rename
	if (::rename(*getFilesystemPath().getString().getUTF8String(), *filesystemPath.getString().getUTF8String()) != 0)
		// Error
		CFolderReportErrorAndReturnError(SErrorFromPOSIXerror(errno), CString(OSSTR("renaming")));

	// Update
	update(filesystemPath);

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFolder::create(bool createIntermediateFolders) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if exists
	if (doesExist())
		// Exists
		return OV<SError>();

	// Check if creating intermediate folders
	if (createIntermediateFolders) {
		// Create parent
		OV<SError>	error = getParentFolder().create(true);
		ReturnErrorIfError(error);
	}

	// Create
	if (::mkdir(*getFilesystemPath().getString().getUTF8String(), 0777) != 0)
		// Error
		CFolderReportErrorAndReturnError(SErrorFromPOSIXerror(errno), CString(OSSTR("creating")));

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFolder::remove() const
//----------------------------------------------------------------------------------------------------------------------
{
	if (::unlink(*getFilesystemPath().getString().getUTF8String()) != 0)
		// Error
		CFolderReportErrorAndReturnError(SErrorFromPOSIXerror(errno), CString(OSSTR("removing")));

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
bool CFolder::doesExist() const
//----------------------------------------------------------------------------------------------------------------------
{
	struct	stat	statInfo;

	return (::stat(*getFilesystemPath().getString().getUTF8String(), &statInfo) == 0) && S_ISDIR(statInfo.st_mode);
}
