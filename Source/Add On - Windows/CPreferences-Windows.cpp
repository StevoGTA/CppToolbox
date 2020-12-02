//----------------------------------------------------------------------------------------------------------------------
//	CPreferences-Windows.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CPreferences.h"

#include "CPlatform.h"

using namespace Windows::Foundation;
using namespace Windows::Storage;

//----------------------------------------------------------------------------------------------------------------------
// MARK: CPreferencesInternals

class CPreferencesInternals {
	public:
		CPreferencesInternals(const SPreferencesReference& preferencesReference) {}
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CPreferences

// MARK: Properties

CPreferences	CPreferences::mDefault(SPreferencesReference(0));

// MARK: Lifecycle methods

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
	return ApplicationData::Current->LocalSettings->Values->HasKey(ref new String(pref.mKeyString));
}

//----------------------------------------------------------------------------------------------------------------------
TNArray<CData> CPreferences::getDataArray(const SPref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return TNArray<CData>();
}

//----------------------------------------------------------------------------------------------------------------------
TNArray<CDictionary> CPreferences::getDictionaryArray(const SPref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return TNArray<CDictionary>();
}

//----------------------------------------------------------------------------------------------------------------------
TNumericArray<OSType> CPreferences::getOSTypeArray(const SPref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return TNumericArray<OSType>();
}

//----------------------------------------------------------------------------------------------------------------------
CData CPreferences::getData(const SPref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return CData::mEmpty;
}

//----------------------------------------------------------------------------------------------------------------------
CDictionary CPreferences::getDictionary(const SPref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return CDictionary::mEmpty;
}

//----------------------------------------------------------------------------------------------------------------------
CString CPreferences::getString(const SStringPref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	String^	value =
					safe_cast<String^>(ApplicationData::Current->LocalSettings->Values->Lookup(
							ref new String(pref.mKeyString)));

	return value ? CPlatform::stringFrom(value) : CString::mEmpty;
}

//----------------------------------------------------------------------------------------------------------------------
Float32 CPreferences::getFloat32(const SFloat32Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	return safe_cast<Float32>(ApplicationData::Current->LocalSettings->Values->Lookup(ref new String(pref.mKeyString)));
}

//----------------------------------------------------------------------------------------------------------------------
Float64 CPreferences::getFloat64(const SFloat64Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	return safe_cast<Float64>(ApplicationData::Current->LocalSettings->Values->Lookup(ref new String(pref.mKeyString)));
}

//----------------------------------------------------------------------------------------------------------------------
SInt32 CPreferences::getSInt32(const SSInt32Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	return safe_cast<SInt32>(ApplicationData::Current->LocalSettings->Values->Lookup(ref new String(pref.mKeyString)));
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CPreferences::getUInt32(const SUInt32Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	return safe_cast<UInt32>(ApplicationData::Current->LocalSettings->Values->Lookup(ref new String(pref.mKeyString)));
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CPreferences::getUInt64(const SUInt64Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	return safe_cast<UInt64>(ApplicationData::Current->LocalSettings->Values->Lookup(ref new String(pref.mKeyString)));
}

//----------------------------------------------------------------------------------------------------------------------
UniversalTimeInterval CPreferences::getUniversalTimeInterval(const SUniversalTimeIntervalPref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	return safe_cast<UniversalTimeInterval>(
			ApplicationData::Current->LocalSettings->Values->Lookup(ref new String(pref.mKeyString)));
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SPref& pref, const TArray<CData>& array)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SPref& pref, const TArray<CDictionary>& array)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SPref& pref, const TNumericArray<OSType>& array)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SPref& pref, const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SPref& pref, const CDictionary& dictionary)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SStringPref& pref, const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	// Set
	ApplicationData::Current->LocalSettings->Values->Insert(ref new String(pref.mKeyString),
			dynamic_cast<PropertyValue^>(PropertyValue::CreateString(ref new String(string.getOSString()))));
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SFloat32Pref& pref, Float32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Set
	ApplicationData::Current->LocalSettings->Values->Insert(ref new String(pref.mKeyString),
			dynamic_cast<PropertyValue^>(PropertyValue::CreateSingle(value)));
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SFloat64Pref& pref, Float64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Set
	ApplicationData::Current->LocalSettings->Values->Insert(ref new String(pref.mKeyString),
			dynamic_cast<PropertyValue^>(PropertyValue::CreateDouble(value)));
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SSInt32Pref& pref, SInt32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Set
	ApplicationData::Current->LocalSettings->Values->Insert(ref new String(pref.mKeyString),
			dynamic_cast<PropertyValue^>(PropertyValue::CreateInt32(value)));
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SUInt32Pref& pref, UInt32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Set
	ApplicationData::Current->LocalSettings->Values->Insert(ref new String(pref.mKeyString),
			dynamic_cast<PropertyValue^>(PropertyValue::CreateUInt32(value)));
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SUInt64Pref& pref, UInt64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Set
	ApplicationData::Current->LocalSettings->Values->Insert(ref new String(pref.mKeyString),
			dynamic_cast<PropertyValue^>(PropertyValue::CreateUInt64(value)));
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SUniversalTimeIntervalPref& pref, UniversalTimeInterval value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Set
	ApplicationData::Current->LocalSettings->Values->Insert(ref new String(pref.mKeyString),
			dynamic_cast<PropertyValue^>(PropertyValue::CreateDouble(value)));
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::remove(const SPref& pref)
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
void CPreferences::setAlternate(const SPreferencesReference& preferencesReference)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
}
