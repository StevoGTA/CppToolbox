//----------------------------------------------------------------------------------------------------------------------
//	CFolder-Windows-Win32.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "shlobj.h"

#include "CFolder.h"

#include "SError-Windows.h"

#pragma comment(lib, "shell32")

//----------------------------------------------------------------------------------------------------------------------
// MARK: Macros

#define	CFolderReportErrorAndReturnError(error, message)									\
				{																			\
					CLogServices::logError(error, message, __FILE__, __func__, __LINE__);	\
					logAsError(CString::mSpaceX4);											\
																							\
					return OV<SError>(error);												\
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
    if (!::MoveFile(getFilesystemPath().getString().getOSString(), filesystemPath.getString().getOSString()))
		// Failed to rename
		CFolderReportErrorAndReturnError(SErrorFromWindowsError(::GetLastError()),
				CString(OSSTR("Failed to rename folder")));

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
	if (!::CreateDirectory(getFilesystemPath().getString().getOSString(), NULL))
		// Error
		CFolderReportErrorAndReturnError(SErrorFromWindowsError(::GetLastError()),
				CString(OSSTR("Failed to create folder")));

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFolder::remove() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Try to remove directory
	if (!::RemoveDirectory(getFilesystemPath().getString().getOSString())) {
		// Check error
		DWORD	errorCode = ::GetLastError();
		if ((errorCode != ERROR_FILE_NOT_FOUND) && (errorCode != ERROR_PATH_NOT_FOUND))
			// Error
			CFolderReportErrorAndReturnError(SErrorFromWindowsError(errorCode),
					CString(OSSTR("Failed to remove folder")));
	}

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
bool CFolder::doesExist() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get attributes
	DWORD	attributes = ::GetFileAttributes(getFilesystemPath().getString().getOSString());
	if (attributes == INVALID_FILE_ATTRIBUTES)
		// Not found
		return false;

	return (attributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
}

#if WINAPI_FAMILY_PARTITION(WINAPI_PARTITION_DESKTOP)
//----------------------------------------------------------------------------------------------------------------------
const CFolder& CFolder::userDesktop()
//----------------------------------------------------------------------------------------------------------------------
{
	static	CFolder*	sFolder = nil;

	if (sFolder == nil) {
		// Setup
		PWSTR	path = NULL;
		auto	result = ::SHGetKnownFolderPath(FOLDERID_Desktop, KF_FLAG_CREATE, NULL, &path);
		if (SUCCEEDED(result)) {
			// Success
			sFolder = new CFolder(CFilesystemPath(CString(path)));
			::CoTaskMemFree(path);
		} else
			// Error
			LogFailedHRESULT(result, OSSTR("SHGetKnownFolderPath for FOLDERID_LocalAppData"));
	}

	return *sFolder;
}

//----------------------------------------------------------------------------------------------------------------------
const CFolder& CFolder::localApplicationData()
//----------------------------------------------------------------------------------------------------------------------
{
	static	CFolder*	sFolder = nil;

	if (sFolder == nil) {
		// Setup
		PWSTR	path = NULL;
		auto	result = ::SHGetKnownFolderPath(FOLDERID_LocalAppData, KF_FLAG_CREATE, NULL, &path);
		if (SUCCEEDED(result)) {
			// Success
			sFolder = new CFolder(CFilesystemPath(CString(path)));
			::CoTaskMemFree(path);
		} else
			// Error
			LogFailedHRESULT(result, OSSTR("SHGetKnownFolderPath for FOLDERID_LocalAppData"));
	}

	return *sFolder;
}
#endif
