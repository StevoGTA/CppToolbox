//----------------------------------------------------------------------------------------------------------------------
//	CApplication-Windows-WinRT.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CApplication.h"

#include <winrt/Windows.ApplicationModel.h>
#include <winrt/Windows.Storage.h>

using namespace winrt::Windows::ApplicationModel;

//----------------------------------------------------------------------------------------------------------------------
// MARK: CApplication

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
const CString& CApplication::getProductName()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	CString*	sProductNameString = nil;

	// Check if have already set up
	if (sProductNameString == nil)
		// Setup
		sProductNameString = new CString(Package::Current().DisplayName().data());

	return (sProductNameString != nil) ? *sProductNameString : CString::mEmpty;
}

//----------------------------------------------------------------------------------------------------------------------
const CString& CApplication::getVersion()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	CString*	sVersion = nil;

	// Check if have already set up
	if (sVersion == nil) {
		// Get info
		auto	version = Package::Current().Id().Version();

		// Setup version
		sVersion =
				new CString(
						SVersionInfo((UInt8) version.Major, (UInt8) version.Minor, (UInt8) version.Build).getString());
	}

	return *sVersion;
}

//----------------------------------------------------------------------------------------------------------------------
const CString& CApplication::getProductNameAndVersion()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	CString*	sProductNameStringAndVersion = nil;

	// Check if have already set up
	if (sProductNameStringAndVersion == nil)
		// Setup
		sProductNameStringAndVersion = new CString(getProductName() + CString(" ") + getVersion().getString());

	return *sProductNameStringAndVersion;
}

//----------------------------------------------------------------------------------------------------------------------
const CFolder& CApplication::getFolder()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	CFolder*	sFolder = nil;

	// Check if have already set up
	if (sFolder == nil)
		// Setup
		sFolder = new CFolder(CFilesystemPath(CString(Package::Current().InstalledLocation().Path().data())));

	return *sFolder;
}
