//----------------------------------------------------------------------------------------------------------------------
//	CFolder-Windows-WinRT.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFolder.h"

#include <winrt/Windows.Storage.h>

using namespace winrt::Windows::Storage;

//----------------------------------------------------------------------------------------------------------------------
// MARK: Macros

//#define	CFolderReportErrorAndReturnError(error, message)									\
//				{																			\
//					CLogServices::logError(error, message, __FILE__, __func__, __LINE__);	\
//					logAsError(CString::mSpaceX4);											\
//																							\
//					return OI<SError>(error);												\
//				}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFolder

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CFolder::rename(const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	// Compose new filesystem path
	CFilesystemPath	filesystemPath = getFilesystemPath().deletingLastComponent().appendingComponent(string);

	// Rename
	AssertFailUnimplemented();
return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CFolder::create() const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CFolder::remove() const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return OI<SError>();
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
const CFolder& CFolder::local()
//----------------------------------------------------------------------------------------------------------------------
{
	static	CFolder*	sFolder = nil;

	if (sFolder == nil)
		// Setup
		sFolder = new CFolder(CFilesystemPath(CString(ApplicationData::Current().LocalFolder().Path().data())));

	return *sFolder;
}

//----------------------------------------------------------------------------------------------------------------------
const CFolder& CFolder::localCache()
//----------------------------------------------------------------------------------------------------------------------
{
	static	CFolder*	sFolder = nil;

	if (sFolder == nil)
		// Setup
		sFolder = new CFolder(CFilesystemPath(CString(ApplicationData::Current().LocalCacheFolder().Path().data())));

	return *sFolder;
}
