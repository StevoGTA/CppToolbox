//----------------------------------------------------------------------------------------------------------------------
//	CString-Apple.cpp			©2003 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CString.h"

#include "CppToolboxAssert.h"
#include "CData.h"
#include "CCoreFoundation.h"
#include "CLogServices.h"

/*
	CoreFoundation has issues doing >= 128 character to length/data when requesting ASCII or MacRoman results.
		Specifically, it will not convert µ to the correct character and replaces with the lossy character
 */

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CString	sErrorDomain(OSSTR("CString-Apple"));
static	SError	sCreateFailedError(sErrorDomain, 1, CString(OSSTR("Unable to create")));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc declarations

static	CFStringCompareFlags	sGetCFStringCompareFlagsForCStringContainsOptions(
										CString::ContainsOptions containsOptions);
static	CFStringEncoding		sGetCFStringEncodingForCStringEncoding(CString::Encoding encoding);
static	bool					sIsCharacterInSet(UTF32Char utf32Char, CString::CharacterSet characterSet);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CString

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CString::CString() : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	mStringRef = CFSTR("");
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(const CString& other) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Make copy
	mStringRef = (CFStringRef) ::CFRetain(other.mStringRef);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(const OSStringVar(initialString)) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	mStringRef = (CFStringRef) ::CFRetain(initialString);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(const void* ptr, UInt32 byteCount, Encoding encoding)
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertNotNil(ptr);

	// Check situation
	if (ptr == nil)
		// No initial string
		mStringRef = CFSTR("");
	else {
		// Use only the length specified
		char	buffer[byteCount + 1];
		::memmove(buffer, ptr, byteCount);
		buffer[byteCount] = 0;
		mStringRef =
				::CFStringCreateWithCString(kCFAllocatorDefault, buffer,
						sGetCFStringEncodingForCStringEncoding(encoding));
	}

	// Validate we have something
	if (mStringRef == nil) {
		// Have something
		LogError(sCreateFailedError, "creating CFStringRef from C string");
		mStringRef = OSSTR("<Unable to create string - likely bad characters or incorrect encoding>");
	}
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(const TBuffer<UTF32Char>& buffer)
//----------------------------------------------------------------------------------------------------------------------
{
	// Create string
	mStringRef =
			::CFStringCreateWithBytes(kCFAllocatorDefault, (UInt8*) *buffer, buffer.getCount() * sizeof(UTF32Char),
					sGetCFStringEncodingForCStringEncoding(kEncodingUTF32Native), false);

	// Validate we have something
	if (mStringRef == nil) {
		// Have something
		LogError(sCreateFailedError, "creating CFStringRef from bytes");
		mStringRef = OSSTR("<Unable to create string - likely bad characters or incorrect encoding>");
	}
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(Float32 value, UInt32 fieldSize, UInt32 digitsAfterDecimalPoint, bool padWithZeros) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check field size
	if (fieldSize == 0)
		// No field size
		mStringRef =
				::CFStringCreateWithFormat(kCFAllocatorDefault, nil, CFSTR("%0.*f"), (int) digitsAfterDecimalPoint,
						(double) value);
	else
		// Have field size
		mStringRef =
				::CFStringCreateWithFormat(kCFAllocatorDefault, nil, padWithZeros ? CFSTR("%0*.*f") : CFSTR("%*.*f"),
						(int) fieldSize, (int) digitsAfterDecimalPoint, (double) value);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(Float64 value, UInt32 fieldSize, UInt32 digitsAfterDecimalPoint, bool padWithZeros) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check field size
	if (fieldSize == 0)
		// No field size
		mStringRef =
				::CFStringCreateWithFormat(kCFAllocatorDefault, nil, CFSTR("%0.*f"), (int) digitsAfterDecimalPoint,
						value);
	else
		// Have field size
		mStringRef =
				::CFStringCreateWithFormat(kCFAllocatorDefault, nil, padWithZeros ? CFSTR("%0*.*f") : CFSTR("%*.*f"),
						(int) fieldSize, (int) digitsAfterDecimalPoint, value);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(SInt8 value, UInt32 fieldSize, bool padWithZeros) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check field size
	if (fieldSize == 0)
		// No field size
		mStringRef = ::CFStringCreateWithFormat(kCFAllocatorDefault, nil, CFSTR("%hi"), (short) value);
	else
		// Have field size
		mStringRef =
				::CFStringCreateWithFormat(kCFAllocatorDefault, nil, padWithZeros ? CFSTR("%.*hi") : CFSTR("%*hi"),
						(int) fieldSize, (short) value);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(SInt16 value, UInt32 fieldSize, bool padWithZeros) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check field size
	if (fieldSize == 0)
		// No field size
		mStringRef = ::CFStringCreateWithFormat(kCFAllocatorDefault, nil, CFSTR("%hi"), value);
	else
		// Have field size
		mStringRef =
				::CFStringCreateWithFormat(kCFAllocatorDefault, nil, padWithZeros ? CFSTR("%.*hi") : CFSTR("%*hi"),
						(int) fieldSize, value);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(SInt32 value, UInt32 fieldSize, bool padWithZeros) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check field size
	if (fieldSize == 0)
		// No field size
		mStringRef = ::CFStringCreateWithFormat(kCFAllocatorDefault, nil, CFSTR("%d"), (int) value);
	else
		// Have field size
		mStringRef =
				::CFStringCreateWithFormat(kCFAllocatorDefault, nil, padWithZeros ? CFSTR("%.*d") : CFSTR("%*d"),
						(int) fieldSize, (int) value);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(SInt64 value, UInt32 fieldSize, bool padWithZeros) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check field size
	if (fieldSize == 0)
		// No field size
		mStringRef = ::CFStringCreateWithFormat(kCFAllocatorDefault, nil, CFSTR("%qi"), value);
	else
		// Have field size
		mStringRef =
				::CFStringCreateWithFormat(kCFAllocatorDefault, nil, padWithZeros ? CFSTR("%.*qi") : CFSTR("%*qi"),
						(int) fieldSize, value);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(UInt8 value, UInt32 fieldSize, bool padWithZeros, bool makeHex) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check situation
	if (fieldSize == 0)
		// No field size
		mStringRef = ::CFStringCreateWithFormat(kCFAllocatorDefault, nil, CFSTR("%hu"), (unsigned short) value);
	else if (makeHex)
		// Making hex
		mStringRef =
				::CFStringCreateWithFormat(kCFAllocatorDefault, nil, padWithZeros ? CFSTR("%#.*x") : CFSTR("%#*x"),
						(int) fieldSize, (unsigned int) value);
	else
		// The rest
		mStringRef =
				::CFStringCreateWithFormat(kCFAllocatorDefault, nil, padWithZeros ? CFSTR("%.*hu") : CFSTR("%*hu"),
						(int) fieldSize, (unsigned short) value);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(UInt16 value, UInt32 fieldSize, bool padWithZeros, bool makeHex) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check situation
	if (fieldSize == 0)
		// No field size
		mStringRef = ::CFStringCreateWithFormat(kCFAllocatorDefault, nil, CFSTR("%hu"), value);
	else if (makeHex)
		// Making hex
		mStringRef =
				::CFStringCreateWithFormat(kCFAllocatorDefault, nil, padWithZeros ? CFSTR("%#.*x") : CFSTR("%#*x"),
						(int) fieldSize, (unsigned int) value);
	else
		// The rest
		mStringRef =
				::CFStringCreateWithFormat(kCFAllocatorDefault, nil, padWithZeros ? CFSTR("%.*hu") : CFSTR("%*hu"),
						(int) fieldSize, value);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(UInt32 value, UInt32 fieldSize, bool padWithZeros, bool makeHex) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check situation
	if (fieldSize == 0)
		// No field size
		mStringRef = ::CFStringCreateWithFormat(kCFAllocatorDefault, nil, CFSTR("%u"), (unsigned int) value);
	else if (makeHex)
		// Making hex
		mStringRef =
				::CFStringCreateWithFormat(kCFAllocatorDefault, nil, padWithZeros ? CFSTR("%#.*x") : CFSTR("%#*x"),
						(int) fieldSize, (unsigned int) value);
	else
		// The rest
		mStringRef =
				::CFStringCreateWithFormat(kCFAllocatorDefault, nil, padWithZeros ? CFSTR("%.*u") : CFSTR("%*u"),
						(int) fieldSize, (unsigned int) value);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(UInt64 value, UInt32 fieldSize, bool padWithZeros, bool makeHex) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check situation
	if (fieldSize == 0)
		// No field size
		mStringRef = ::CFStringCreateWithFormat(kCFAllocatorDefault, nil, CFSTR("%qu"), value);
	else if (makeHex)
		// Making hex
		mStringRef =
				::CFStringCreateWithFormat(kCFAllocatorDefault, nil, padWithZeros ? CFSTR("%#.*qx") : CFSTR("%#*qx"),
						(int) fieldSize, value);
	else
		// The rest
		mStringRef =
				::CFStringCreateWithFormat(kCFAllocatorDefault, nil, padWithZeros ? CFSTR("%.*qu") : CFSTR("%*qu"),
						(int) fieldSize, value);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(OSType osType, bool isOSType, bool includeQuotes) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mStringRef =
			::CFStringCreateWithFormat(kCFAllocatorDefault, nil,
					includeQuotes ? CFSTR("\'%c%c%c%c\'") : CFSTR("%c%c%c"), (osType >> 24) & 0xFF,
					(osType >> 16) & 0xFF, (osType >> 8) & 0xFF, osType & 0xFF);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(const void* pointer) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	mStringRef = ::CFStringCreateWithFormat(kCFAllocatorDefault, nil, CFSTR("%p"), pointer);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(const TArray<CString>& components, const CString& separator) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFMutableStringRef	stringRef = ::CFStringCreateMutable(kCFAllocatorDefault, 0);

	// Iterate array
	for (CArray::ItemIndex i = 0; i < components.getCount(); i++) {
		// Check if need to add separator
		if (i > 0)
			// Add separator
			::CFStringAppend(stringRef, separator.mStringRef);

		// Append this component
		::CFStringAppend(stringRef, components[i].mStringRef);
	}

	// Store
	mStringRef = stringRef;
}

