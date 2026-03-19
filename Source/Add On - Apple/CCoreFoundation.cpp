//----------------------------------------------------------------------------------------------------------------------
//	CCoreFoundation.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CCoreFoundation.h"

#include "CCoreServices.h"
#include "CData.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CCoreFoundation

// MARK: Array methods

//----------------------------------------------------------------------------------------------------------------------
TArray<CData> CCoreFoundation::dataArrayFrom(CFArrayRef arrayRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNArray<CData>	array;

	// Get values
	CFIndex		count = ::CFArrayGetCount(arrayRef);
	CFDataRef	dataRefs[count];
	::CFArrayGetValues(arrayRef, CFRangeMake(0, count), (const void**) &dataRefs);
	for (CFIndex i = 0; i < count; i++)
		// Add data
		array += dataFrom(dataRefs[i]);

	return array;
}

//----------------------------------------------------------------------------------------------------------------------
TArray<CDictionary> CCoreFoundation::dictionaryArrayFrom(CFArrayRef arrayRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNArray<CDictionary>	array;

	// Get values
	CFIndex			count = ::CFArrayGetCount(arrayRef);
	CFDictionaryRef	dictionaryRefs[count];
	::CFArrayGetValues(arrayRef, CFRangeMake(0, count), (const void**) &dictionaryRefs);
	for (CFIndex i = 0; i < count; i++)
		// Add dictionary
		array += dictionaryFrom(dictionaryRefs[i]);

	return array;
}

//----------------------------------------------------------------------------------------------------------------------
TNumberArray<OSType> CCoreFoundation::osTypeArrayFrom(CFArrayRef arrayRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNumberArray<OSType>	array;

	// Get values
	CFIndex		count = ::CFArrayGetCount(arrayRef);
	CFNumberRef	numberRefs[count];
	::CFArrayGetValues(arrayRef, CFRangeMake(0, count), (const void**) &numberRefs);

	for (CFIndex i = 0; i < count; i++)
		// Add number
		array += (OSType) uInt32From(numberRefs[i]);

	return array;
}

//----------------------------------------------------------------------------------------------------------------------
TArray<CString> CCoreFoundation::stringArrayFrom(CFArrayRef arrayRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNArray<CString>	array;

	// Get values
	CFIndex		count = ::CFArrayGetCount(arrayRef);
	CFStringRef	stringRefs[count];
	::CFArrayGetValues(arrayRef, CFRangeMake(0, count), (const void**) &stringRefs);
	for (CFIndex i = 0; i < count; i++)
		// Add data
		array += CString(stringRefs[i]);

	return array;
}

//----------------------------------------------------------------------------------------------------------------------
CCoreFoundation::O<CFArrayRef> CCoreFoundation::arrayRefFrom(const TArray<CData>& array)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFMutableArrayRef	arrayRef =
								::CFArrayCreateMutable(kCFAllocatorDefault, array.getCount(), &kCFTypeArrayCallBacks);
	for (TArray<CData>::Iterator iterator = array.getIterator(); iterator; iterator++)
		// Add data
		::CFArrayAppendValue(arrayRef, *dataRefFrom(*iterator));

	return O<CFArrayRef>(arrayRef);
}

//----------------------------------------------------------------------------------------------------------------------
CCoreFoundation::O<CFArrayRef> CCoreFoundation::arrayRefFrom(const TArray<CDictionary>& array)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFMutableArrayRef	arrayRef =
								::CFArrayCreateMutable(kCFAllocatorDefault, array.getCount(), &kCFTypeArrayCallBacks);
	for (TArray<CDictionary>::Iterator iterator = array.getIterator(); iterator; iterator++)
		// Add dictionary
		::CFArrayAppendValue(arrayRef, *dictionaryRefFrom(*iterator));

	return O<CFArrayRef>(arrayRef);
}

//----------------------------------------------------------------------------------------------------------------------
CCoreFoundation::O<CFArrayRef> CCoreFoundation::arrayRefFrom(const TNumberArray<OSType>& array)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFMutableArrayRef	arrayRef =
								::CFArrayCreateMutable(kCFAllocatorDefault, array.getCount(), &kCFTypeArrayCallBacks);
	for (TNumberArray<OSType>::Iterator iterator = array.getIterator(); iterator; iterator++)
		// Add string
		::CFArrayAppendValue(arrayRef, *numberRefFrom(*iterator));

	return O<CFArrayRef>(arrayRef);
}

