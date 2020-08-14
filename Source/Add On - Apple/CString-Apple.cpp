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
// MARK: Local proc declarations

static	CFOptionFlags		sGetCFOptionFlagsForCStringOptionFlags(UInt32 flags);
static	CFStringEncoding	sGetCFStringEncodingForCStringEncoding(EStringEncoding encoding);

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
CString::CString(const CString& other, OV<CStringLength> length) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	if (length.hasValue()) {
		// Create limited copy
		mStringRef = ::CFStringCreateMutableCopy(kCFAllocatorDefault, 0, other.mStringRef);
		if (::CFStringGetLength(mStringRef) > *length)
			// Truncate
			::CFStringReplace((CFMutableStringRef) mStringRef,
					CFRangeMake(*length, ::CFStringGetLength(mStringRef) - *length), CFSTR(""));
	} else
		// Make copy
		mStringRef = (CFStringRef) ::CFRetain(other.mStringRef);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(const OSStringVar(initialString), OV<CStringLength> length) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	mStringRef = (CFStringRef) ::CFRetain(initialString);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(const char* initialString, CStringLength bufferLen, EStringEncoding encoding) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertNotNil(initialString);

	// Setup
	if (initialString == nil)
		// No initial string
		mStringRef = CFSTR("");
	else if (bufferLen == kCStringDefaultMaxLength)
		// Use entire string
		mStringRef =
				::CFStringCreateWithCString(kCFAllocatorDefault, initialString,
						sGetCFStringEncodingForCStringEncoding(encoding));
	else {
		// Use only the length specified
		char	buffer[bufferLen + 1];
		::memmove(buffer, initialString, bufferLen);
		buffer[bufferLen] = 0;
		mStringRef =
				::CFStringCreateWithCString(kCFAllocatorDefault, buffer,
						sGetCFStringEncodingForCStringEncoding(encoding));
	}
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(const UTF16Char* initialString, CStringLength length, EStringEncoding encoding) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertNotNil(initialString);

	bool	encodingIsValid =
					(encoding == kStringEncodingUnicode) ||
					(encoding == kStringEncodingUTF16) ||
					(encoding == kStringEncodingUTF16BE) ||
					(encoding == kStringEncodingUTF16LE);
	AssertFailIf(!encodingIsValid);

	// Setup
	if ((initialString == nil) || !encodingIsValid)
		// Missing or invalid parameters
		mStringRef = CFSTR("");
	else
		// Create string
		mStringRef =
				::CFStringCreateWithBytes(kCFAllocatorDefault, (UInt8*) initialString,
						length * sizeof(UTF16Char), sGetCFStringEncodingForCStringEncoding(encoding), false);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(const UTF32Char* initialString, CStringLength length, EStringEncoding encoding) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertNotNil(initialString);
	AssertFailIf((encoding != kStringEncodingUTF32BE) && (encoding != kStringEncodingUTF32LE));

	// Setup
	if ((initialString == nil) || ((encoding != kStringEncodingUTF32BE) && (encoding != kStringEncodingUTF32LE)))
		// Missing or invalid parameters
		mStringRef = CFSTR("");
	else
		// Create string
		mStringRef =
				::CFStringCreateWithBytes(kCFAllocatorDefault, (UInt8*) initialString,
						length * sizeof(UTF32Char), sGetCFStringEncodingForCStringEncoding(encoding), false);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(Float32 value, UInt32 fieldSize, UInt32 digitsAfterDecimalPoint, bool padWithZeros) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check field size
	if (fieldSize == 0)
		mStringRef =
				::CFStringCreateWithFormat(kCFAllocatorDefault, nil, CFSTR("%*.*f"), (int) fieldSize,
						(int) digitsAfterDecimalPoint, (double) value);
	else
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
		mStringRef =
				::CFStringCreateWithFormat(kCFAllocatorDefault, nil, CFSTR("%*.*f"), (int) fieldSize,
						(int) digitsAfterDecimalPoint, value);
	else
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
		mStringRef = ::CFStringCreateWithFormat(kCFAllocatorDefault, nil, CFSTR("%hi"), (short) value);
	else
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
		mStringRef = ::CFStringCreateWithFormat(kCFAllocatorDefault, nil, CFSTR("%hi"), value);
	else
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
		mStringRef = ::CFStringCreateWithFormat(kCFAllocatorDefault, nil, CFSTR("%d"), (int) value);
	else
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
		mStringRef = ::CFStringCreateWithFormat(kCFAllocatorDefault, nil, CFSTR("%qi"), value);
	else
		mStringRef =
				::CFStringCreateWithFormat(kCFAllocatorDefault, nil, padWithZeros ? CFSTR("%.*qi") : CFSTR("%*qi"),
						(int) fieldSize, value);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(UInt8 value, UInt32 fieldSize, bool padWithZeros, bool makeHex) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check field size
	if (fieldSize == 0)
		mStringRef = ::CFStringCreateWithFormat(kCFAllocatorDefault, nil, CFSTR("%hu"), (unsigned short) value);
	else if (makeHex)
		mStringRef =
				::CFStringCreateWithFormat(kCFAllocatorDefault, nil, padWithZeros ? CFSTR("%#.*x") : CFSTR("%#*x"),
						(int) fieldSize, (unsigned int) value);
	else
		mStringRef =
				::CFStringCreateWithFormat(kCFAllocatorDefault, nil, padWithZeros ? CFSTR("%.*hu") : CFSTR("%*hu"),
						(int) fieldSize, (unsigned short) value);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(UInt16 value, UInt32 fieldSize, bool padWithZeros, bool makeHex) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check field size
	if (fieldSize == 0)
		mStringRef = ::CFStringCreateWithFormat(kCFAllocatorDefault, nil, CFSTR("%hu"), value);
	else if (makeHex)
		mStringRef =
				::CFStringCreateWithFormat(kCFAllocatorDefault, nil, padWithZeros ? CFSTR("%#.*x") : CFSTR("%#*x"),
						(int) fieldSize, (unsigned int) value);
	else
		mStringRef =
				::CFStringCreateWithFormat(kCFAllocatorDefault, nil, padWithZeros ? CFSTR("%.*hu") : CFSTR("%*hu"),
						(int) fieldSize, value);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(UInt32 value, UInt32 fieldSize, bool padWithZeros, bool makeHex) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check field size
	if (fieldSize == 0)
		mStringRef = ::CFStringCreateWithFormat(kCFAllocatorDefault, nil, CFSTR("%u"), (unsigned int) value);
	else if (makeHex)
		mStringRef =
				::CFStringCreateWithFormat(kCFAllocatorDefault, nil, padWithZeros ? CFSTR("%#.*x") : CFSTR("%#*x"),
						(int) fieldSize, (unsigned int) value);
	else
		mStringRef =
				::CFStringCreateWithFormat(kCFAllocatorDefault, nil, padWithZeros ? CFSTR("%.*u") : CFSTR("%*u"),
						(int) fieldSize, (unsigned int) value);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(UInt64 value, UInt32 fieldSize, bool padWithZeros, bool makeHex) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check field size
	if (fieldSize == 0)
		mStringRef = ::CFStringCreateWithFormat(kCFAllocatorDefault, nil, CFSTR("%qu"), value);
	else if (makeHex)
		mStringRef =
				::CFStringCreateWithFormat(kCFAllocatorDefault, nil, padWithZeros ? CFSTR("%#.*qx") : CFSTR("%#*qx"),
						(int) fieldSize, value);
	else
		mStringRef =
				::CFStringCreateWithFormat(kCFAllocatorDefault, nil, padWithZeros ? CFSTR("%.*qu") : CFSTR("%*qu"),
						(int) fieldSize, value);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(OSType osType, bool isOSType, bool includeQuotes) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	osType = EndianU32_NtoB(osType);
	mStringRef =
			::CFStringCreateWithFormat(kCFAllocatorDefault, nil, includeQuotes ? CFSTR("\'%4.4s\'") : CFSTR("%4.4s"),
					(char*) &osType);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(const void* pointer)
