//----------------------------------------------------------------------------------------------------------------------
//	CPreferences-Apple.cpp			©2003 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CPreferences.h"

#include "CData.h"
#include "CCoreFoundation.h"
#include "CLogServices.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CPreferences::Internals

class CPreferences::Internals {
	public:
					Internals() :
						mApplicationIDStringRef(kCFPreferencesCurrentApplication), mDelayWriteCount(0)
						{}
					Internals(const CPreferences::Reference& reference) :
						mApplicationIDStringRef(reference.getApplicationIDStringRef()), mDelayWriteCount(0)
						{}

		CFTypeRef	copyFrom(CFStringRef keyStringRef)
						{
							// Setup
							CFTypeRef	typeRef;

							::CFPreferencesAppSynchronize(mApplicationIDStringRef);
							typeRef = ::CFPreferencesCopyAppValue(keyStringRef, mApplicationIDStringRef);

							// Did we get it?
							if ((typeRef == nil) && mAlternatePreferencesReference.hasValue())
								// Try to get from old prefs file
								typeRef =
										::CFPreferencesCopyAppValue(keyStringRef,
												mAlternatePreferencesReference->getApplicationIDStringRef());

							return typeRef;
						}
		void		setTo(CFStringRef keyStringRef, CFTypeRef valueTypeRef)
						{
							// Store
							::CFPreferencesSetAppValue(keyStringRef, valueTypeRef, mApplicationIDStringRef);

							if (mDelayWriteCount == 0)
								::CFPreferencesAppSynchronize(mApplicationIDStringRef);
						}
		void		beginGroupSet()
						{ mDelayWriteCount++; }
		void		endGroupSet()
						{
							if (--mDelayWriteCount == 0)
								::CFPreferencesAppSynchronize(mApplicationIDStringRef);
						}

		CFStringRef					mApplicationIDStringRef;
		OV<CPreferences::Reference>	mAlternatePreferencesReference;
		UInt32						mDelayWriteCount;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CPreferences

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CPreferences::CPreferences()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals();
}

//----------------------------------------------------------------------------------------------------------------------
CPreferences::CPreferences(const Reference& reference)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(reference);
}

//----------------------------------------------------------------------------------------------------------------------
CPreferences::~CPreferences()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
bool CPreferences::hasValue(const Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	CFTypeRef	typeRef = mInternals->copyFrom(pref.getKeyString());
	bool		flag = typeRef != nil;
	
	if (typeRef != nil)
		::CFRelease(typeRef);
	
	return flag;
}

//----------------------------------------------------------------------------------------------------------------------
bool CPreferences::getBool(const BoolPref& boolPref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFBooleanRef	booleanRef = (CFBooleanRef) mInternals->copyFrom(boolPref.getKeyString());

	// Check if have number
	if (booleanRef != nil) {
		// Have value
		bool	value = booleanRef == kCFBooleanTrue;
		::CFRelease(booleanRef);
		
		return value;
	} else
		return boolPref.getDefaultValue();
}

//----------------------------------------------------------------------------------------------------------------------
OV<TArray<CData> > CPreferences::getDataArray(const Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFArrayRef	arrayRef = (CFArrayRef) mInternals->copyFrom(pref.getKeyString());

	// Check if have array
	if (arrayRef != nil) {
		// Setup array
		TNArray<CData>	array = CCoreFoundation::arrayOfDatasFrom(arrayRef);

		// Cleanup
		::CFRelease(arrayRef);

		return OV<TArray<CData> >(array);
	} else
		// Not found
		return OV<TArray<CData> >();
}

//----------------------------------------------------------------------------------------------------------------------
OV<TArray<CDictionary> > CPreferences::getDictionaryArray(const Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFArrayRef	arrayRef = (CFArrayRef) mInternals->copyFrom(pref.getKeyString());

	// Check if have array
	if (arrayRef != nil) {
		// Setup array
		TNArray<CDictionary>	array = CCoreFoundation::arrayOfDictionariesFrom(arrayRef);

		// Cleanup
		::CFRelease(arrayRef);

		return OV<TArray<CDictionary> >(array);
	} else
		// Not found
		return OV<TArray<CDictionary> >();
}