//----------------------------------------------------------------------------------------------------------------------
CCoreFoundation::O<CFArrayRef> CCoreFoundation::arrayRefFrom(const TArray<CString>& array)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFMutableArrayRef	arrayRef =
								::CFArrayCreateMutable(kCFAllocatorDefault, array.getCount(), &kCFTypeArrayCallBacks);
	for (TArray<CString>::Iterator iterator = array.getIterator(); iterator; iterator++)
		// Add string
		::CFArrayAppendValue(arrayRef, iterator->getOSString());

	return O<CFArrayRef>(arrayRef);
}

// MARK: Data methods

//----------------------------------------------------------------------------------------------------------------------
CData CCoreFoundation::dataFrom(CFDataRef dataRef)
//----------------------------------------------------------------------------------------------------------------------
{
	return CData(::CFDataGetBytePtr(dataRef), (CData::ByteCount) ::CFDataGetLength(dataRef));
}

//----------------------------------------------------------------------------------------------------------------------
CCoreFoundation::O<CFDataRef> CCoreFoundation::dataRefFrom(const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	return O<CFDataRef>(::CFDataCreate(kCFAllocatorDefault, (const UInt8*) data.getBytePtr(), data.getByteCount()));
}

// MARK: Dictionary methods

//----------------------------------------------------------------------------------------------------------------------
CDictionary CCoreFoundation::dictionaryFrom(CFDictionaryRef dictionaryRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CDictionary	dictionary;

	// Get keys and values
	CFIndex		count = ::CFDictionaryGetCount(dictionaryRef);
	CFStringRef	keyStringRefs[count];
	CFTypeRef	valueTypeRefs[count];
	::CFDictionaryGetKeysAndValues(dictionaryRef, (const void**) &keyStringRefs, (const void**) &valueTypeRefs);

	// Add all items
	for (CFIndex i = 0; i < count; i++) {
		// What type
		CFTypeRef	valueTypeRef = valueTypeRefs[i];
		if (::CFGetTypeID(valueTypeRef) == ::CFBooleanGetTypeID())
			// Boolean
			dictionary.set(CString(keyStringRefs[i]), (CFBooleanRef) valueTypeRef == ::kCFBooleanTrue);
		else if (::CFGetTypeID(valueTypeRef) == ::CFArrayGetTypeID()) {
			// Array
			CFArrayRef	arrayRef = (CFArrayRef) valueTypeRef;
			if (::CFArrayGetCount(arrayRef) > 0) {
				// What type
				CFTypeID	arrayElementTypeRef = ::CFGetTypeID(::CFArrayGetValueAtIndex(arrayRef, 0));
				if (arrayElementTypeRef == ::CFDictionaryGetTypeID())
					// Array of dictionaries
					dictionary.set(CString(keyStringRefs[i]), dictionaryArrayFrom(arrayRef));
				else if (arrayElementTypeRef == ::CFStringGetTypeID())
					// Array of strings
					dictionary.set(CString(keyStringRefs[i]), stringArrayFrom(arrayRef));
				else
					// Uh oh
					CCoreServices::stopInDebugger();
			}
		} else if (::CFGetTypeID(valueTypeRef) == ::CFDataGetTypeID())
			// Data
			dictionary.set(CString(keyStringRefs[i]), dataFrom((CFDataRef) valueTypeRef));
		else if (::CFGetTypeID(valueTypeRef) == ::CFDictionaryGetTypeID())
			// Dictionary
			dictionary.set(CString(keyStringRefs[i]), dictionaryFrom((CFDictionaryRef) valueTypeRef));
		else if (::CFGetTypeID(valueTypeRef) == ::CFNumberGetTypeID()) {
			// Number
			switch (::CFNumberGetType((CFNumberRef) valueTypeRef)) {
				case kCFNumberFloat32Type:
				case kCFNumberFloatType:
					// Float 32
					dictionary.set(CString(keyStringRefs[i]), float32From((CFNumberRef) valueTypeRef));
					break;

				case kCFNumberFloat64Type:
				case kCFNumberDoubleType:
					// Float 64
					dictionary.set(CString(keyStringRefs[i]), float64From((CFNumberRef) valueTypeRef));
					break;

				case kCFNumberSInt8Type:
				case kCFNumberCharType:
					// SInt8
					dictionary.set(CString(keyStringRefs[i]), sInt8From((CFNumberRef) valueTypeRef));
					break;

				case kCFNumberSInt16Type:
				case kCFNumberShortType:
					// SInt16
					dictionary.set(CString(keyStringRefs[i]), sInt16From((CFNumberRef) valueTypeRef));
					break;

				case kCFNumberSInt32Type:
				case kCFNumberIntType:
					// SInt32
					dictionary.set(CString(keyStringRefs[i]), sInt32From((CFNumberRef) valueTypeRef));
					break;

				case kCFNumberSInt64Type:
				case kCFNumberLongType:
				case kCFNumberLongLongType:
					// SInt64
					dictionary.set(CString(keyStringRefs[i]), sInt64From((CFNumberRef) valueTypeRef));
					break;

				default:
					// The rest are unsupported (unseen actually)
					break;
			}
		} else if (::CFGetTypeID(valueTypeRef) == ::CFStringGetTypeID())
			// String
			dictionary.set(CString(keyStringRefs[i]), CString((CFStringRef) valueTypeRef));
		else {
			// Uh oh
			CCoreServices::stopInDebugger();
		}
	}

	return dictionary;
}

