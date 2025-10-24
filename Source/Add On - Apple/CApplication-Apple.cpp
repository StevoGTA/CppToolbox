//----------------------------------------------------------------------------------------------------------------------
//	CApplication-Apple.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CApplication.h"

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
	if (sProductNameString == nil) {
		// Setup
		CFStringRef	stringRef =
							(CFStringRef) ::CFBundleGetValueForInfoDictionaryKey(::CFBundleGetMainBundle(),
									CFSTR("CFBundleDisplayName"));
		if (stringRef == nil)
			// Try another key
			stringRef =
					(CFStringRef) ::CFBundleGetValueForInfoDictionaryKey(::CFBundleGetMainBundle(),
							CFSTR("CFBundleName"));
		if (stringRef != nil)
			// Got string
			sProductNameString = new CString(stringRef);
	}

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
		CFStringRef		stringRef =
								(CFStringRef) ::CFBundleGetValueForInfoDictionaryKey(::CFBundleGetMainBundle(),
										CFSTR("CFBundleShortVersionString"));
		TArray<CString>	array = CString(stringRef).components(CString::mPeriod);
		UInt8			majorVersion = (array.getCount() > 0) ? array[0].getUInt8() : 0;
		UInt8			minorVersion = (array.getCount() > 1) ? array[1].getUInt8() : 0;
		UInt8			patchVersion = (array.getCount() > 2) ? array[2].getUInt8() : 0;

		sVersionInfo =
				(majorVersion != 0) ?
						new SVersionInfo(majorVersion, minorVersion, patchVersion) :
						new SVersionInfo(CString(stringRef));
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
		sProductNameStringAndVersion = new CString(getProductName() + CString::mSpace + getVersion().getString());

	return *sProductNameStringAndVersion;
}

//----------------------------------------------------------------------------------------------------------------------
const CString& CApplication::getCopyright()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	CString*	sCopyrightString = nil;

	// Check if have already set up
	if (sCopyrightString == nil) {
		// Setup
		CFStringRef	stringRef =
							(CFStringRef) ::CFBundleGetValueForInfoDictionaryKey(::CFBundleGetMainBundle(),
									CFSTR("CFBundleShortCopyrightString"));
		if (stringRef != nil)
			// Got string
			sCopyrightString = new CString(stringRef);
	}

	return (sCopyrightString != nil) ? *sCopyrightString : CString::mEmpty;
}