//----------------------------------------------------------------------------------------------------------------------
CString::~CString()
//----------------------------------------------------------------------------------------------------------------------
{
	::CFRelease(mStringRef);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
OSStringType CString::getOSString() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mStringRef;
}

//----------------------------------------------------------------------------------------------------------------------
const CString::C CString::getUTF8String() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	Length	length =
					(Length) ::CFStringGetMaximumSizeForEncoding(::CFStringGetLength(mStringRef),
							kCFStringEncodingUTF8) + 1;

	// Get c string
	C	c(length);
	::CFStringGetCString(mStringRef, (char*) *c, length, kCFStringEncodingUTF8);

	return c;
}

//----------------------------------------------------------------------------------------------------------------------
CString::Length CString::getLength() const
//----------------------------------------------------------------------------------------------------------------------
{
	return (Length) ::CFStringGetLength(mStringRef);
}

//----------------------------------------------------------------------------------------------------------------------
Float32 CString::getFloat32() const
//----------------------------------------------------------------------------------------------------------------------
{
	return (Float32) ::CFStringGetDoubleValue(mStringRef);
}

//----------------------------------------------------------------------------------------------------------------------
Float64 CString::getFloat64() const
//----------------------------------------------------------------------------------------------------------------------
{
	return ::CFStringGetDoubleValue(mStringRef);
}

