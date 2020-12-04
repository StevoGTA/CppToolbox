//----------------------------------------------------------------------------------------------------------------------
//	CPreferences-Windows.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CPreferences.h"

#include "CPlatform.h"
#include "SError.h"

using namespace Windows::Foundation;
using namespace Windows::Storage;

//----------------------------------------------------------------------------------------------------------------------
// MARK: CPreferencesInternals

class CPreferencesInternals {
	public:
		CPreferencesInternals(const CPreferences::Reference& reference) {}
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CPreferences

// MARK: Properties

CPreferences	CPreferences::mDefault(Reference(0));

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CPreferences::CPreferences(const Reference& reference)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CPreferencesInternals(reference);
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
	return ApplicationData::Current->LocalSettings->Values->HasKey(ref new String(pref.mKeyString));
}

//----------------------------------------------------------------------------------------------------------------------
TNArray<CData> CPreferences::getDataArray(const Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return TNArray<CData>();
}

//----------------------------------------------------------------------------------------------------------------------
TNArray<CDictionary> CPreferences::getDictionaryArray(const Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return TNArray<CDictionary>();
}

//----------------------------------------------------------------------------------------------------------------------
TNumericArray<OSType> CPreferences::getOSTypeArray(const Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return TNumericArray<OSType>();
}

//----------------------------------------------------------------------------------------------------------------------
CData CPreferences::getData(const Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return CData::mEmpty;
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary CPreferences::getDictionary(const Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return CDictionary::mEmpty;
}

//----------------------------------------------------------------------------------------------------------------------
CString CPreferences::getString(const StringPref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	String^	value =
					safe_cast<String^>(ApplicationData::Current->LocalSettings->Values->Lookup(
							ref new String(pref.mKeyString)));

	return value ? CPlatform::stringFrom(value) : CString::mEmpty;
}

//----------------------------------------------------------------------------------------------------------------------
Float32 CPreferences::getFloat32(const Float32Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	return safe_cast<Float32>(ApplicationData::Current->LocalSettings->Values->Lookup(ref new String(pref.mKeyString)));
}

//----------------------------------------------------------------------------------------------------------------------
Float64 CPreferences::getFloat64(const Float64Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	return safe_cast<Float64>(ApplicationData::Current->LocalSettings->Values->Lookup(ref new String(pref.mKeyString)));
}

//----------------------------------------------------------------------------------------------------------------------
SInt32 CPreferences::getSInt32(const SInt32Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	return safe_cast<SInt32>(ApplicationData::Current->LocalSettings->Values->Lookup(ref new String(pref.mKeyString)));
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CPreferences::getUInt32(const UInt32Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	return safe_cast<UInt32>(ApplicationData::Current->LocalSettings->Values->Lookup(ref new String(pref.mKeyString)));
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CPreferences::getUInt64(const UInt64Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	return safe_cast<UInt64>(ApplicationData::Current->LocalSettings->Values->Lookup(ref new String(pref.mKeyString)));
}

//----------------------------------------------------------------------------------------------------------------------
UniversalTimeInterval CPreferences::getUniversalTimeInterval(const UniversalTimeIntervalPref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	return safe_cast<UniversalTimeInterval>(
			ApplicationData::Current->LocalSettings->Values->Lookup(ref new String(pref.mKeyString)));
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const Pref& pref, const TArray<CData>& array)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const Pref& pref, const TArray<CDictionary>& array)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const Pref& pref, const TNumericArray<OSType>& array)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const Pref& pref, const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const Pref& pref, const CDictionary& dictionary)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const StringPref& pref, const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	// Set
	ApplicationData::Current->LocalSettings->Values->Insert(ref new String(pref.mKeyString),
			dynamic_cast<PropertyValue^>(PropertyValue::CreateString(ref new String(string.getOSString()))));
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const Float32Pref& pref, Float32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Set
	ApplicationData::Current->LocalSettings->Values->Insert(ref new String(pref.mKeyString),
			dynamic_cast<PropertyValue^>(PropertyValue::CreateSingle(value)));
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const Float64Pref& pref, Float64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Set
	ApplicationData::Current->LocalSettings->Values->Insert(ref new String(pref.mKeyString),
			dynamic_cast<PropertyValue^>(PropertyValue::CreateDouble(value)));
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SInt32Pref& pref, SInt32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Set
	ApplicationData::Current->LocalSettings->Values->Insert(ref new String(pref.mKeyString),
			dynamic_cast<PropertyValue^>(PropertyValue::CreateInt32(value)));
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const UInt32Pref& pref, UInt32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Set
	ApplicationData::Current->LocalSettings->Values->Insert(ref new String(pref.mKeyString),
			dynamic_cast<PropertyValue^>(PropertyValue::CreateUInt32(value)));
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const UInt64Pref& pref, UInt64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Set
	ApplicationData::Current->LocalSettings->Values->Insert(ref new String(pref.mKeyString),
			dynamic_cast<PropertyValue^>(PropertyValue::CreateUInt64(value)));
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const UniversalTimeIntervalPref& pref, UniversalTimeInterval value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Set
	ApplicationData::Current->LocalSettings->Values->Insert(ref new String(pref.mKeyString),
			dynamic_cast<PropertyValue^>(PropertyValue::CreateDouble(value)));
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::remove(const Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove
	ApplicationData::Current->LocalSettings->Values->Remove(ref new String(pref.mKeyString));
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::beginGroupSet()
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::endGroupSet()
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::setAlternate(const Reference& reference)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
}
