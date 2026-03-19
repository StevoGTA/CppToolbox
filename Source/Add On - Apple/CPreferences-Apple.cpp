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
											mApplicationIDStringRef(kCFPreferencesCurrentApplication),
													mDelaySynchronizeCount(0)
											{}
										Internals(const CPreferences::Reference& reference) :
											mApplicationIDStringRef(reference.getApplicationIDStringRef()),
													mDelaySynchronizeCount(0)
											{}

		CCoreFoundation::OO<CFTypeRef>	get(CFStringRef keyStringRef)
											{
												// Setup
												CFTypeRef	typeRef;

												::CFPreferencesAppSynchronize(mApplicationIDStringRef);
												typeRef =
														::CFPreferencesCopyAppValue(keyStringRef,
																mApplicationIDStringRef);
												if (typeRef != nil)
													// Found
													return CCoreFoundation::OO<CFTypeRef>(typeRef);

												// Did we get it?
												if (mAlternatePreferencesReference.hasValue())
													// Try to get from old prefs file
													return CCoreFoundation::OO<CFTypeRef>(
															::CFPreferencesCopyAppValue(keyStringRef,
																	mAlternatePreferencesReference->
																			getApplicationIDStringRef()));

												return CCoreFoundation::OO<CFTypeRef>();
											}
		void							set(CFStringRef keyStringRef, CFTypeRef valueTypeRef)
											{
												// Store
												::CFPreferencesSetAppValue(keyStringRef, valueTypeRef,
														mApplicationIDStringRef);

												// Check if synchronizing now
												if (mDelaySynchronizeCount == 0)
													// Syncshronize
													::CFPreferencesAppSynchronize(mApplicationIDStringRef);
											}
		void							beginGroupSet()
											{ mDelaySynchronizeCount++; }
		void							endGroupSet()
											{
												if (--mDelaySynchronizeCount == 0)
													::CFPreferencesAppSynchronize(mApplicationIDStringRef);
											}

		CFStringRef					mApplicationIDStringRef;
		OV<CPreferences::Reference>	mAlternatePreferencesReference;
		UInt32						mDelaySynchronizeCount;
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
	return mInternals->get(pref.getKeyString()).hasObject();
}

//----------------------------------------------------------------------------------------------------------------------
bool CPreferences::getBool(const BoolPref& boolPref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	CCoreFoundation::OO<CFTypeRef>	booleanRef = mInternals->get(boolPref.getKeyString());

	return booleanRef.hasObject() ? (CFBooleanRef) *booleanRef == kCFBooleanTrue : boolPref.getDefaultValue();
}

//----------------------------------------------------------------------------------------------------------------------
OV<TArray<CData> > CPreferences::getDataArray(const Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	CCoreFoundation::OO<CFTypeRef>	arrayRef = mInternals->get(pref.getKeyString());

	return arrayRef.hasObject() ?
			OV<TArray<CData> >(CCoreFoundation::dataArrayFrom((CFArrayRef) *arrayRef)) : OV<TArray<CData> >();
}

//----------------------------------------------------------------------------------------------------------------------
OV<TArray<CDictionary> > CPreferences::getDictionaryArray(const Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	CCoreFoundation::OO<CFTypeRef>	arrayRef = mInternals->get(pref.getKeyString());

	return arrayRef.hasObject() ?
			OV<TArray<CDictionary> >(CCoreFoundation::dictionaryArrayFrom((CFArrayRef) *arrayRef)) :
			OV<TArray<CDictionary> >();
}

//----------------------------------------------------------------------------------------------------------------------
OV<TNumberArray<OSType> > CPreferences::getOSTypeArray(const Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CCoreFoundation::OO<CFTypeRef>	arrayRef = mInternals->get(pref.getKeyString());

	return arrayRef.hasObject() ?
			OV<TNumberArray<OSType> >(CCoreFoundation::osTypeArrayFrom((CFArrayRef) *arrayRef)) :
			OV<TNumberArray<OSType> >();
}