//----------------------------------------------------------------------------------------------------------------------
TBuffer<char> CString::getUTF8Chars() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFIndex	length = ::CFStringGetLength(mStringRef);

	// Check length
	if (length > 0) {
		// Setup
		TBuffer<char>	buffer((UInt32) length * 4);

		// Convert
		CFIndex	count =
						::CFStringGetBytes(mStringRef, CFRangeMake(0, length), kCFStringEncodingUTF8, 0, false,
								(UInt8*) *buffer, length * 4, nil);

		return TBuffer<char>(buffer, (UInt32) count);
	} else
		// Empty
		return TBuffer<char>(0);
}

//----------------------------------------------------------------------------------------------------------------------
TBuffer<UTF32Char> CString::getUTF32Chars() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Collect characters
	CFIndex				length = ::CFStringGetLength(mStringRef);
	TBuffer<UTF32Char>	buffer((UInt32) length);
	::CFStringGetBytes(mStringRef, CFRangeMake(0, length), sGetCFStringEncodingForCStringEncoding(kEncodingUTF32Native),
			0, false, (UInt8*) *buffer, length * sizeof(UTF32Char), nil);

	return buffer;
}

//----------------------------------------------------------------------------------------------------------------------
OV<CData> CString::getData(Encoding encoding) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Compose dataRef
	CFDataRef	dataRef =
						::CFStringCreateExternalRepresentation(kCFAllocatorDefault, mStringRef,
								sGetCFStringEncodingForCStringEncoding(encoding), '_');
	if (dataRef != nil) {
		// Create data
		CData	data(::CFDataGetBytePtr(dataRef), (CData::ByteCount) ::CFDataGetLength(dataRef));
		::CFRelease(dataRef);
		
		return OV<CData>(data);
	} else {
		// Failed
		LogError(sCreateFailedError, "getting data");

		return OV<CData>();
	}
}

