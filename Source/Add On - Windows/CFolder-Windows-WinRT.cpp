//----------------------------------------------------------------------------------------------------------------------
//	CFolder-Windows-WinRT.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFolder.h"

#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.Storage.h>

using namespace winrt::Windows::ApplicationModel;
using namespace winrt::Windows::Storage;

//----------------------------------------------------------------------------------------------------------------------
// MARK: CFolder

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
OV<SError> CFolder::create(bool createIntermediateFolders) const
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

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
const CFolder& CFolder::applicationFolder()
//----------------------------------------------------------------------------------------------------------------------
{
	static	CFolder*	sFolder = nil;

	if (sFolder == nil)
		// Setup
		sFolder = new CFolder(CFilesystemPath(CString(Package::Current().InstalledLocation().Path().data())));

	return *sFolder;
}