//----------------------------------------------------------------------------------------------------------------------
{
	mStringRef = ::CFStringCreateWithFormat(kCFAllocatorDefault, nil, CFSTR("%p"), pointer);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(const CData& data, EStringEncoding encoding)
//----------------------------------------------------------------------------------------------------------------------
{
	CFDataRef	dataRef = ::CFDataCreate(kCFAllocatorDefault, (const UInt8*) data.getBytePtr(), data.getSize());
	mStringRef =
			::CFStringCreateFromExternalRepresentation(kCFAllocatorDefault, dataRef,
					sGetCFStringEncodingForCStringEncoding(encoding));
	::CFRelease(dataRef);

	if (mStringRef == nil) {
		LogIfError(kParamError, "creating CFStringRef from external representation");
		mStringRef = CFSTR("");
	}
}

//----------------------------------------------------------------------------------------------------------------------
CString::~CString()
//----------------------------------------------------------------------------------------------------------------------
{
	::CFRelease(mStringRef);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
const OSStringType CString::getOSString() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mStringRef;
}

//----------------------------------------------------------------------------------------------------------------------
const SCString CString::getCString(EStringEncoding encoding) const
//----------------------------------------------------------------------------------------------------------------------
{
	CFStringEncoding	stringEncoding = sGetCFStringEncodingForCStringEncoding(encoding);
	CStringLength		length =
								(CStringLength) ::CFStringGetMaximumSizeForEncoding(
										::CFStringGetLength(mStringRef), stringEncoding) + 1;
	SCString	cString(length);
	::CFStringGetCString(mStringRef, cString.mBuffer, length, stringEncoding);

	return cString;
}

//----------------------------------------------------------------------------------------------------------------------
CStringLength CString::getLength() const
//----------------------------------------------------------------------------------------------------------------------
{
	return (CStringLength) ::CFStringGetLength(mStringRef);
}

//----------------------------------------------------------------------------------------------------------------------
CStringLength CString::getLength(EStringEncoding encoding, SInt8 lossCharacter, bool forExternalStorageOrTransmission)
		const
//----------------------------------------------------------------------------------------------------------------------
{
	CFIndex	byteCount;
	::CFStringGetBytes(mStringRef, CFRangeMake(0, ::CFStringGetLength(mStringRef)),
			sGetCFStringEncodingForCStringEncoding(encoding), lossCharacter, forExternalStorageOrTransmission, nil, 0,
			&byteCount);

	return (CStringLength) byteCount;
}

//----------------------------------------------------------------------------------------------------------------------
CStringLength CString::get(char* buffer, CStringLength bufferLen, bool addNull, EStringEncoding encoding) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertNotNil(buffer);
	if (buffer == nil)
		return 0;

	AssertFailIf(bufferLen == 0);
	if (bufferLen == 0)
		return 0;
	
	if ((buffer != nil) && (bufferLen > 0)) {
		// Save space for the null
		if (addNull)
			bufferLen--;

		// Calculate max characters to get
		CFIndex	length = ::CFStringGetLength(mStringRef);
		if (length > (CFIndex) bufferLen)
			// Each character takes at least one byte
			length = (CFIndex) bufferLen;
		
		// Take off characters at the end until we can fit in our buffer
		CFIndex	byteCount;
		do {
			::CFStringGetBytes(mStringRef, CFRangeMake(0, length), sGetCFStringEncodingForCStringEncoding(encoding), 0,
					false, nil, 0, &byteCount);
			if ((CStringLength) byteCount > bufferLen)
				length--;
		} while ((CStringLength) byteCount > bufferLen);

		// Get characters
		::CFStringGetBytes(mStringRef, CFRangeMake(0, length), sGetCFStringEncodingForCStringEncoding(encoding), 0,
				false, (UInt8*) buffer, bufferLen, &byteCount);

		// Add null if needed
		if (addNull)
			buffer[byteCount++] = 0;

		return (CStringLength) byteCount;
	} else
		return 0;
}