//----------------------------------------------------------------------------------------------------------------------
CString CString::getSubString(CharIndex startIndex, OV<Length> length) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if need to limit length
	CFIndex	lengthUse;
	if (length.hasValue() && ((startIndex + *length) <= ::CFStringGetLength(mStringRef)))
		// Use requested
		lengthUse = *length;
	else
		// Limit to remainder of string
		lengthUse = ::CFStringGetLength(mStringRef) - startIndex;

	// Compose new string
	CFStringRef	stringRef =
						::CFStringCreateWithSubstring(kCFAllocatorDefault, mStringRef,
								::CFRangeMake(startIndex, lengthUse));
	CString	string(stringRef);
	::CFRelease(stringRef);
	
	return string;
}

//----------------------------------------------------------------------------------------------------------------------
CString CString::replacingSubStrings(const CString& subStringToReplace, const CString& replacementString) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Compose new string
	CFMutableStringRef	stringRef = ::CFStringCreateMutableCopy(kCFAllocatorDefault, 0, mStringRef);
	::CFStringFindAndReplace(stringRef, subStringToReplace.mStringRef, replacementString.mStringRef,
			::CFRangeMake(0, ::CFStringGetLength(mStringRef)), 0);
	CString	string(stringRef);
	::CFRelease(stringRef);

	return string;
}

//----------------------------------------------------------------------------------------------------------------------
CString CString::replacingCharacters(CharIndex startIndex, OV<Length> length, const CString& replacementString) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if need to limit length
	CFIndex	lengthUse;
	if (length.hasValue() && ((startIndex + *length) <= ::CFStringGetLength(mStringRef)))
		// Use requested
		lengthUse = *length;
	else
		// Limit to remainder of string
		lengthUse = ::CFStringGetLength(mStringRef) - startIndex;

	// Compose new string
	CFMutableStringRef	stringRef = ::CFStringCreateMutableCopy(kCFAllocatorDefault, 0, mStringRef);
	::CFStringReplace(stringRef, ::CFRangeMake(startIndex, lengthUse), replacementString.mStringRef);
	CString	string(stringRef);
	::CFRelease(stringRef);

	return string;
}

//----------------------------------------------------------------------------------------------------------------------
OV<SRange32> CString::findSubString(const CString& subString, CharIndex startIndex, const OV<Length>& length) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get length to check
	CFIndex	lengthUse =
					(length.hasValue() && ((startIndex + *length) <= ::CFStringGetLength(mStringRef))) ?
							*length : ::CFStringGetLength(mStringRef) - startIndex;

	// Find
	CFRange	range = {0, 0};
	::CFStringFindWithOptions(mStringRef, subString.mStringRef, CFRangeMake(startIndex, lengthUse), 0, &range);

	return (range.length > 0) ? OV<SRange32>(SRange32((UInt32) range.location, (UInt32) range.length)) : OV<SRange32>();
}

