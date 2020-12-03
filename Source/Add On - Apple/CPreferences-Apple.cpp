//----------------------------------------------------------------------------------------------------------------------
//	CPreferences-Apple.cpp			©2003 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CPreferences.h"

#include "CData.h"
#include "CCoreFoundation.h"
#include "CLogServices.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CPreferencesInternals

class CPreferencesInternals {
	public:
					CPreferencesInternals() :
						mApplicationID((CFStringRef) ::CFRetain(kCFPreferencesCurrentApplication)), mDelayWriteCount(0)
						{}
					CPreferencesInternals(const SPreferencesReference& preferencesReference) :
						mApplicationID(CCoreFoundation::createStringRefFrom(preferencesReference.mApplicationID)),
								mDelayWriteCount(0)
						{}
					~CPreferencesInternals()
						{
							::CFRelease(mApplicationID);
						}

		CFTypeRef	copyFrom(const SPref& pref)
						{
							// Check for no key
							if (pref.mKeyString == nil)
								return nil;

							// Setup
							CFTypeRef	typeRef;

							::CFPreferencesAppSynchronize(mApplicationID);
							typeRef = ::CFPreferencesCopyAppValue(pref.mKeyString, mApplicationID);

							// Did we get it?
							if ((typeRef == nil) && mAlternatePreferencesReference.hasInstance()) {
								// Try to get from old prefs file
								CFStringRef	alternateApplicationIDStringRef =
													CCoreFoundation::createStringRefFrom(
															mAlternatePreferencesReference->mApplicationID);
								typeRef =
										::CFPreferencesCopyAppValue(pref.mKeyString, alternateApplicationIDStringRef);
								::CFRelease(alternateApplicationIDStringRef);
							}

							return typeRef;
						}
		void		setTo(const SPref& pref, CFTypeRef valueTypeRef)
						{
							// Check for no key
							if (pref.mKeyString == nil)
								return;

							// Store
							::CFPreferencesSetAppValue(pref.mKeyString, valueTypeRef, mApplicationID);

							if (mDelayWriteCount == 0)
								::CFPreferencesAppSynchronize(mApplicationID);
						}
		void		beginGroupSet()
						{ mDelayWriteCount++; }
		void		endGroupSet()
						{
							if (--mDelayWriteCount == 0)
								::CFPreferencesAppSynchronize(mApplicationID);
						}

		CFStringRef					mApplicationID;
		OI<SPreferencesReference>	mAlternatePreferencesReference;
		UInt32						mDelayWriteCount;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CPreferences

// MARK: Properties

CPreferences	CPreferences::mDefault;

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CPreferences::CPreferences()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CPreferencesInternals();
}

//----------------------------------------------------------------------------------------------------------------------
CPreferences::CPreferences(const SPreferencesReference& preferencesReference)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CPreferencesInternals(preferencesReference);
}

//----------------------------------------------------------------------------------------------------------------------
CPreferences::~CPreferences()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
bool CPreferences::hasValue(const SPref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	CFTypeRef	typeRef = mInternals->copyFrom(pref);
	bool		flag = typeRef != nil;
	
	if (typeRef != nil)
		::CFRelease(typeRef);
	
	return flag;
}

//----------------------------------------------------------------------------------------------------------------------
TNArray<CData> CPreferences::getDataArray(const SPref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFArrayRef	arrayRef = (CFArrayRef) mInternals->copyFrom(pref);

	// Check if have array
	if (arrayRef != nil) {
		// Setup array
		TNArray<CData>	array = CCoreFoundation::arrayOfDatasFrom(arrayRef);

		// Cleanup
		::CFRelease(arrayRef);

		return array;
	} else
		// Not found
		return TNArray<CData>();
}

//----------------------------------------------------------------------------------------------------------------------
TNArray<CDictionary> CPreferences::getDictionaryArray(const SPref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFArrayRef	arrayRef = (CFArrayRef) mInternals->copyFrom(pref);

	// Check if have array
	if (arrayRef != nil) {
		// Setup array
		TNArray<CDictionary>	array = CCoreFoundation::arrayOfDictionariesFrom(arrayRef);

		// Cleanup
		::CFRelease(arrayRef);

		return array;
	} else
		// Not found
		return TNArray<CDictionary>();
}