//----------------------------------------------------------------------------------------------------------------------
TDictionary<CString> CCoreFoundation::dictionaryOfStringsFrom(CFDictionaryRef dictionaryRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNDictionary<CString>	dictionary;

	// Get keys and values
	CFIndex		count = ::CFDictionaryGetCount(dictionaryRef);
	CFStringRef	keyStringRefs[count];
	CFTypeRef	valueTypeRefs[count];
	::CFDictionaryGetKeysAndValues(dictionaryRef, (const void**) &keyStringRefs, (const void**) &valueTypeRefs);

	// Add all items
	for (CFIndex i = 0; i < count; i++) {
		// What type
		CFTypeRef	valueTypeRef = valueTypeRefs[i];
		if (::CFGetTypeID(valueTypeRef) == ::CFStringGetTypeID())
			// String
			dictionary.set(CString(keyStringRefs[i]), CString((CFStringRef) valueTypeRef));
		else {
			// Uh oh
			CCoreServices::stopInDebugger();
		}
	}

	return dictionary;
}

//----------------------------------------------------------------------------------------------------------------------
CCoreFoundation::O<CFDictionaryRef> CCoreFoundation::dictionaryRefFrom(const CDictionary& dictionary)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFMutableDictionaryRef	dictionaryRef =
									::CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,
											&kCFTypeDictionaryValueCallBacks);

	// Copy all items
	for (CDictionary::Iterator iterator = dictionary.getIterator(); iterator; iterator++) {
		// Get info
		const	CString&	key = iterator.getKey();
				CFStringRef	keyStringRef = key.getOSString();

		// Store value in dictionary
		const	SValue&	value = iterator.getValue();
		switch (value.getType()) {
			case SValue::kTypeEmpty:
				// Empty (null)
				::CFDictionarySetValue(dictionaryRef, keyStringRef, kCFNull);
				break;

			case SValue::kTypeBool:
				// Bool
				::CFDictionarySetValue(dictionaryRef, keyStringRef, value.getBool() ? kCFBooleanTrue : kCFBooleanFalse);
				break;

			case SValue::kTypeArrayOfDictionaries:
				// Array of dictionaries
				::CFDictionarySetValue(dictionaryRef, keyStringRef, *arrayRefFrom(value.getArrayOfDictionaries()));
				break;

			case SValue::kTypeArrayOfStrings:
				// Array of strings
				::CFDictionarySetValue(dictionaryRef, keyStringRef, *arrayRefFrom(value.getArrayOfStrings()));
				break;

			case SValue::kTypeData:
				// Data
				::CFDictionarySetValue(dictionaryRef, keyStringRef, *dataRefFrom(value.getData()));
				break;

			case SValue::kTypeDictionary:
				// Dictionary
				::CFDictionarySetValue(dictionaryRef, keyStringRef, *dictionaryRefFrom(value.getDictionary()));
				break;

			case SValue::kTypeString:
				// String
				::CFDictionarySetValue(dictionaryRef, keyStringRef, value.getString().getOSString());
				break;

			case SValue::kTypeFloat32:
				// Float32
				::CFDictionarySetValue(dictionaryRef, keyStringRef, *numberRefFrom(value.getFloat32()));
				break;

			case SValue::kTypeFloat64:
				// Float64
				::CFDictionarySetValue(dictionaryRef, keyStringRef, *numberRefFrom(value.getFloat64()));
				break;

			case SValue::kTypeSInt8:
				// SInt8
				::CFDictionarySetValue(dictionaryRef, keyStringRef, *numberRefFrom(value.getSInt8()));
				break;

			case SValue::kTypeSInt16:
				// SInt16
				::CFDictionarySetValue(dictionaryRef, keyStringRef, *numberRefFrom(value.getSInt16()));
				break;

			case SValue::kTypeSInt32:
				// SInt32
				::CFDictionarySetValue(dictionaryRef, keyStringRef, *numberRefFrom(value.getSInt32()));
				break;

			case SValue::kTypeSInt64:
				// SInt64
				::CFDictionarySetValue(dictionaryRef, keyStringRef, *numberRefFrom(value.getSInt64()));
				break;

			case SValue::kTypeUInt8:
				// UInt8
				::CFDictionarySetValue(dictionaryRef, keyStringRef, *numberRefFrom(value.getUInt8()));
				break;

			case SValue::kTypeUInt16:
				// UInt16
				::CFDictionarySetValue(dictionaryRef, keyStringRef, *numberRefFrom(value.getUInt16()));
				break;

			case SValue::kTypeUInt32:
				// UInt32
				::CFDictionarySetValue(dictionaryRef, keyStringRef, *numberRefFrom(value.getUInt32()));
				break;

			case SValue::kTypeUInt64:
				// UInt64
				::CFDictionarySetValue(dictionaryRef, keyStringRef, *numberRefFrom(value.getUInt64()));
				break;

			case SValue::kTypeOpaque:
				// Something else that cannot be represented by Core Foundation
				break;
		}
	}

	return dictionaryRef;
}