//----------------------------------------------------------------------------------------------------------------------
CStringLength CString::get(UTF16Char* buffer, CStringLength bufferLen, EStringEncoding encoding) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertNotNil(buffer);
	if (buffer == nil)
		return 0;

	AssertFailIf((encoding != kStringEncodingUTF16BE) && (encoding != kStringEncodingUTF16LE));
	if ((encoding != kStringEncodingUTF16BE) && (encoding != kStringEncodingUTF16LE))
		return 0;

	if (bufferLen > getLength())
		bufferLen = getLength();
	
	return (CStringLength) ::CFStringGetBytes(mStringRef, CFRangeMake(0, bufferLen),
			sGetCFStringEncodingForCStringEncoding(encoding), 0, false, (UInt8*) buffer, bufferLen * sizeof(UTF16Char),
			nil);
}

//----------------------------------------------------------------------------------------------------------------------
UTF32Char CString::getCharacterAtIndex(CStringCharIndex index) const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailIf(index > getLength());
	if (index > getLength())
		return 0;
	
	UTF32Char	_char;
	::CFStringGetBytes(mStringRef, CFRangeMake(index, 1),
			sGetCFStringEncodingForCStringEncoding(kStringEncodingUTF32Native), 0, false, (UInt8*) &_char,
			sizeof(UTF32Char), nil);

	return _char;
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
CData CString::getData(EStringEncoding encoding, SInt8 lossCharacter) const
//----------------------------------------------------------------------------------------------------------------------
{
	CFDataRef	dataRef =
						::CFStringCreateExternalRepresentation(kCFAllocatorDefault, mStringRef,
								sGetCFStringEncodingForCStringEncoding(encoding), lossCharacter);
	if (dataRef != nil) {
		// Create data
		CData	data(::CFDataGetBytePtr(dataRef), (CDataSize) ::CFDataGetLength(dataRef));
		::CFRelease(dataRef);
		
		return data;
	} else
		LogIfErrorAndReturnValue(kParamError, "creating external representation", CData::mEmpty);
}