//----------------------------------------------------------------------------------------------------------------------
CString CString::lowercased() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Compose new string
	CFMutableStringRef	stringRef = ::CFStringCreateMutableCopy(kCFAllocatorDefault, 0, mStringRef);
	::CFStringLowercase(stringRef, nil);
	CString	string(stringRef);
	::CFRelease(stringRef);

	return string;
}

//----------------------------------------------------------------------------------------------------------------------
CString CString::uppercased() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Compose new string
	CFMutableStringRef	stringRef = ::CFStringCreateMutableCopy(kCFAllocatorDefault, 0, mStringRef);
	::CFStringUppercase(stringRef, nil);
	CString	string(stringRef);
	::CFRelease(stringRef);

	return string;
}

//----------------------------------------------------------------------------------------------------------------------
CString CString::removingLeadingAndTrailingWhitespace() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFMutableStringRef	stringRef = ::CFStringCreateMutableCopy(kCFAllocatorDefault, 0, mStringRef);

	// Remove any leading spaces
	while (::CFStringHasPrefix(stringRef, CFSTR(" ")) ||
			::CFStringHasPrefix(stringRef, CFSTR("\t")) ||
			::CFStringHasPrefix(stringRef, CFSTR("\n")) ||
			::CFStringHasPrefix(stringRef, CFSTR("\r")))
		::CFStringDelete(stringRef, CFRangeMake(0, 1));

	// Remove any trailing spaces
	while (::CFStringHasSuffix(stringRef, CFSTR(" ")) ||
			::CFStringHasSuffix(stringRef, CFSTR("\t")) ||
			::CFStringHasSuffix(stringRef, CFSTR("\n")) ||
			::CFStringHasSuffix(stringRef, CFSTR("\r")))
		::CFStringDelete(stringRef, CFRangeMake(::CFStringGetLength(stringRef) - 1, 1));

	CString	string(stringRef);
	::CFRelease(stringRef);

	return string;
}

//----------------------------------------------------------------------------------------------------------------------
CString CString::removingLeadingAndTrailingQuotes() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFMutableStringRef	stringRef = ::CFStringCreateMutableCopy(kCFAllocatorDefault, 0, mStringRef);

	// Remove any leading quotes
	if (::CFStringHasPrefix(stringRef, CFSTR("\"")))
		::CFStringDelete(stringRef, CFRangeMake(0, 1));

	// Remove any trailing quotes
	if (::CFStringHasSuffix(stringRef, CFSTR("\"")))
		::CFStringDelete(stringRef, CFRangeMake(getLength() - 1, 1));

	CString	string(stringRef);
	::CFRelease(stringRef);

	return string;
}

//----------------------------------------------------------------------------------------------------------------------
CString CString::getCommonPrefix(const CString& other) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get characters
	CFIndex	length1 = ::CFStringGetLength(mStringRef);
	UniChar	characters1[length1];
	::CFStringGetCharacters(mStringRef, CFRangeMake(0, length1), characters1);

	CFIndex	length2 = ::CFStringGetLength(other.mStringRef);
	UniChar	characters2[length2];
	::CFStringGetCharacters(other.mStringRef, CFRangeMake(0, length2), characters2);

	// See if need to limit length
	if (length2 > length1)
		// Limit length
		length1 = length2;

	// Create stringRef and copy characters that match
	CFMutableStringRef	stringRef = ::CFStringCreateMutable(kCFAllocatorDefault, 0);
	for (CFIndex i = 0; i < length1; i++) {
		if (characters1[i] == characters2[i])
			::CFStringAppendCharacters(stringRef, &characters1[i], 1);
		else
			break;
	}

	// Convert to string
	CString	string(stringRef);
	::CFRelease(stringRef);

	return string;
}

//----------------------------------------------------------------------------------------------------------------------
TArray<CString> CString::components(const CString& separator, bool includeEmptyComponents) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNArray<CString>	array;

	// Get array
	CFArrayRef	arrayRef =
						::CFStringCreateArrayBySeparatingStrings(kCFAllocatorDefault, mStringRef, separator.mStringRef);

	for (CFIndex i = 0; i < ::CFArrayGetCount(arrayRef); i++) {
		// Get string
		CFStringRef	stringRef = (CFStringRef) ::CFArrayGetValueAtIndex(arrayRef, i);

		// Check if empty
		if (includeEmptyComponents || (::CFStringGetLength(stringRef) > 0))
			// Add
			array += CString(stringRef);
	}
	::CFRelease(arrayRef);
	
	return array;
}

