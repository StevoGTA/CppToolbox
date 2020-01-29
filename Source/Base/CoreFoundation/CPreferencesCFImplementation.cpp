//----------------------------------------------------------------------------------------------------------------------
//	CPreferences.cpp			©2003 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CPreferences.h"

//#include "CArrayX.h"
//#include "CArrayCFImplementation.h"
//#include "CFUtilities.h"
#include "CData.h"
//#include "CDictionaryX.h"
//#include "CDictionaryCFImplementation.h"
#include "CFUtilities.h"
#include "CLogServices.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CString		sAlternateApplicationID;

static	UInt32		sDelayWriteCount = 0;

SPref			CPreferences::mNoPref(nil);
SStringPref		CPreferences::mNoStringPref(nil, OSSTR(""));
SFloat32Pref	CPreferences::mNoFloat32Pref(nil, 0.0);
SFloat64Pref	CPreferences::mNoFloat64Pref(nil, 0.0);
SUInt32Pref		CPreferences::mNoUInt32Pref(nil, 0);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc declarations

static	CFTypeRef	sCopyFrom(const char* keyString, const CString& applicationID);
static	void		sSetTo(const char* keyString, CFTypeRef valueTypeRef);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: CPreferences

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
bool CPreferences::hasValue(const SPref& pref, const CString& applicationID)
//----------------------------------------------------------------------------------------------------------------------
{
	CFTypeRef	typeRef = sCopyFrom(pref.mKeyString, applicationID);
	bool		flag = typeRef != nil;
	
	if (typeRef != nil)
		::CFRelease(typeRef);
	
	return flag;
}

//----------------------------------------------------------------------------------------------------------------------
TArray<CData> CPreferences::getDataArray(const SPref& pref, const CString& applicationID)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TArray<CData>	array;
	CFArrayRef		arrayRef = (CFArrayRef) sCopyFrom(pref.mKeyString, applicationID);

	// Check if have array
	if (arrayRef != nil) {
		// Setup array
		array = eArrayOfDatasFrom(arrayRef);

		// Cleanup
		::CFRelease(arrayRef);
	}

	return array;
}

//----------------------------------------------------------------------------------------------------------------------
TNumericArray<OSType> CPreferences::getOSTypeArray(const SPref& pref, const CString& applicationID)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNumericArray<OSType>	array;
	CFArrayRef				arrayRef = (CFArrayRef) sCopyFrom(pref.mKeyString, applicationID);

	// Check if have array
	if (arrayRef != nil) {
		// Iterate all items
		for (CFIndex i = 0; i < ::CFArrayGetCount(arrayRef); i++) {
			// Get value
			CFNumberRef	numberRef = (CFNumberRef) ::CFArrayGetValueAtIndex(arrayRef, i);

			SInt64	value;
			::CFNumberGetValue(numberRef, kCFNumberSInt64Type, &value);

			// Add to array
			array += (OSType) value;
		}

		// Cleanup
		::CFRelease(arrayRef);
	}

	return array;
}