//----------------------------------------------------------------------------------------------------------------------
CString CString::getSubString(CStringCharIndex startIndex, CStringLength charCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	if (((UInt64) startIndex + (UInt64) charCount) > (UInt64) ::CFStringGetLength(mStringRef))
		charCount = (CStringLength) (::CFStringGetLength(mStringRef) - startIndex);

	CFStringRef	stringRef =
						::CFStringCreateWithSubstring(kCFAllocatorDefault, mStringRef,
								CFRangeMake(startIndex, charCount));
	CString	string(stringRef);
	::CFRelease(stringRef);
	
	return string;
}

//----------------------------------------------------------------------------------------------------------------------
CString CString::replacingSubStrings(const CString& subStringToReplace, const CString& replacementString) const
//----------------------------------------------------------------------------------------------------------------------
{
	CFMutableStringRef	stringRef = ::CFStringCreateMutableCopy(kCFAllocatorDefault, 0, mStringRef);
	::CFStringFindAndReplace(stringRef, subStringToReplace.mStringRef, replacementString.mStringRef,
			::CFRangeMake(0, ::CFStringGetLength(mStringRef)), 0);
	CString	string(stringRef);
	::CFRelease(stringRef);

	return string;
}

//----------------------------------------------------------------------------------------------------------------------
CString CString::replacingCharacters(CStringCharIndex startIndex, CStringLength charCount,
		const CString& replacementString) const
//----------------------------------------------------------------------------------------------------------------------
{
	if (((UInt64) startIndex + (UInt64) charCount) > (UInt64) ::CFStringGetLength(mStringRef))
		charCount = (CStringLength) (::CFStringGetLength(mStringRef) - startIndex);

	CFMutableStringRef	stringRef = ::CFStringCreateMutableCopy(kCFAllocatorDefault, 0, mStringRef);
	::CFStringReplace(stringRef, ::CFRangeMake(startIndex, charCount), replacementString.mStringRef);
	CString	string(stringRef);
	::CFRelease(stringRef);

	return string;
}

//----------------------------------------------------------------------------------------------------------------------
SStringRange CString::findSubString(const CString& subString, CStringCharIndex startIndex, CStringLength charCount)
		const
//----------------------------------------------------------------------------------------------------------------------
{
	CFRange	range = {0, 0};
	::CFStringFindWithOptions(mStringRef, subString.mStringRef, CFRangeMake(startIndex, charCount), 0, &range);

	return SStringRange((CStringCharIndex) range.location, (CStringLength) range.length);
}

//----------------------------------------------------------------------------------------------------------------------
CString CString::lowercased() const
//----------------------------------------------------------------------------------------------------------------------
{
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
CString CString::removingAllWhitespace() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFMutableStringRef	stringRef = ::CFStringCreateMutableCopy(kCFAllocatorDefault, 0, mStringRef);

	::CFStringFindAndReplace(stringRef, CFSTR(" "), CFSTR(""), ::CFRangeMake(0, ::CFStringGetLength(stringRef)), 0);
	::CFStringFindAndReplace(stringRef, CFSTR("\t"), CFSTR(""), ::CFRangeMake(0, ::CFStringGetLength(stringRef)), 0);
	::CFStringFindAndReplace(stringRef, CFSTR("\n"), CFSTR(""), ::CFRangeMake(0, ::CFStringGetLength(stringRef)), 0);
	::CFStringFindAndReplace(stringRef, CFSTR("\r"), CFSTR(""), ::CFRangeMake(0, ::CFStringGetLength(stringRef)), 0);

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

	if (length2 > length1)
		length1 = length2;
	
	CFMutableStringRef	stringRef = ::CFStringCreateMutable(kCFAllocatorDefault, 0);
	
	for (CFIndex i = 0; i < length1; i++) {
		if (characters1[i] == characters2[i])
			::CFStringAppendCharacters(stringRef, &characters1[i], 1);
		else
			break;
	}

	CString	string(stringRef);
	::CFRelease(stringRef);

	return string;
}