// MARK: Comparison methods

//----------------------------------------------------------------------------------------------------------------------
bool CString::compareTo(const CString& other, CompareToOptions compareToOptions) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFStringCompareFlags	compareFlags = 0;
	if (compareToOptions & CString::kCompareToOptionsCaseInsensitive)
		compareFlags |= kCFCompareCaseInsensitive;
	if (compareToOptions & CString::kCompareToOptionsNonliteral)
		compareFlags |= kCFCompareNonliteral;
	if (compareToOptions & CString::kCompareToOptionsNumerically)
		compareFlags |= kCFCompareNumerically;
	
	return ::CFStringCompare(mStringRef, other.mStringRef, compareFlags) != kCFCompareGreaterThan;
}

//----------------------------------------------------------------------------------------------------------------------
bool CString::hasPrefix(const CString& other) const
//----------------------------------------------------------------------------------------------------------------------
{
	return ::CFStringHasPrefix(mStringRef, other.mStringRef);
}

//----------------------------------------------------------------------------------------------------------------------
bool CString::hasSuffix(const CString& other) const
//----------------------------------------------------------------------------------------------------------------------
{
	return ::CFStringHasSuffix(mStringRef, other.mStringRef);
}

//----------------------------------------------------------------------------------------------------------------------
bool CString::contains(const CString& other, ContainsOptions containsOptions) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get range
	CFRange	range =
					::CFStringFind(mStringRef, other.mStringRef,
							sGetCFStringCompareFlagsForCStringContainsOptions(containsOptions));

	return range.location != kCFNotFound;
}

//----------------------------------------------------------------------------------------------------------------------
bool CString::equals(const CString& other, ContainsOptions containsOptions) const
//----------------------------------------------------------------------------------------------------------------------
{
	return ::CFStringCompare(mStringRef, other.mStringRef,
			sGetCFStringCompareFlagsForCStringContainsOptions(containsOptions)) == kCFCompareEqualTo;
}

//----------------------------------------------------------------------------------------------------------------------
bool CString::containsOnly(CharacterSet characterSet) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TBuffer<UTF32Char>	buffer = getUTF32Chars();

	// Check
	for (CharIndex i = 0; i < buffer.getCount(); i++) {
		// Check character
		if (!sIsCharacterInSet(buffer[i], characterSet))
			// Nope
			return false;
	}

	return true;
}

