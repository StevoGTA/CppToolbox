//----------------------------------------------------------------------------------------------------------------------
//	CFolder-Windows.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFolder.h"

#undef Delete
#include <Windows.h>
#define Delete(x)		{ delete x; x = nil; }

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
	AssertFailUnimplemented();
return kNoError;
}

//----------------------------------------------------------------------------------------------------------------------
UError CFolder::create() const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return kNoError;
}

//----------------------------------------------------------------------------------------------------------------------
UError CFolder::remove() const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return kNoError;
}

//----------------------------------------------------------------------------------------------------------------------
bool CFolder::doesExist() const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return false;
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
const CFolder& CFolder::currentFolder()
//----------------------------------------------------------------------------------------------------------------------
{
	static	CFolder*	sFolder = nil;

	if (sFolder == nil) {
		// Setup
		TCHAR	directory[MAX_PATH];
		GetCurrentDirectory(MAX_PATH, directory);

		sFolder = new CFolder(CFilesystemPath(CString(directory)));
	}

	return *sFolder;
}