//----------------------------------------------------------------------------------------------------------------------
CCoreFoundation::O<CFDictionaryRef> CCoreFoundation::dictionaryRefFrom(const TDictionary<CString>& dictionary)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFMutableDictionaryRef	dictionaryRef =
									::CFDictionaryCreateMutable(kCFAllocatorDefault, 0, &kCFTypeDictionaryKeyCallBacks,
											&kCFTypeDictionaryValueCallBacks);

	// Copy all items
	for (TDictionary<CString>::Iterator iterator = dictionary.getIterator(); iterator; iterator++)
		// Store value in dictionary
		::CFDictionarySetValue(dictionaryRef, iterator.getKey().getOSString(), iterator.getValue().getOSString());

	return O<CFDictionaryRef>(dictionaryRef);
}

// MAKR: Number methods

//----------------------------------------------------------------------------------------------------------------------
Float32 CCoreFoundation::float32From(CFNumberRef numberRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	Float32	value;
	::CFNumberGetValue(numberRef, kCFNumberFloat32Type, &value);

	return value;
}

//----------------------------------------------------------------------------------------------------------------------
Float64 CCoreFoundation::float64From(CFNumberRef numberRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	Float64	value;
	::CFNumberGetValue(numberRef, kCFNumberFloat64Type, &value);

	return value;
}

//----------------------------------------------------------------------------------------------------------------------
SInt8 CCoreFoundation::sInt8From(CFNumberRef numberRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SInt8	value;
	::CFNumberGetValue(numberRef, kCFNumberSInt8Type, &value);

	return value;
}

//----------------------------------------------------------------------------------------------------------------------
SInt16 CCoreFoundation::sInt16From(CFNumberRef numberRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SInt16	value;
	::CFNumberGetValue(numberRef, kCFNumberSInt16Type, &value);

	return value;
}

//----------------------------------------------------------------------------------------------------------------------
SInt32 CCoreFoundation::sInt32From(CFNumberRef numberRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SInt32	value;
	::CFNumberGetValue(numberRef, kCFNumberSInt32Type, &value);

	return value;
}

//----------------------------------------------------------------------------------------------------------------------
SInt64 CCoreFoundation::sInt64From(CFNumberRef numberRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SInt64	value;
	::CFNumberGetValue(numberRef, kCFNumberSInt64Type, &value);

	return value;
}