//----------------------------------------------------------------------------------------------------------------------
CString& CString::operator=(const CString& other)
//----------------------------------------------------------------------------------------------------------------------
{
	// Bye bye to us
	::CFRelease(mStringRef);

	// Hello to new
	mStringRef = (CFStringRef) ::CFRetain(other.mStringRef);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CString& CString::operator+=(const CString& other)
//----------------------------------------------------------------------------------------------------------------------
{
	// Append
	CFMutableStringRef	stringRef = ::CFStringCreateMutableCopy(kCFAllocatorDefault, 0, mStringRef);
	::CFStringAppend(stringRef, other.mStringRef);

	// Bye bye to us
	::CFRelease(mStringRef);

	// Hello to new
	mStringRef = stringRef;

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CString CString::operator+(const CString& other) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Compose new stringRef
	CFMutableStringRef	stringRef = ::CFStringCreateMutableCopy(kCFAllocatorDefault, 0, mStringRef);
	::CFStringAppend(stringRef, other.mStringRef);

	// Convert to string
	CString	string(stringRef);
	::CFRelease(stringRef);

	return string;
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CString CString::make(OSStringType format, va_list args)
//----------------------------------------------------------------------------------------------------------------------
{
	// Compose stringRef
	CFStringRef	stringRef = ::CFStringCreateWithFormatAndArguments(kCFAllocatorDefault, nil, format, args);

	// Convert to string
	CString	string(stringRef);
	::CFRelease(stringRef);

	return string;
}

// MARK: Internal methods

//----------------------------------------------------------------------------------------------------------------------
void CString::init()
//----------------------------------------------------------------------------------------------------------------------
{
	mStringRef = CFSTR("");
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions

//----------------------------------------------------------------------------------------------------------------------
CFStringCompareFlags sGetCFStringCompareFlagsForCStringContainsOptions(CString::ContainsOptions containsOptions)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFStringCompareFlags	compareFlags = 0;
	if (containsOptions & CString::kContainsOptionsCaseInsensitive)
		compareFlags |= kCFCompareCaseInsensitive;
	if (containsOptions & CString::kContainsOptionsNonliteral)
		compareFlags |= kCFCompareNonliteral;

	return compareFlags;
}

//----------------------------------------------------------------------------------------------------------------------
CFStringEncoding sGetCFStringEncodingForCStringEncoding(CString::Encoding encoding)
//----------------------------------------------------------------------------------------------------------------------
{
	switch (encoding) {
		case CString::kEncodingASCII:		return kCFStringEncodingASCII;
		case CString::kEncodingMacRoman:	return kCFStringEncodingMacRoman;
		case CString::kEncodingUTF8:		return kCFStringEncodingUTF8;
		case CString::kEncodingISOLatin:	return kCFStringEncodingISOLatin1;

		case CString::kEncodingUTF16:		return kCFStringEncodingUTF16;
		case CString::kEncodingUTF16BE:		return kCFStringEncodingUTF16BE;
		case CString::kEncodingUTF16LE:		return kCFStringEncodingUTF16LE;

		case CString::kEncodingUTF32BE:		return kCFStringEncodingUTF32BE;
		case CString::kEncodingUTF32LE:		return kCFStringEncodingUTF32LE;
	}
}

//----------------------------------------------------------------------------------------------------------------------
bool sIsCharacterInSet(UTF32Char utf32Char, CString::CharacterSet characterSet)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check character set
	switch (characterSet) {
		case CString::kCharacterSetControl:
			// Control
			return CFCharacterSetIsLongCharacterMember(
					::CFCharacterSetGetPredefined(kCFCharacterSetControl), utf32Char);

		case CString::kCharacterSetWhitespace:
			// Whitespace
			return CFCharacterSetIsLongCharacterMember(
					::CFCharacterSetGetPredefined(kCFCharacterSetWhitespace), utf32Char);

		case CString::kCharacterSetWhitespaceAndNewline:
			// Whitespace and newline
			return CFCharacterSetIsLongCharacterMember(
					::CFCharacterSetGetPredefined(kCFCharacterSetWhitespaceAndNewline), utf32Char);

		case CString::kCharacterSetDecimalDigit:
			// Decimal digit
			return CFCharacterSetIsLongCharacterMember(
					::CFCharacterSetGetPredefined(kCFCharacterSetDecimalDigit), utf32Char);

		case CString::kCharacterSetLetter:
			// Letter
			return CFCharacterSetIsLongCharacterMember(
					::CFCharacterSetGetPredefined(kCFCharacterSetLetter), utf32Char);

		case CString::kCharacterSetLowercaseLetter:
			// Lowercase letter
			return CFCharacterSetIsLongCharacterMember(
					::CFCharacterSetGetPredefined(kCFCharacterSetLowercaseLetter), utf32Char);

		case CString::kCharacterSetUppercaseLetter:
			// Uppercase letter
			return CFCharacterSetIsLongCharacterMember(
					::CFCharacterSetGetPredefined(kCFCharacterSetUppercaseLetter), utf32Char);

		case CString::kCharacterSetAlphaNumeric:
			// Alpha numeric
			return CFCharacterSetIsLongCharacterMember(
					::CFCharacterSetGetPredefined(kCFCharacterSetAlphaNumeric), utf32Char);

		case CString::kCharacterSetPunctuation:
			// Puncuation
			return CFCharacterSetIsLongCharacterMember(
					::CFCharacterSetGetPredefined(kCFCharacterSetPunctuation), utf32Char);
	}
}
