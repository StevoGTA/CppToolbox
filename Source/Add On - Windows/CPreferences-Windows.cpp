//----------------------------------------------------------------------------------------------------------------------
//	CPreferences-Windows.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CPreferences.h"

#include "CFileDataSource.h"
#include "CFileWriter.h"
#include "CFolder.h"
#include "CJSON.h"
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
// MARK: Local data

// Structured values (data, dictionaries, and arrays of them) are stored as files in the local folder since the
//	settings container only takes WinRT base types and limits each value to 8 KB.
static	const	CString	sPreferencesFolderName(OSSTR("Preferences"));
static	const	CString	sJSONExtension(OSSTR("json"));
static	const	CString	sDataExtension(OSSTR("data"));
static	const	CString	sValuesKey(OSSTR("values"));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local method declarations

static	CFile		sFileFor(const CString& key, const CString& extension);
static	OV<CData>	sReadData(const CString& key, const CString& extension);
static	void		sWriteData(const CString& key, const CString& extension, const CData& data);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CPreferences

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
	// Check settings
#if defined(__cplusplus_winrt)
	// C++/CX
	if (ApplicationData::Current->LocalSettings->Values->HasKey(ref new String(pref.getKeyString())))
		return true;
#else
	// C++/WinRT
	if (ApplicationData::Current().LocalSettings().Values().HasKey(hstring(pref.getKeyString())))
		return true;
#endif

	// Check files
	return sFileFor(pref.getKeyString(), sJSONExtension).doesExist() ||
			sFileFor(pref.getKeyString(), sDataExtension).doesExist();
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
	// Get dictionary (stored as an array of Base64 strings)
	OV<CDictionary>	dictionary = getDictionary(pref);
	if (!dictionary.hasValue())
		// No value
		return OV<TArray<CData> >();

	// Decode
	return OV<TArray<CData> >(
			TNArray<CData>(dictionary->getArrayOfStrings(sValuesKey),
					(TNArray<CData>::MapProc) CData::fromBase64StringPtr));
}

//----------------------------------------------------------------------------------------------------------------------
OV<TArray<CDictionary> > CPreferences::getDictionaryArray(const Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Read file
	OV<CData>	data = sReadData(pref.getKeyString(), sJSONExtension);
	if (!data.hasValue())
		// No value
		return OV<TArray<CDictionary> >();

	// Decode
	TVResult<TArray<CDictionary> >	result = CJSON::arrayOfDictionariesFrom(*data);
	if (!result.hasValue())
		// No value
		return OV<TArray<CDictionary> >();

	return OV<TArray<CDictionary> >(*result);
}

//----------------------------------------------------------------------------------------------------------------------
OV<TNumberArray<OSType> > CPreferences::getOSTypeArray(const Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get dictionary (stored as an array of UInt32s)
	OV<CDictionary>	dictionary = getDictionary(pref);

	return dictionary.hasValue() ? dictionary->getOVArrayOfUInt32s(sValuesKey) : OV<TNumberArray<OSType> >();
}

//----------------------------------------------------------------------------------------------------------------------
OV<CData> CPreferences::getData(const Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Read file
	return sReadData(pref.getKeyString(), sDataExtension);
}