//----------------------------------------------------------------------------------------------------------------------
OV<CData> CPreferences::getData(const Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	CCoreFoundation::OO<CFTypeRef>	dataRef = mInternals->get(pref.getKeyString());

	return dataRef.hasObject() ? OV<CData>(CCoreFoundation::dataFrom((CFDataRef) *dataRef)) : OV<CData>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<CDictionary> CPreferences::getDictionary(const Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	CCoreFoundation::OO<CFTypeRef>	dictionaryRef = mInternals->get(pref.getKeyString());

	return dictionaryRef.hasObject() ?
			OV<CDictionary>(CCoreFoundation::dictionaryFrom((CFDictionaryRef) *dictionaryRef)) : OV<CDictionary>();
}

//----------------------------------------------------------------------------------------------------------------------
CString CPreferences::getString(const StringPref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	CCoreFoundation::OO<CFTypeRef>	stringRef = mInternals->get(pref.getKeyString());

	return stringRef.hasObject() ? CString(*stringRef) : pref.getDefaultValue();
}

//----------------------------------------------------------------------------------------------------------------------
Float32 CPreferences::getFloat32(const Float32Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	CCoreFoundation::OO<CFTypeRef>	numberRef = mInternals->get(pref.getKeyString());

	return numberRef.hasObject() ? CCoreFoundation::float32From((CFNumberRef) *numberRef) : pref.getDefaultValue();
}

//----------------------------------------------------------------------------------------------------------------------
Float64 CPreferences::getFloat64(const Float64Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	CCoreFoundation::OO<CFTypeRef>	numberRef = mInternals->get(pref.getKeyString());

	return numberRef.hasObject() ? CCoreFoundation::float32From((CFNumberRef) *numberRef) : pref.getDefaultValue();
}

//----------------------------------------------------------------------------------------------------------------------
SInt8 CPreferences::getSInt8(const SInt8Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	CCoreFoundation::OO<CFTypeRef>	numberRef = mInternals->get(pref.getKeyString());

	return numberRef.hasObject() ? CCoreFoundation::sInt8From((CFNumberRef) *numberRef) : pref.getDefaultValue();
}

//----------------------------------------------------------------------------------------------------------------------
SInt16 CPreferences::getSInt16(const SInt16Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	CCoreFoundation::OO<CFTypeRef>	numberRef = mInternals->get(pref.getKeyString());

	return numberRef.hasObject() ? CCoreFoundation::sInt16From((CFNumberRef) *numberRef) : pref.getDefaultValue();
}

//----------------------------------------------------------------------------------------------------------------------
SInt32 CPreferences::getSInt32(const SInt32Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	CCoreFoundation::OO<CFTypeRef>	numberRef = mInternals->get(pref.getKeyString());

	return numberRef.hasObject() ? CCoreFoundation::sInt32From((CFNumberRef) *numberRef) : pref.getDefaultValue();
}

//----------------------------------------------------------------------------------------------------------------------
SInt64 CPreferences::getSInt64(const SInt64Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	CCoreFoundation::OO<CFTypeRef>	numberRef = mInternals->get(pref.getKeyString());

	return numberRef.hasObject() ? CCoreFoundation::sInt64From((CFNumberRef) *numberRef) : pref.getDefaultValue();
}

//----------------------------------------------------------------------------------------------------------------------
UInt8 CPreferences::getUInt8(const UInt8Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	CCoreFoundation::OO<CFTypeRef>	numberRef = mInternals->get(pref.getKeyString());

	return numberRef.hasObject() ? CCoreFoundation::uInt8From((CFNumberRef) *numberRef) : pref.getDefaultValue();
}

//----------------------------------------------------------------------------------------------------------------------
UInt16 CPreferences::getUInt16(const UInt16Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	CCoreFoundation::OO<CFTypeRef>	numberRef = mInternals->get(pref.getKeyString());

	return numberRef.hasObject() ? CCoreFoundation::uInt16From((CFNumberRef) *numberRef) : pref.getDefaultValue();
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CPreferences::getUInt32(const UInt32Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	CCoreFoundation::OO<CFTypeRef>	numberRef = mInternals->get(pref.getKeyString());

	return numberRef.hasObject() ? CCoreFoundation::uInt32From((CFNumberRef) *numberRef) : pref.getDefaultValue();
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CPreferences::getUInt64(const UInt64Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	CCoreFoundation::OO<CFTypeRef>	numberRef = mInternals->get(pref.getKeyString());

	return numberRef.hasObject() ? CCoreFoundation::uInt64From((CFNumberRef) *numberRef) : pref.getDefaultValue();
}

//----------------------------------------------------------------------------------------------------------------------
UniversalTimeInterval CPreferences::getUniversalTimeInterval(const UniversalTimeIntervalPref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	CCoreFoundation::OO<CFTypeRef>	numberRef = mInternals->get(pref.getKeyString());

	return numberRef.hasObject() ? CCoreFoundation::float64From((CFNumberRef) *numberRef) : pref.getDefaultValue();
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const BoolPref& boolPref, bool value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->set(boolPref.getKeyString(), value ? kCFBooleanTrue : kCFBooleanFalse);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const Pref& pref, const TArray<CData>& array)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->set(pref.getKeyString(), *CCoreFoundation::arrayRefFrom(array));
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const Pref& pref, const TArray<CDictionary>& array)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->set(pref.getKeyString(), *CCoreFoundation::arrayRefFrom(array));
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const Pref& pref, const TNumberArray<OSType>& array)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->set(pref.getKeyString(), *CCoreFoundation::arrayRefFrom(array));
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const Pref& pref, const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->set(pref.getKeyString(), *CCoreFoundation::dataRefFrom(data));
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const Pref& pref, const CDictionary& dictionary)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->set(pref.getKeyString(), *CCoreFoundation::dictionaryRefFrom(dictionary));
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const StringPref& pref, const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->set(pref.getKeyString(), string.getOSString());
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const Float32Pref& pref, Float32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->set(pref.getKeyString(), *CCoreFoundation::numberRefFrom(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const Float64Pref& pref, Float64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->set(pref.getKeyString(), *CCoreFoundation::numberRefFrom(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SInt8Pref& pref, SInt8 value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->set(pref.getKeyString(), *CCoreFoundation::numberRefFrom(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SInt16Pref& pref, SInt16 value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->set(pref.getKeyString(), *CCoreFoundation::numberRefFrom(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SInt32Pref& pref, SInt32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->set(pref.getKeyString(), *CCoreFoundation::numberRefFrom(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SInt64Pref& pref, SInt64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->set(pref.getKeyString(), *CCoreFoundation::numberRefFrom(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const UInt8Pref& pref, UInt8 value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->set(pref.getKeyString(), *CCoreFoundation::numberRefFrom(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const UInt16Pref& pref, UInt16 value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->set(pref.getKeyString(), *CCoreFoundation::numberRefFrom(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const UInt32Pref& pref, UInt32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->set(pref.getKeyString(), *CCoreFoundation::numberRefFrom(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const UInt64Pref& pref, UInt64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->set(pref.getKeyString(), *CCoreFoundation::numberRefFrom(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const UniversalTimeIntervalPref& pref, UniversalTimeInterval value)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->set(pref.getKeyString(), *CCoreFoundation::numberRefFrom(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::remove(const Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->set(pref.getKeyString(), nil);
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