//----------------------------------------------------------------------------------------------------------------------
CData CPreferences::getData(const SPref& pref, const CString& applicationID)
//----------------------------------------------------------------------------------------------------------------------
{
	CFDataRef	dataRef = (CFDataRef) sCopyFrom(pref.mKeyString, applicationID);
	if (dataRef != nil) {
		CData	data = eDataFrom(dataRef);
		::CFRelease(dataRef);

		return data;
	} else
		return CData::mEmpty;
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary CPreferences::getDictionary(const SPref& pref, const CString& applicationID)
//----------------------------------------------------------------------------------------------------------------------
{
	CFDictionaryRef	dictionaryRef = (CFDictionaryRef) sCopyFrom(pref.mKeyString, applicationID);
	if (dictionaryRef != nil) {
		CDictionary	dictionary = eDictionaryFrom(dictionaryRef);
		::CFRelease(dictionaryRef);

		return dictionary;
	} else
		return CDictionary();
}

//----------------------------------------------------------------------------------------------------------------------
CString CPreferences::getString(const SStringPref& pref, const CString& applicationID)
//----------------------------------------------------------------------------------------------------------------------
{
	CFStringRef	stringRef = (CFStringRef) sCopyFrom(pref.mKeyString, applicationID);
	if (stringRef != nil) {
		CString	string(stringRef);
		::CFRelease(stringRef);
		
		return string;
	} else
		return CString(pref.mDefaultValue);
}

//----------------------------------------------------------------------------------------------------------------------
Float32 CPreferences::getFloat32(const SFloat32Pref& pref, const CString& applicationID)
//----------------------------------------------------------------------------------------------------------------------
{
	CFNumberRef	numberRef = (CFNumberRef) sCopyFrom(pref.mKeyString, applicationID);
	if (numberRef != nil) {
		Float32	value;
		::CFNumberGetValue(numberRef, kCFNumberFloat32Type, &value);
		::CFRelease(numberRef);
		
		return value;
	} else
		return pref.mDefaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
Float64 CPreferences::getFloat64(const SFloat64Pref& pref, const CString& applicationID)
//----------------------------------------------------------------------------------------------------------------------
{
	CFNumberRef	numberRef = (CFNumberRef) sCopyFrom(pref.mKeyString, applicationID);
	if (numberRef != nil) {
		Float64	value;
		::CFNumberGetValue(numberRef, kCFNumberFloat64Type, &value);
		::CFRelease(numberRef);
		
		return value;
	} else
		return pref.mDefaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
SInt32 CPreferences::getSInt32(const SSInt32Pref& pref, const CString& applicationID)
//----------------------------------------------------------------------------------------------------------------------
{
	CFNumberRef	numberRef = (CFNumberRef) sCopyFrom(pref.mKeyString, applicationID);
	if (numberRef != nil) {
		SInt64	value;
		::CFNumberGetValue(numberRef, kCFNumberSInt64Type, &value);
		::CFRelease(numberRef);

		return (SInt32) value;
	} else
		return pref.mDefaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CPreferences::getUInt32(const SUInt32Pref& pref, const CString& applicationID)
//----------------------------------------------------------------------------------------------------------------------
{
	CFNumberRef	numberRef = (CFNumberRef) sCopyFrom(pref.mKeyString, applicationID);
	if (numberRef != nil) {
		SInt64	value;
		::CFNumberGetValue(numberRef, kCFNumberSInt64Type, &value);
		::CFRelease(numberRef);
		
		return (UInt32) value;
	} else
		return pref.mDefaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CPreferences::getUInt64(const SUInt64Pref& pref, const CString& applicationID)
//----------------------------------------------------------------------------------------------------------------------
{
	CFNumberRef	numberRef = (CFNumberRef) sCopyFrom(pref.mKeyString, applicationID);
	if (numberRef != nil) {
		SInt64	value;
		::CFNumberGetValue(numberRef, kCFNumberSInt64Type, &value);
		::CFRelease(numberRef);

		return (UInt64) value;
	} else
		return pref.mDefaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SPref& pref, const TPtrArray<CData*>& array)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFMutableArrayRef	arrayRef = ::CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);

	// Iterate items
	for (CArrayItemIndex i = 0; i < array.getCount(); i++) {
		// Add data
		CData*		data = array[i];
		CFDataRef	dataRef = eDataCopyCFDataRef(*data);
		::CFArrayAppendValue(arrayRef, dataRef);
		::CFRelease(dataRef);
	}

	// Store
	sSetTo(pref.mKeyString, arrayRef);

	// Cleanup
	::CFRelease(arrayRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SPref& pref, const TNumericArray<OSType>& array)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFMutableArrayRef	arrayRef = ::CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);

	// Iterate items
	for (CArrayItemIndex i = 0; i < array.getCount(); i++) {
		// Add data
		SInt64		sInt64Value = array[i];
		CFNumberRef	numberRef = ::CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type, &sInt64Value);
		::CFArrayAppendValue(arrayRef, numberRef);
		::CFRelease(numberRef);
	}

	// Store
	sSetTo(pref.mKeyString, arrayRef);

	// Cleanup
	::CFRelease(arrayRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SPref& pref, const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	CFDataRef	dataRef = eDataCopyCFDataRef(data);
	sSetTo(pref.mKeyString, dataRef);
	::CFRelease(dataRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SPref& pref, const CDictionary& dictionary)
//----------------------------------------------------------------------------------------------------------------------
{
	CFDictionaryRef	dictionaryRef = eDictionaryCopyCFDictionaryRef(dictionary);
	sSetTo(pref.mKeyString, dictionaryRef);
	::CFRelease(dictionaryRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SStringPref& pref, const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	CFStringRef	stringRef = eStringCopyCFStringRef(string);
	sSetTo(pref.mKeyString, stringRef);
	::CFRelease(stringRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SFloat32Pref& pref, Float32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Create storage
	CFNumberRef	numberRef = ::CFNumberCreate(kCFAllocatorDefault, kCFNumberFloat32Type, &value);
	if (numberRef == nil)
		LogIfErrorAndReturn(kMemFullError, "creating CFNumberRef");

	// Write
	sSetTo(pref.mKeyString, numberRef);
	::CFRelease(numberRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SFloat64Pref& pref, Float64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Create storage
	CFNumberRef	numberRef = ::CFNumberCreate(kCFAllocatorDefault, kCFNumberFloat64Type, &value);
	if (numberRef == nil)
		LogIfErrorAndReturn(kMemFullError, "creating CFNumberRef");

	// Write
	sSetTo(pref.mKeyString, numberRef);
	::CFRelease(numberRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SSInt32Pref& pref, SInt32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Create storage
	SInt64		sInt64Value = value;
	CFNumberRef	numberRef = ::CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type, &sInt64Value);
	if (numberRef == nil)
		LogIfErrorAndReturn(kMemFullError, "creating CFNumberRef");

	// Write
	sSetTo(pref.mKeyString, numberRef);
	::CFRelease(numberRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SUInt32Pref& pref, UInt32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Create storage
	SInt64		sInt64Value = value;
	CFNumberRef	numberRef = ::CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type, &sInt64Value);
	if (numberRef == nil)
		LogIfErrorAndReturn(kMemFullError, "creating CFNumberRef");

	// Write
	sSetTo(pref.mKeyString, numberRef);
	::CFRelease(numberRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SUInt64Pref& pref, UInt64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Create storage
	SInt64		sInt64Value = value;
	CFNumberRef	numberRef = ::CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type, &sInt64Value);
	if (numberRef == nil)
		LogIfErrorAndReturn(kMemFullError, "creating CFNumberRef");

	// Write
	sSetTo(pref.mKeyString, numberRef);
	::CFRelease(numberRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::remove(const SPref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	sSetTo(pref.mKeyString, nil);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::beginGroupSet()
//----------------------------------------------------------------------------------------------------------------------
{
	sDelayWriteCount++;
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::endGroupSet()
//----------------------------------------------------------------------------------------------------------------------
{
	if (--sDelayWriteCount == 0)
		::CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::setAlternateApplicationID(const CString& applicationID)
//----------------------------------------------------------------------------------------------------------------------
{
	sAlternateApplicationID = applicationID;
}

//#if TARGET_OS_MACOS
////----------------------------------------------------------------------------------------------------------------------
//CFileX CPreferences::getPrimaryFile()
////----------------------------------------------------------------------------------------------------------------------
//{
//	// Start with user Library folder
//	CFolderX	folder = CFolderX::userLibraryFolder();
//	if (folder.doesExist()) {
//		// Go to Preferences subfolder
//		folder = CFolderX(folder, CString("Preferences"));
//		if (folder.doesExist()) {
//			CString	bundleID((CFStringRef) CFBundleGetValueForInfoDictionaryKey(CFBundleGetMainBundle(),
//							kCFBundleIdentifierKey));
//			return CFileX(folder, bundleID + CString(".plist"));
//		}
//	}
//
//	return CFileX::mInvalidFile;
//}
//#endif
//
////----------------------------------------------------------------------------------------------------------------------
//CArrayX CPreferences::getArray(const SPref& pref, const CString& applicationID)
////----------------------------------------------------------------------------------------------------------------------
//{
//	CFArrayRef	arrayRef = (CFArrayRef) sCopyFrom(pref.mKeyString, applicationID);
//	if (arrayRef != nil) {
//		CArrayX	array = CArrayWithCFArrayRef(arrayRef);
//		::CFRelease(arrayRef);
//
//		return array;
//	} else
//		return CArrayX();
//}
//
////----------------------------------------------------------------------------------------------------------------------
//CDictionaryX CPreferences::getDictionaryX(const SPref& pref, const CString& applicationID)
////----------------------------------------------------------------------------------------------------------------------
//{
//	CFDictionaryRef	dictionaryRef = (CFDictionaryRef) sCopyFrom(pref.mKeyString, applicationID);
//	if (dictionaryRef != nil) {
//		CDictionaryX	dictionary = CDictionaryWithCFDictionaryRef(dictionaryRef);
//		::CFRelease(dictionaryRef);
//
//		return dictionary;
//	} else
//		return CDictionaryX();
//}
//
////----------------------------------------------------------------------------------------------------------------------
//void CPreferences::set(const SPref& pref, const CArrayX& array)
////----------------------------------------------------------------------------------------------------------------------
//{
//	sSetTo(pref.mKeyString, CArrayGetCFArrayRef(array));
//}
//
////----------------------------------------------------------------------------------------------------------------------
//void CPreferences::set(const SPref& pref, const CDictionaryX& dict)
////----------------------------------------------------------------------------------------------------------------------
//{
//	sSetTo(pref.mKeyString, CDictionaryGetCFDictionaryRef(dict));
//}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions

//----------------------------------------------------------------------------------------------------------------------
CFTypeRef sCopyFrom(const char* keyString, const CString& applicationID)
//----------------------------------------------------------------------------------------------------------------------
{
	if (keyString == nil)
		return nil;
	else {
		CFTypeRef	typeRef;
		CFStringRef	keyStringRef = eStringCopyCFStringRef(keyString);

		// Do we have a prefs file specified?
		if (!applicationID.isEmpty()) {
			// Yes, read from it
			CFStringRef	applicationIDStringRef = eStringCopyCFStringRef(applicationID);
			::CFPreferencesAppSynchronize(applicationIDStringRef);
			typeRef = ::CFPreferencesCopyAppValue(keyStringRef, applicationIDStringRef);
			::CFRelease(applicationIDStringRef);
		} else {
			// No, read from current
			typeRef = ::CFPreferencesCopyAppValue(keyStringRef, kCFPreferencesCurrentApplication);
		}
		
		// Did we get it?
		if ((typeRef == nil) && !sAlternateApplicationID.isEmpty()) {
			// Try to get from old prefs file
			CFStringRef	alternateApplicationIDStringRef = eStringCopyCFStringRef(sAlternateApplicationID);
			typeRef = ::CFPreferencesCopyAppValue(keyStringRef, alternateApplicationIDStringRef);
			::CFRelease(alternateApplicationIDStringRef);
		}

		// Cleanup
		::CFRelease(keyStringRef);

		return typeRef;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void sSetTo(const char* keyString, CFTypeRef valueTypeRef)
//----------------------------------------------------------------------------------------------------------------------
{
	if (keyString != nil) {
		CFStringRef	keyStringRef = eStringCopyCFStringRef(keyString);
		::CFPreferencesSetAppValue(keyStringRef, valueTypeRef, kCFPreferencesCurrentApplication);
		::CFRelease(keyStringRef);

		if (sDelayWriteCount == 0)
			::CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
	}
}