//----------------------------------------------------------------------------------------------------------------------
OV<CDictionary> CPreferences::getDictionary(const Pref& pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Read file
	OV<CData>	data = sReadData(pref.getKeyString(), sJSONExtension);
	if (!data.hasValue())
		// No value
		return OV<CDictionary>();

	// Decode
	TVResult<CDictionary>	result = CJSON::dictionaryFrom(*data);
	if (!result.hasValue())
		// No value
		return OV<CDictionary>();

	return OV<CDictionary>(*result);
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
SInt8 CPreferences::getSInt8(const SInt8Pref& sInt8Pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
#if defined(__cplusplus_winrt)
	// C++/CX
	Object^	object = ApplicationData::Current->LocalSettings->Values->Lookup(ref new String(sInt8Pref.getKeyString()));

	return (object != nullptr) ? safe_cast<SInt8>(object) : sInt8Pref.getDefaultValue();
#else
	// C++/WinRT
	auto	object = ApplicationData::Current().LocalSettings().Values().Lookup(hstring(sInt8Pref.getKeyString()));

	return (object != nullptr) ? unbox_value<SInt8>(object) : sInt8Pref.getDefaultValue();
#endif
}

//----------------------------------------------------------------------------------------------------------------------
SInt16 CPreferences::getSInt16(const SInt16Pref& sInt16Pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
#if defined(__cplusplus_winrt)
	// C++/CX
	Object^	object = ApplicationData::Current->LocalSettings->Values->Lookup(ref new String(sInt16Pref.getKeyString()));

	return (object != nullptr) ? safe_cast<SInt16>(object) : sInt16Pref.getDefaultValue();
#else
	// C++/WinRT
	auto	object = ApplicationData::Current().LocalSettings().Values().Lookup(hstring(sInt16Pref.getKeyString()));

	return (object != nullptr) ? unbox_value<SInt16>(object) : sInt16Pref.getDefaultValue();
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
SInt64 CPreferences::getSInt64(const SInt64Pref& sInt64Pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
#if defined(__cplusplus_winrt)
	// C++/CX
	Object^	object = ApplicationData::Current->LocalSettings->Values->Lookup(ref new String(sInt64Pref.getKeyString()));

	return (object != nullptr) ? safe_cast<SInt64>(object) : sInt64Pref.getDefaultValue();
#else
	// C++/WinRT
	auto	object = ApplicationData::Current().LocalSettings().Values().Lookup(hstring(sInt64Pref.getKeyString()));

	return (object != nullptr) ? unbox_value<SInt64>(object) : sInt64Pref.getDefaultValue();
#endif
}

//----------------------------------------------------------------------------------------------------------------------
UInt8 CPreferences::getUInt8(const UInt8Pref& uInt8Pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
#if defined(__cplusplus_winrt)
	// C++/CX
	Object^	object = ApplicationData::Current->LocalSettings->Values->Lookup(ref new String(uInt8Pref.getKeyString()));

	return (object != nullptr) ? safe_cast<UInt8>(object) : uInt8Pref.getDefaultValue();
#else
	// C++/WinRT
	auto	object = ApplicationData::Current().LocalSettings().Values().Lookup(hstring(uInt8Pref.getKeyString()));

	return (object != nullptr) ? unbox_value<UInt8>(object) : uInt8Pref.getDefaultValue();
#endif
}

//----------------------------------------------------------------------------------------------------------------------
UInt16 CPreferences::getUInt16(const UInt16Pref& uInt16Pref)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
#if defined(__cplusplus_winrt)
	// C++/CX
	Object^	object = ApplicationData::Current->LocalSettings->Values->Lookup(ref new String(uInt16Pref.getKeyString()));

	return (object != nullptr) ? safe_cast<UInt16>(object) : uInt16Pref.getDefaultValue();
#else
	// C++/WinRT
	auto	object = ApplicationData::Current().LocalSettings().Values().Lookup(hstring(uInt16Pref.getKeyString()));

	return (object != nullptr) ? unbox_value<UInt16>(object) : uInt16Pref.getDefaultValue();
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
	// Compose dictionary (stored as an array of Base64 strings)
	CDictionary	dictionary;
	dictionary.set(sValuesKey, TNArray<CString>(array, (TNArray<CString>::MapProc) CData::toBase64String));

	// Set
	set(pref, dictionary);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const Pref& pref, const TArray<CDictionary>& array)
//----------------------------------------------------------------------------------------------------------------------
{
	// Write file
	sWriteData(pref.getKeyString(), sJSONExtension, *CJSON::dataFrom(array));
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const Pref& pref, const TNumberArray<OSType>& array)
//----------------------------------------------------------------------------------------------------------------------
{
	// Compose dictionary (stored as an array of UInt32s)
	CDictionary	dictionary;
	dictionary.set(sValuesKey, array);

	// Set
	set(pref, dictionary);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const Pref& pref, const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	// Write file
	sWriteData(pref.getKeyString(), sDataExtension, data);
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const Pref& pref, const CDictionary& dictionary)
//----------------------------------------------------------------------------------------------------------------------
{
	// Write file
	sWriteData(pref.getKeyString(), sJSONExtension, *CJSON::dataFrom(dictionary));
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
void CPreferences::set(const SInt8Pref& sInt8Pref, SInt8 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Set
#if defined(__cplusplus_winrt)
	// C++/CX
	ApplicationData::Current->LocalSettings->Values->Insert(ref new String(sInt8Pref.getKeyString()),
			dynamic_cast<PropertyValue^>(PropertyValue::CreateInt8(value)));
#else
	// C++/WinRT
	ApplicationData::Current().LocalSettings().Values().Insert(hstring(sInt8Pref.getKeyString()), box_value(value));
#endif
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const SInt16Pref& sInt16Pref, SInt16 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Set
#if defined(__cplusplus_winrt)
	// C++/CX
	ApplicationData::Current->LocalSettings->Values->Insert(ref new String(sInt16Pref.getKeyString()),
			dynamic_cast<PropertyValue^>(PropertyValue::CreateInt16(value)));
#else
	// C++/WinRT
	ApplicationData::Current().LocalSettings().Values().Insert(hstring(sInt16Pref.getKeyString()), box_value(value));
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
void CPreferences::set(const SInt64Pref& sInt64Pref, SInt64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Set
#if defined(__cplusplus_winrt)
	// C++/CX
	ApplicationData::Current->LocalSettings->Values->Insert(ref new String(sInt64Pref.getKeyString()),
			dynamic_cast<PropertyValue^>(PropertyValue::CreateInt64(value)));
#else
	// C++/WinRT
	ApplicationData::Current().LocalSettings().Values().Insert(hstring(sInt64Pref.getKeyString()), box_value(value));
#endif
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const UInt8Pref& uInt8Pref, UInt8 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Set
#if defined(__cplusplus_winrt)
	// C++/CX
	ApplicationData::Current->LocalSettings->Values->Insert(ref new String(uInt8Pref.getKeyString()),
			dynamic_cast<PropertyValue^>(PropertyValue::CreateUInt8(value)));
#else
	// C++/WinRT
	ApplicationData::Current().LocalSettings().Values().Insert(hstring(uInt8Pref.getKeyString()), box_value(value));
#endif
}

//----------------------------------------------------------------------------------------------------------------------
void CPreferences::set(const UInt16Pref& uInt16Pref, UInt16 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Set
#if defined(__cplusplus_winrt)
	// C++/CX
	ApplicationData::Current->LocalSettings->Values->Insert(ref new String(uInt16Pref.getKeyString()),
			dynamic_cast<PropertyValue^>(PropertyValue::CreateUInt16(value)));
#else
	// C++/WinRT
	ApplicationData::Current().LocalSettings().Values().Insert(hstring(uInt16Pref.getKeyString()), box_value(value));
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
	// Remove from settings
#if defined(__cplusplus_winrt)
	// C++/CX
	ApplicationData::Current->LocalSettings->Values->Remove(ref new String(pref.getKeyString()));
#else
	// C++/WinRT
	ApplicationData::Current().LocalSettings().Values().Remove(hstring(pref.getKeyString()));
#endif

	// Remove files
	CFile	jsonFile = sFileFor(pref.getKeyString(), sJSONExtension);
	if (jsonFile.doesExist())
		jsonFile.remove();

	CFile	dataFile = sFileFor(pref.getKeyString(), sDataExtension);
	if (dataFile.doesExist())
		dataFile.remove();
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

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CPreferences& CPreferences::shared()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static    CPreferences* sPreferences = nil;

	// Check if first time
	if (sPreferences == nil)
		// Instantiate
		sPreferences = new CPreferences();

	return *sPreferences;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local method definitions

//----------------------------------------------------------------------------------------------------------------------
CFile sFileFor(const CString& key, const CString& extension)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
#if defined(__cplusplus_winrt)
	// C++/CX
	CString	localFolderPath = CPlatform::stringFrom(ApplicationData::Current->LocalFolder->Path);
#else
	// C++/WinRT
	CString	localFolderPath(ApplicationData::Current().LocalFolder().Path().c_str());
#endif

	return CFolder(CFilesystemPath(localFolderPath).appendingComponent(sPreferencesFolderName))
			.getFile(key + CString(OSSTR(".")) + extension);
}

//----------------------------------------------------------------------------------------------------------------------
OV<CData> sReadData(const CString& key, const CString& extension)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check file
	CFile	file = sFileFor(key, extension);
	if (!file.doesExist())
		// No value
		return OV<CData>();

	// Read file
	TVResult<CData>	result = CFileDataSource::readData(file);
	if (!result.hasValue())
		// No value
		return OV<CData>();

	return OV<CData>(*result);
}

//----------------------------------------------------------------------------------------------------------------------
void sWriteData(const CString& key, const CString& extension, const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	// Write file
	CFile		file = sFileFor(key, extension);
	OV<SError>	error = file.getFolder().create(true);
	if (!error.hasValue())
		// Write
		error = CFileWriter::write(file, data);
	if (error.hasValue())
		// Error
		AssertFailWith(*error);
}