//----------------------------------------------------------------------------------------------------------------------
OV<TNumberArray<OSType> > CPreferences::getOSTypeArray(const Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFArrayRef	arrayRef = (CFArrayRef) mInternals->copyFrom(pref.getKeyString());

	// Check if have array
	if (arrayRef != nil) {
		// Iterate all items
		TNumberArray<OSType>	array;
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

		return OV<TNumberArray<OSType> >(array);
	} else
		// Not found
		return OV<TNumberArray<OSType> >();
}

//----------------------------------------------------------------------------------------------------------------------
OV<CData> CPreferences::getData(const Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFDataRef	dataRef = (CFDataRef) mInternals->copyFrom(pref.getKeyString());

	// Check if have data
	if (dataRef != nil) {
		// Have value
		CData	data = CCoreFoundation::dataFrom(dataRef);
		::CFRelease(dataRef);

		return OV<CData>(data);
	} else
		return OV<CData>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<CDictionary> CPreferences::getDictionary(const Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFDictionaryRef	dictionaryRef = (CFDictionaryRef) mInternals->copyFrom(pref.getKeyString());

	// Check if have dictionary
	if (dictionaryRef != nil) {
		// Have value
		CDictionary	dictionary = CCoreFoundation::dictionaryFrom(dictionaryRef);
		::CFRelease(dictionaryRef);

		return OV<CDictionary>(dictionary);
	} else
		return OV<CDictionary>();
}

//----------------------------------------------------------------------------------------------------------------------
CString CPreferences::getString(const StringPref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFStringRef	stringRef = (CFStringRef) mInternals->copyFrom(pref.getKeyString());

	// Check if have string
	if (stringRef != nil) {
		// Have value
		CString	string(stringRef);
		::CFRelease(stringRef);
		
		return string;
	} else
		return CString(pref.getDefaultValue());
}

//----------------------------------------------------------------------------------------------------------------------
Float32 CPreferences::getFloat32(const Float32Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFNumberRef	numberRef = (CFNumberRef) mInternals->copyFrom(pref.getKeyString());

	// Check if have number
	if (numberRef != nil) {
		// Have value
		Float32	value;
		::CFNumberGetValue(numberRef, kCFNumberFloat32Type, &value);
		::CFRelease(numberRef);
		
		return value;
	} else
		return pref.getDefaultValue();
}

//----------------------------------------------------------------------------------------------------------------------
Float64 CPreferences::getFloat64(const Float64Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFNumberRef	numberRef = (CFNumberRef) mInternals->copyFrom(pref.getKeyString());

	// Check if have number
	if (numberRef != nil) {
		// Have value
		Float64	value;
		::CFNumberGetValue(numberRef, kCFNumberFloat64Type, &value);
		::CFRelease(numberRef);
		
		return value;
	} else
		return pref.getDefaultValue();
}

//----------------------------------------------------------------------------------------------------------------------
SInt32 CPreferences::getSInt32(const SInt32Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFNumberRef	numberRef = (CFNumberRef) mInternals->copyFrom(pref.getKeyString());

	// Check if have number
	if (numberRef != nil) {
		// Have value
		SInt64	value;
		::CFNumberGetValue(numberRef, kCFNumberSInt64Type, &value);
		::CFRelease(numberRef);

		return (SInt32) value;
	} else
		return pref.getDefaultValue();
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CPreferences::getUInt32(const UInt32Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFNumberRef	numberRef = (CFNumberRef) mInternals->copyFrom(pref.getKeyString());

	// Check if have number
	if (numberRef != nil) {
		// Have value
		SInt64	value;
		::CFNumberGetValue(numberRef, kCFNumberSInt64Type, &value);
		::CFRelease(numberRef);
		
		return (UInt32) value;
	} else
		return pref.getDefaultValue();
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CPreferences::getUInt64(const UInt64Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFNumberRef	numberRef = (CFNumberRef) mInternals->copyFrom(pref.getKeyString());

	// Check if have number
	if (numberRef != nil) {
		// Have value
		SInt64	value;
		::CFNumberGetValue(numberRef, kCFNumberSInt64Type, &value);
		::CFRelease(numberRef);

		return (UInt64) value;
	} else
		return pref.getDefaultValue();
}

//----------------------------------------------------------------------------------------------------------------------
UniversalTimeInterval CPreferences::getUniversalTimeInterval(const UniversalTimeIntervalPref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFNumberRef	numberRef = (CFNumberRef) mInternals->copyFrom(pref.getKeyString());

	// Check if have number
	if (numberRef != nil) {
		// Have value
		Float64	value;
		::CFNumberGetValue(numberRef, kCFNumberFloat64Type, &value);
		::CFRelease(numberRef);

		return value;
	} else
		return pref.getDefaultValue();
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const BoolPref& boolPref, bool value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Write
	mInternals->setTo(boolPref.getKeyString(), value ? kCFBooleanTrue : kCFBooleanFalse);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const Pref& pref, const TArray<CData>& array)
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
	mInternals->setTo(pref.getKeyString(), arrayRef);

	// Cleanup
	::CFRelease(arrayRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const Pref& pref, const TArray<CDictionary>& array)
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
	mInternals->setTo(pref.getKeyString(), arrayRef);

	// Cleanup
	::CFRelease(arrayRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const Pref& pref, const TNumberArray<OSType>& array)
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
	mInternals->setTo(pref.getKeyString(), arrayRef);

	// Cleanup
	::CFRelease(arrayRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const Pref& pref, const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	CFDataRef	dataRef = CCoreFoundation::createDataRefFrom(data);
	mInternals->setTo(pref.getKeyString(), dataRef);
	::CFRelease(dataRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const Pref& pref, const CDictionary& dictionary)
//----------------------------------------------------------------------------------------------------------------------
{
	CFDictionaryRef	dictionaryRef = CCoreFoundation::createDictionaryRefFrom(dictionary);
	mInternals->setTo(pref.getKeyString(), dictionaryRef);
	::CFRelease(dictionaryRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const StringPref& pref, const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->setTo(pref.getKeyString(), string.getOSString());
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const Float32Pref& pref, Float32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Create storage
	CFNumberRef	numberRef = ::CFNumberCreate(kCFAllocatorDefault, kCFNumberFloat32Type, &value);

	// Write
	mInternals->setTo(pref.getKeyString(), numberRef);
	::CFRelease(numberRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const Float64Pref& pref, Float64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Create storage
	CFNumberRef	numberRef = ::CFNumberCreate(kCFAllocatorDefault, kCFNumberFloat64Type, &value);

	// Write
	mInternals->setTo(pref.getKeyString(), numberRef);
	::CFRelease(numberRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SInt32Pref& pref, SInt32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Create storage
	SInt64		sInt64Value = value;
	CFNumberRef	numberRef = ::CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type, &sInt64Value);

	// Write
	mInternals->setTo(pref.getKeyString(), numberRef);
	::CFRelease(numberRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const UInt32Pref& pref, UInt32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Create storage
	SInt64		sInt64Value = value;
	CFNumberRef	numberRef = ::CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type, &sInt64Value);

	// Write
	mInternals->setTo(pref.getKeyString(), numberRef);
	::CFRelease(numberRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const UInt64Pref& pref, UInt64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Create storage
	SInt64		sInt64Value = value;
	CFNumberRef	numberRef = ::CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type, &sInt64Value);

	// Write
	mInternals->setTo(pref.getKeyString(), numberRef);
	::CFRelease(numberRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const UniversalTimeIntervalPref& pref, UniversalTimeInterval value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Create storage
	CFNumberRef	numberRef = ::CFNumberCreate(kCFAllocatorDefault, kCFNumberFloat64Type, &value);

	// Write
	mInternals->setTo(pref.getKeyString(), numberRef);
	::CFRelease(numberRef);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::remove(const Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->setTo(pref.getKeyString(), nil);
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
void CPreferences::setAlternate(const Reference& reference)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mAlternatePreferencesReference = OV<Reference>(reference);
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CPreferences& CPreferences::shared()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	CPreferences*	sPreferences = nil;

	// Check if first time
	if (sPreferences == nil)
		// Instantiate
		sPreferences = new CPreferences();

	return *sPreferences;
}
