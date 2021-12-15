//----------------------------------------------------------------------------------------------------------------------
//	CApplication-Windows-WinRT.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CApplication.h"

#include <winrt/Windows.ApplicationModel.h>

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
const SVersionInfo& CApplication::getVersion()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	SVersionInfo*	sVersionInfo = nil;

	// Check if have already set up
	if (sVersionInfo == nil) {
		// Get info
		auto	version = Package::Current().Id().Version();

		// Setup version
		sVersionInfo = new SVersionInfo((UInt8) version.Major, (UInt8) version.Minor, (UInt8) version.Build);
	}

	return *sVersionInfo;
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
