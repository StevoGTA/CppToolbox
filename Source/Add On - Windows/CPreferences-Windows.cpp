//----------------------------------------------------------------------------------------------------------------------
//	CPreferences-Windows.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CPreferences.h"

#include "SError.h"

#if defined(__cplusplus_winrt)
	// C++/CX
	#include "CPlatform.h"

	using namespace Windows::Foundation;
	using namespace Windows::Storage;
#else
	// C++/WinRT
	#include <winrt/Windows.Foundation.Collections.h>
	#include <winrt/Windows.Storage.h>

	using namespace winrt;
	using namespace winrt::Windows::Storage;
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: CPreferences

// MARK: Properties

CPreferences	CPreferences::mDefault;

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CPreferences::CPreferences()
//----------------------------------------------------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------------------------------------------------
CPreferences::CPreferences(const Reference& reference)
//----------------------------------------------------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------------------------------------------------
CPreferences::~CPreferences()
//----------------------------------------------------------------------------------------------------------------------
{
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
bool CPreferences::hasValue(const Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
#if defined(__cplusplus_winrt)
	// C++/CX
	return ApplicationData::Current->LocalSettings->Values->HasKey(ref new String(pref.getKeyString()));
#else
	// C++/WinRT
	return ApplicationData::Current().LocalSettings().Values().HasKey(hstring(pref.getKeyString()));
#endif
}

//----------------------------------------------------------------------------------------------------------------------
bool CPreferences::getBool(const BoolPref& boolPref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
#if defined(__cplusplus_winrt)
	// C++/CX
	Object^	object = ApplicationData::Current->LocalSettings->Values->Lookup(ref new String(boolPref.getKeyString()));

	return (object != nullptr) ? safe_cast<UInt32>(object) == 1 : boolPref.getDefaultValue();
#else
	// C++/WinRT
	auto	object = ApplicationData::Current().LocalSettings().Values().Lookup(hstring(boolPref.getKeyString()));

	return (object != nullptr) ? unbox_value<int>(object) == 1 : boolPref.getDefaultValue();
#endif
}

//----------------------------------------------------------------------------------------------------------------------
OV<TArray<CData> > CPreferences::getDataArray(const Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return OV<TArray<CData> >();
}

//----------------------------------------------------------------------------------------------------------------------
OV<TArray<CDictionary> > CPreferences::getDictionaryArray(const Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return OV<TArray<CDictionary> >();
}

//----------------------------------------------------------------------------------------------------------------------
OV<TNumberArray<OSType> > CPreferences::getOSTypeArray(const Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return OV<TNumberArray<OSType> >();
}

//----------------------------------------------------------------------------------------------------------------------
OV<CData> CPreferences::getData(const Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return OV<CData>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<CDictionary> CPreferences::getDictionary(const Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return OV<CDictionary>();
}

//----------------------------------------------------------------------------------------------------------------------
CString CPreferences::getString(const StringPref& stringPref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
#if defined(__cplusplus_winrt)
	// C++/CX
	Object^	object = ApplicationData::Current->LocalSettings->Values->Lookup(ref new String(stringPref.getKeyString()));

	return (object != nullptr) ? CPlatform::stringFrom(safe_cast<String^>(object)) : stringPref.getDefaultValue();
#else
	// C++/WinRT
	auto	object = ApplicationData::Current().LocalSettings().Values().Lookup(hstring(stringPref.getKeyString()));

	return (object != nullptr) ? CString(unbox_value<hstring>(object).data()) : stringPref.getDefaultValue();
#endif
}

//----------------------------------------------------------------------------------------------------------------------
Float32 CPreferences::getFloat32(const Float32Pref& float32Pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
#if defined(__cplusplus_winrt)
	// C++/CX
	Object^	object =
					ApplicationData::Current->LocalSettings->Values->Lookup(ref new String(float32Pref.getKeyString()));

	return (object != nullptr) ? safe_cast<Float32>(object) : float32Pref.getDefaultValue();
#else
	// C++/WinRT
	auto	object = ApplicationData::Current().LocalSettings().Values().Lookup(hstring(float32Pref.getKeyString()));

	return (object != nullptr) ? unbox_value<Float32>(object) : float32Pref.getDefaultValue();
#endif
}

//----------------------------------------------------------------------------------------------------------------------
Float64 CPreferences::getFloat64(const Float64Pref& float64Pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
#if defined(__cplusplus_winrt)
	// C++/CX
	Object^	object =
					ApplicationData::Current->LocalSettings->Values->Lookup(ref new String(float64Pref.getKeyString()));

	return (object != nullptr) ? safe_cast<Float64>(object) : float64Pref.getDefaultValue();
#else
	// C++/WinRT
	auto	object = ApplicationData::Current().LocalSettings().Values().Lookup(hstring(float64Pref.getKeyString()));

	return (object != nullptr) ? unbox_value<Float64>(object) : float64Pref.getDefaultValue();
#endif
}

//----------------------------------------------------------------------------------------------------------------------
SInt32 CPreferences::getSInt32(const SInt32Pref& sInt32Pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
#if defined(__cplusplus_winrt)
	// C++/CX
	Object^	object = ApplicationData::Current->LocalSettings->Values->Lookup(ref new String(sInt32Pref.getKeyString()));

	return (object != nullptr) ? safe_cast<SInt32>(object) : sInt32Pref.getDefaultValue();
#else
	// C++/WinRT
	auto	object = ApplicationData::Current().LocalSettings().Values().Lookup(hstring(sInt32Pref.getKeyString()));

	return (object != nullptr) ? unbox_value<SInt32>(object) : sInt32Pref.getDefaultValue();
#endif
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CPreferences::getUInt32(const UInt32Pref& uInt32Pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
#if defined(__cplusplus_winrt)
	// C++/CX
	Object^	object = ApplicationData::Current->LocalSettings->Values->Lookup(ref new String(uInt32Pref.getKeyString()));

	return (object != nullptr) ? safe_cast<UInt32>(object) : uInt32Pref.getDefaultValue();
#else
	// C++/WinRT
	auto	object = ApplicationData::Current().LocalSettings().Values().Lookup(hstring(uInt32Pref.getKeyString()));

	return (object != nullptr) ? unbox_value<UInt32>(object) : uInt32Pref.getDefaultValue();
#endif
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CPreferences::getUInt64(const UInt64Pref& uInt64Pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
#if defined(__cplusplus_winrt)
	// C++/CX
	Object^	object = ApplicationData::Current->LocalSettings->Values->Lookup(ref new String(uInt64Pref.getKeyString()));

	return (object != nullptr) ? safe_cast<UInt64>(object) : uInt64Pref.getDefaultValue();
#else
	// C++/WinRT
	auto	object = ApplicationData::Current().LocalSettings().Values().Lookup(hstring(uInt64Pref.getKeyString()));

	return (object != nullptr) ? unbox_value<UInt64>(object) : uInt64Pref.getDefaultValue();
#endif
}

//----------------------------------------------------------------------------------------------------------------------
UniversalTimeInterval CPreferences::getUniversalTimeInterval(
		const UniversalTimeIntervalPref& universalTimeIntervalPref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
#if defined(__cplusplus_winrt)
	// C++/CX
	Object^	object =
					ApplicationData::Current->LocalSettings->Values->Lookup(
							ref new String(universalTimeIntervalPref.getKeyString()));

	return (object != nullptr) ? safe_cast<UniversalTimeInterval>(object) : universalTimeIntervalPref.getDefaultValue();
#else
	// C++/WinRT
	auto	object =
					ApplicationData::Current().LocalSettings().Values().Lookup(
							hstring(universalTimeIntervalPref.getKeyString()));

	return (object != nullptr) ?
			unbox_value<UniversalTimeInterval>(object) : universalTimeIntervalPref.getDefaultValue();
#endif
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const BoolPref& boolPref, bool value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Set
#if defined(__cplusplus_winrt)
	// C++/CX
	ApplicationData::Current->LocalSettings->Values->Insert(ref new String(boolPref.getKeyString()),
			dynamic_cast<PropertyValue^>(PropertyValue::CreateUInt32(value ? 1 : 0)));
#else
	// C++/WinRT
	ApplicationData::Current().LocalSettings().Values().Insert(hstring(boolPref.getKeyString()),
			box_value(value ? 1 : 0));
#endif
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
void CPreferences::set(const Pref& pref, const TNumberArray<OSType>& array)
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
void CPreferences::set(const StringPref& stringPref, const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	// Set
#if defined(__cplusplus_winrt)
	// C++/CX
	ApplicationData::Current->LocalSettings->Values->Insert(ref new String(stringPref.getKeyString()),
			dynamic_cast<PropertyValue^>(PropertyValue::CreateString(ref new String(string.getOSString()))));
#else
	// C++/WinRT
	ApplicationData::Current().LocalSettings().Values().Insert(hstring(stringPref.getKeyString()),
			box_value(hstring(string.getOSString())));
#endif
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const Float32Pref& float32Pref, Float32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Set
#if defined(__cplusplus_winrt)
	// C++/CX
	ApplicationData::Current->LocalSettings->Values->Insert(ref new String(float32Pref.getKeyString()),
			dynamic_cast<PropertyValue^>(PropertyValue::CreateSingle(value)));
#else
	// C++/WinRT
	ApplicationData::Current().LocalSettings().Values().Insert(hstring(float32Pref.getKeyString()), box_value(value));
#endif
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const Float64Pref& float64Pref, Float64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Set
#if defined(__cplusplus_winrt)
	// C++/CX
	ApplicationData::Current->LocalSettings->Values->Insert(ref new String(float64Pref.getKeyString()),
			dynamic_cast<PropertyValue^>(PropertyValue::CreateDouble(value)));
#else
	// C++/WinRT
	ApplicationData::Current().LocalSettings().Values().Insert(hstring(float64Pref.getKeyString()), box_value(value));
#endif
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SInt32Pref& sInt32Pref, SInt32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Set
#if defined(__cplusplus_winrt)
	// C++/CX
	ApplicationData::Current->LocalSettings->Values->Insert(ref new String(sInt32Pref.getKeyString()),
			dynamic_cast<PropertyValue^>(PropertyValue::CreateInt32(value)));
#else
	// C++/WinRT
	ApplicationData::Current().LocalSettings().Values().Insert(hstring(sInt32Pref.getKeyString()), box_value(value));
#endif
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const UInt32Pref& uInt32Pref, UInt32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Set
#if defined(__cplusplus_winrt)
	// C++/CX
	ApplicationData::Current->LocalSettings->Values->Insert(ref new String(uInt32Pref.getKeyString()),
			dynamic_cast<PropertyValue^>(PropertyValue::CreateUInt32(value)));
#else
	// C++/WinRT
	ApplicationData::Current().LocalSettings().Values().Insert(hstring(uInt32Pref.getKeyString()), box_value(value));
#endif
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const UInt64Pref& uInt64Pref, UInt64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Set
#if defined(__cplusplus_winrt)
	// C++/CX
	ApplicationData::Current->LocalSettings->Values->Insert(ref new String(uInt64Pref.getKeyString()),
			dynamic_cast<PropertyValue^>(PropertyValue::CreateUInt64(value)));
#else
	// C++/WinRT
	ApplicationData::Current().LocalSettings().Values().Insert(hstring(uInt64Pref.getKeyString()), box_value(value));
#endif
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const UniversalTimeIntervalPref& universalTimeIntervalPref, UniversalTimeInterval value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Set
#if defined(__cplusplus_winrt)
	// C++/CX
	ApplicationData::Current->LocalSettings->Values->Insert(ref new String(universalTimeIntervalPref.getKeyString()),
			dynamic_cast<PropertyValue^>(PropertyValue::CreateDouble(value)));
#else
	// C++/WinRT
	ApplicationData::Current().LocalSettings().Values().Insert(hstring(universalTimeIntervalPref.getKeyString()),
			box_value(value));
#endif
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::remove(const Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove
#if defined(__cplusplus_winrt)
	// C++/CX
	ApplicationData::Current->LocalSettings->Values->Remove(ref new String(pref.getKeyString()));
#else
	// C++/WinRT
	ApplicationData::Current().LocalSettings().Values().Remove(hstring(pref.getKeyString()));
#endif
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