//----------------------------------------------------------------------------------------------------------------------
TArray<CString> CString::breakUp(const CString& delimiterString) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNArray<CString>	array;

	// Get array
	CFArrayRef	arrayRef =
						::CFStringCreateArrayBySeparatingStrings(kCFAllocatorDefault, mStringRef,
								delimiterString.mStringRef);

	for (CFIndex i = 0; i < ::CFArrayGetCount(arrayRef); i++)
		array += CString((CFStringRef) ::CFArrayGetValueAtIndex(arrayRef, i));
	::CFRelease(arrayRef);
	
	return array;
}

// MARK: Comparison methods

//----------------------------------------------------------------------------------------------------------------------
ECompareResult CString::compareTo(const CString& other, EStringCompareFlags flags) const
//----------------------------------------------------------------------------------------------------------------------
{
	ECompareResult	compareResult =
							(ECompareResult) ::CFStringCompare(mStringRef, other.mStringRef,
									sGetCFOptionFlagsForCStringOptionFlags(flags));

	return compareResult;
}

//----------------------------------------------------------------------------------------------------------------------
bool CString::equals(const CString& other, EStringCompareFlags flags) const
//----------------------------------------------------------------------------------------------------------------------
{
	return ::CFStringCompare(mStringRef, other.mStringRef, sGetCFOptionFlagsForCStringOptionFlags(flags)) ==
			kCFCompareEqualTo;
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
bool CString::contains(const CString& other, EStringCompareFlags flags) const
//----------------------------------------------------------------------------------------------------------------------
{
	CFRange	range = ::CFStringFind(mStringRef, other.mStringRef, sGetCFOptionFlagsForCStringOptionFlags(flags));

	return range.location != kCFNotFound;
}

// MARK: Convenience operators

//----------------------------------------------------------------------------------------------------------------------
CString& CString::operator=(const CString& other)
//----------------------------------------------------------------------------------------------------------------------
{
	::CFRelease(mStringRef);
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

	::CFRelease(mStringRef);
	mStringRef = stringRef;

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CString CString::operator+(const CString& other) const
//----------------------------------------------------------------------------------------------------------------------
{
	CFMutableStringRef	stringRef = ::CFStringCreateMutableCopy(kCFAllocatorDefault, 0, mStringRef);
	::CFStringAppend(stringRef, other.mStringRef);

	CString	string(stringRef);
	::CFRelease(stringRef);

	return string;
}

// MARK: Utility methods

//----------------------------------------------------------------------------------------------------------------------
CString CString::make(const char* format, va_list args)
//----------------------------------------------------------------------------------------------------------------------
{
	CFStringRef	formatStringRef = ::CFStringCreateWithCString(kCFAllocatorDefault, format, kCFStringEncodingMacRoman);

	CFStringRef	stringRef = ::CFStringCreateWithFormat(kCFAllocatorDefault, nil, formatStringRef, args);
	::CFRelease(formatStringRef);

	CString		string(stringRef);
	::CFRelease(stringRef);

	return string;
}

//----------------------------------------------------------------------------------------------------------------------
bool CString::isCharacterInSet(UTF32Char utf32Char, EStringCharacterSet characterSet)
//----------------------------------------------------------------------------------------------------------------------
{
	switch (characterSet) {
		case kStringCharacterSetControl:
			return CFCharacterSetIsLongCharacterMember(
					::CFCharacterSetGetPredefined(kCFCharacterSetControl), utf32Char);

		case kStringCharacterSetWhitespace:
			return CFCharacterSetIsLongCharacterMember(
					::CFCharacterSetGetPredefined(kCFCharacterSetWhitespace), utf32Char);

		case kStringCharacterSetWhitespaceAndNewline:
			return CFCharacterSetIsLongCharacterMember(
					::CFCharacterSetGetPredefined(kCFCharacterSetWhitespaceAndNewline), utf32Char);

		case kStringCharacterSetDecimalDigit:
			return CFCharacterSetIsLongCharacterMember(
					::CFCharacterSetGetPredefined(kCFCharacterSetDecimalDigit), utf32Char);

		case kStringCharacterSetLetter:
			return CFCharacterSetIsLongCharacterMember(
					::CFCharacterSetGetPredefined(kCFCharacterSetLetter), utf32Char);

		case kStringCharacterSetLowercaseLetter:
			return CFCharacterSetIsLongCharacterMember(
					::CFCharacterSetGetPredefined(kCFCharacterSetLowercaseLetter), utf32Char);

		case kStringCharacterSetUppercaseLetter:
			return CFCharacterSetIsLongCharacterMember(
					::CFCharacterSetGetPredefined(kCFCharacterSetUppercaseLetter), utf32Char);

		case kStringCharacterSetNonBase:
			return CFCharacterSetIsLongCharacterMember(
					::CFCharacterSetGetPredefined(kCFCharacterSetNonBase), utf32Char);

		case kStringCharacterSetDecomposable:
			return CFCharacterSetIsLongCharacterMember(
					::CFCharacterSetGetPredefined(kCFCharacterSetDecomposable), utf32Char);

		case kStringCharacterSetAlphaNumeric:
			return CFCharacterSetIsLongCharacterMember(
					::CFCharacterSetGetPredefined(kCFCharacterSetAlphaNumeric), utf32Char);

		case kStringCharacterSetPunctuation:
			return CFCharacterSetIsLongCharacterMember(
					::CFCharacterSetGetPredefined(kCFCharacterSetPunctuation), utf32Char);

		case kStringCharacterSetIllegal:
			return CFCharacterSetIsLongCharacterMember(
					::CFCharacterSetGetPredefined(kCFCharacterSetIllegal), utf32Char);

		case kStringCharacterSetCapitalizedLetter:
			return CFCharacterSetIsLongCharacterMember(
					::CFCharacterSetGetPredefined(kCFCharacterSetCapitalizedLetter), utf32Char);

		case kStringCharacterSetSymbol:
			return CFCharacterSetIsLongCharacterMember(
					::CFCharacterSetGetPredefined(kCFCharacterSetSymbol), utf32Char);

		default:
			return false;
	}
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
CFOptionFlags sGetCFOptionFlagsForCStringOptionFlags(UInt32 flags)
//----------------------------------------------------------------------------------------------------------------------
{
	CFOptionFlags	outFlags = 0;
	
	if (flags & kStringCompareFlagsCaseInsensitive)
		outFlags |= kCFCompareCaseInsensitive;
	if (flags & kStringCompareFlagsBackwards)
		outFlags |= kCFCompareBackwards;
	if (flags & kStringCompareFlagsNonliteral)
		outFlags |= kCFCompareNonliteral;
	if (flags & kStringCompareFlagsLocalized)
		outFlags |= kCFCompareLocalized;
	if (flags & kStringCompareFlagsNumerically)
		outFlags |= kCFCompareNumerically;
	
	return outFlags;
}

//----------------------------------------------------------------------------------------------------------------------
CFStringEncoding sGetCFStringEncodingForCStringEncoding(EStringEncoding encoding)
//----------------------------------------------------------------------------------------------------------------------
{
	switch (encoding) {
		case kStringEncodingASCII:		return kCFStringEncodingASCII;
		case kStringEncodingMacRoman:	return kCFStringEncodingMacRoman;
		case kStringEncodingUTF8:		return kCFStringEncodingUTF8;
		case kStringEncodingISOLatin:	return kCFStringEncodingISOLatin1;
		case kStringEncodingUnicode:	return kCFStringEncodingUnicode;

		case kStringEncodingUTF16:		return kCFStringEncodingUTF16;
		case kStringEncodingUTF16BE:	return kCFStringEncodingUTF16BE;
		case kStringEncodingUTF16LE:	return kCFStringEncodingUTF16LE;
		case kStringEncodingUTF32:		return kCFStringEncodingUTF32;
		case kStringEncodingUTF32BE:	return kCFStringEncodingUTF32BE;
		case kStringEncodingUTF32LE:	return kCFStringEncodingUTF32LE;

		default:						return kCFStringEncodingUTF8;
	}
}
