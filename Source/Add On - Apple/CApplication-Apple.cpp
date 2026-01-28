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
const CString& CApplication::getVersion()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	CString*	sVersion = nil;

	// Check if have already set up
	if (sVersion == nil)
		// Setup
		sVersion =
				new CString((CFStringRef)
						::CFBundleGetValueForInfoDictionaryKey(::CFBundleGetMainBundle(),
								CFSTR("CFBundleShortVersionString")));

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
		sProductNameStringAndVersion = new CString(getProductName() + CString::mSpace + getVersion());

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