//----------------------------------------------------------------------------------------------------------------------
UInt8 CCoreFoundation::uInt8From(CFNumberRef numberRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SInt64	value;
	::CFNumberGetValue(numberRef, kCFNumberSInt64Type, &value);

	return (UInt8) value;
}

//----------------------------------------------------------------------------------------------------------------------
UInt16 CCoreFoundation::uInt16From(CFNumberRef numberRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SInt64	value;
	::CFNumberGetValue(numberRef, kCFNumberSInt64Type, &value);

	return (UInt16) value;
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CCoreFoundation::uInt32From(CFNumberRef numberRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SInt64	value;
	::CFNumberGetValue(numberRef, kCFNumberSInt64Type, &value);

	return (UInt32) value;
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CCoreFoundation::uInt64From(CFNumberRef numberRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get value
	SInt64	value;
	::CFNumberGetValue(numberRef, kCFNumberSInt64Type, &value);

	return (UInt64) value;
}

//----------------------------------------------------------------------------------------------------------------------
CCoreFoundation::O<CFNumberRef> CCoreFoundation::numberRefFrom(Float32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	return O<CFNumberRef>(::CFNumberCreate(kCFAllocatorDefault, kCFNumberFloat32Type, &value));
}

//----------------------------------------------------------------------------------------------------------------------
CCoreFoundation::O<CFNumberRef> CCoreFoundation::numberRefFrom(Float64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	return O<CFNumberRef>(::CFNumberCreate(kCFAllocatorDefault, kCFNumberFloat64Type, &value));
}

//----------------------------------------------------------------------------------------------------------------------
CCoreFoundation::O<CFNumberRef> CCoreFoundation::numberRefFrom(SInt8 value)
//----------------------------------------------------------------------------------------------------------------------
{
	return O<CFNumberRef>(::CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt8Type, &value));
}

//----------------------------------------------------------------------------------------------------------------------
CCoreFoundation::O<CFNumberRef> CCoreFoundation::numberRefFrom(SInt16 value)
//----------------------------------------------------------------------------------------------------------------------
{
	return O<CFNumberRef>(::CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt16Type, &value));
}

//----------------------------------------------------------------------------------------------------------------------
CCoreFoundation::O<CFNumberRef> CCoreFoundation::numberRefFrom(SInt32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	return O<CFNumberRef>(::CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt32Type, &value));
}

//----------------------------------------------------------------------------------------------------------------------
CCoreFoundation::O<CFNumberRef> CCoreFoundation::numberRefFrom(SInt64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	return O<CFNumberRef>(::CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type, &value));
}

//----------------------------------------------------------------------------------------------------------------------
CCoreFoundation::O<CFNumberRef> CCoreFoundation::numberRefFrom(UInt8 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Cast
	SInt64	sInt64Value = value;

	return O<CFNumberRef>(::CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type, &sInt64Value));
}

//----------------------------------------------------------------------------------------------------------------------
CCoreFoundation::O<CFNumberRef> CCoreFoundation::numberRefFrom(UInt16 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Cast
	SInt64	sInt64Value = value;

	return O<CFNumberRef>(::CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type, &sInt64Value));
}

//----------------------------------------------------------------------------------------------------------------------
CCoreFoundation::O<CFNumberRef> CCoreFoundation::numberRefFrom(UInt32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Cast
	SInt64	sInt64Value = value;

	return O<CFNumberRef>(::CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type, &sInt64Value));
}

//----------------------------------------------------------------------------------------------------------------------
CCoreFoundation::O<CFNumberRef> CCoreFoundation::numberRefFrom(UInt64 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Cast
	SInt64	sInt64Value = value;

	return O<CFNumberRef>(::CFNumberCreate(kCFAllocatorDefault, kCFNumberSInt64Type, &sInt64Value));
}

// MARK: Set methods

//----------------------------------------------------------------------------------------------------------------------
TSet<CString> CCoreFoundation::setOfStringsFrom(const CFSetRef setRef)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNSet<CString>	set;

	// Get values
	CFIndex		count = ::CFSetGetCount(setRef);
	CFStringRef	stringRefs[count];
	::CFSetGetValues(setRef, (const void**) stringRefs);
	for (CFIndex i = 0; i < count; i++)
		// Add data
		set += CString(stringRefs[i]);

	return set;
}