//----------------------------------------------------------------------------------------------------------------------
TNumericArray<OSType> CPreferences::getOSTypeArray(const SPref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFArrayRef	arrayRef = (CFArrayRef) mInternals->copyFrom(pref);

	// Check if have array
	TNumericArray<OSType>	array;
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
CData CPreferences::getData(const SPref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	CFDataRef	dataRef = (CFDataRef) mInternals->copyFrom(pref);
	if (dataRef != nil) {
		CData	data = CCoreFoundation::dataFrom(dataRef);
		::CFRelease(dataRef);

		return data;
	} else
		return CData::mEmpty;
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary CPreferences::getDictionary(const SPref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	CFDictionaryRef	dictionaryRef = (CFDictionaryRef) mInternals->copyFrom(pref);
	if (dictionaryRef != nil) {
		CDictionary	dictionary = CCoreFoundation::dictionaryFrom(dictionaryRef);
		::CFRelease(dictionaryRef);

		return dictionary;
	} else
		return CDictionary();
}

//----------------------------------------------------------------------------------------------------------------------
CString CPreferences::getString(const SStringPref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	CFStringRef	stringRef = (CFStringRef) mInternals->copyFrom(pref);
	if (stringRef != nil) {
		CString	string(stringRef);
		::CFRelease(stringRef);
		
		return string;
	} else
		return CString(pref.mDefaultValue);
}

//----------------------------------------------------------------------------------------------------------------------
Float32 CPreferences::getFloat32(const SFloat32Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	CFNumberRef	numberRef = (CFNumberRef) mInternals->copyFrom(pref);
	if (numberRef != nil) {
		Float32	value;
		::CFNumberGetValue(numberRef, kCFNumberFloat32Type, &value);
		::CFRelease(numberRef);
		
		return value;
	} else
		return pref.mDefaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
Float64 CPreferences::getFloat64(const SFloat64Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	CFNumberRef	numberRef = (CFNumberRef) mInternals->copyFrom(pref);
	if (numberRef != nil) {
		Float64	value;
		::CFNumberGetValue(numberRef, kCFNumberFloat64Type, &value);
		::CFRelease(numberRef);
		
		return value;
	} else
		return pref.mDefaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
SInt32 CPreferences::getSInt32(const SSInt32Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	CFNumberRef	numberRef = (CFNumberRef) mInternals->copyFrom(pref);
	if (numberRef != nil) {
		SInt64	value;
		::CFNumberGetValue(numberRef, kCFNumberSInt64Type, &value);
		::CFRelease(numberRef);

		return (SInt32) value;
	} else
		return pref.mDefaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CPreferences::getUInt32(const SUInt32Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	CFNumberRef	numberRef = (CFNumberRef) mInternals->copyFrom(pref);
	if (numberRef != nil) {
		SInt64	value;
		::CFNumberGetValue(numberRef, kCFNumberSInt64Type, &value);
		::CFRelease(numberRef);
		
		return (UInt32) value;
	} else
		return pref.mDefaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CPreferences::getUInt64(const SUInt64Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	CFNumberRef	numberRef = (CFNumberRef) mInternals->copyFrom(pref);
	if (numberRef != nil) {
		SInt64	value;
		::CFNumberGetValue(numberRef, kCFNumberSInt64Type, &value);
		::CFRelease(numberRef);

		return (UInt64) value;
	} else
		return pref.mDefaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
UniversalTimeInterval CPreferences::getUniversalTimeInterval(const SUniversalTimeIntervalPref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	CFNumberRef	numberRef = (CFNumberRef) mInternals->copyFrom(pref);
	if (numberRef != nil) {
		Float64	value;
		::CFNumberGetValue(numberRef, kCFNumberFloat64Type, &value);
		::CFRelease(numberRef);

		return value;
	} else
		return pref.mDefaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SPref& pref, const TArray<CData>& array)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFMutableArrayRef	arrayRef = ::CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);

	// Iterate items
	for (CArray::ItemIndex i = 0; i < array.getCount(); i++) {
		// Add data
		CFDataRef	dataRef = CCoreFoundation::createDataRefFrom(array[i]);
		::CFArrayAppendValue(arrayRef, dataRef);
		::CFRelease(dataRef);
	}

	// Store
	mInternals->setTo(pref, arrayRef);

	// Cleanup
	::CFRelease(arrayRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SPref& pref, const TArray<CDictionary>& array)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFMutableArrayRef	arrayRef = ::CFArrayCreateMutable(kCFAllocatorDefault, 0, &kCFTypeArrayCallBacks);

	// Iterate items
	for (CArray::ItemIndex i = 0; i < array.getCount(); i++) {
		// Add dictionary
		CFDictionaryRef	dictionaryRef = CCoreFoundation::createDictionaryRefFrom(array[i]);
		::CFArrayAppendValue(arrayRef, dictionaryRef);
		::CFRelease(dictionaryRef);
	}

	// Store
	mInternals->setTo(pref, arrayRef);

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
	for (CArray::ItemIndex i = 0; i < array.getCount(); i++) {
		// Add data
		SInt64		sInt64Value = array[i];
		CFNumberRef	numberRef = ::CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type, &sInt64Value);
		::CFArrayAppendValue(arrayRef, numberRef);
		::CFRelease(numberRef);
	}

	// Store
	mInternals->setTo(pref, arrayRef);

	// Cleanup
	::CFRelease(arrayRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SPref& pref, const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	CFDataRef	dataRef = CCoreFoundation::createDataRefFrom(data);
	mInternals->setTo(pref, dataRef);
	::CFRelease(dataRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SPref& pref, const CDictionary& dictionary)
//----------------------------------------------------------------------------------------------------------------------
{
	CFDictionaryRef	dictionaryRef = CCoreFoundation::createDictionaryRefFrom(dictionary);
	mInternals->setTo(pref, dictionaryRef);
	::CFRelease(dictionaryRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SStringPref& pref, const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	CFStringRef	stringRef = CCoreFoundation::createStringRefFrom(string);
	mInternals->setTo(pref, stringRef);
	::CFRelease(stringRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SFloat32Pref& pref, Float32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Create storage
	CFNumberRef	numberRef = ::CFNumberCreate(kCFAllocatorDefault, kCFNumberFloat32Type, &value);

	// Write
	mInternals->setTo(pref, numberRef);
	::CFRelease(numberRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SFloat64Pref& pref, Float64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Create storage
	CFNumberRef	numberRef = ::CFNumberCreate(kCFAllocatorDefault, kCFNumberFloat64Type, &value);

	// Write
	mInternals->setTo(pref, numberRef);
	::CFRelease(numberRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SSInt32Pref& pref, SInt32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Create storage
	SInt64		sInt64Value = value;
	CFNumberRef	numberRef = ::CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type, &sInt64Value);

	// Write
	mInternals->setTo(pref, numberRef);
	::CFRelease(numberRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SUInt32Pref& pref, UInt32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Create storage
	SInt64		sInt64Value = value;
	CFNumberRef	numberRef = ::CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type, &sInt64Value);

	// Write
	mInternals->setTo(pref, numberRef);
	::CFRelease(numberRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SUInt64Pref& pref, UInt64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Create storage
	SInt64		sInt64Value = value;
	CFNumberRef	numberRef = ::CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type, &sInt64Value);

	// Write
	mInternals->setTo(pref, numberRef);
	::CFRelease(numberRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SUniversalTimeIntervalPref& pref, UniversalTimeInterval value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Create storage
	CFNumberRef	numberRef = ::CFNumberCreate(kCFAllocatorDefault, kCFNumberFloat64Type, &value);

	// Write
	mInternals->setTo(pref, numberRef);
	::CFRelease(numberRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::remove(const SPref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->setTo(pref, nil);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::beginGroupSet()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->beginGroupSet();
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::endGroupSet()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->endGroupSet();
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::setAlternate(const SPreferencesReference& preferencesReference)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mAlternatePreferencesReference = OI<SPreferencesReference>(preferencesReference);
}
