//----------------------------------------------------------------------------------------------------------------------
//	CFolder-Windows-Win32.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFolder.h"

#include "SError-Windows.h"

#include "shlobj.h"

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
	AssertFailUnimplemented();
return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFolder::create() const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFolder::remove() const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
bool CFolder::doesExist() const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return false;
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
