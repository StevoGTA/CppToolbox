//----------------------------------------------------------------------------------------------------------------------
//	CStringCFImplementation.cpp			©2003 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CString.h"

#include "CppToolboxAssert.h"
#include "CData.h"
#include "CFUtilities.h"
#include "CLogServices.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CStringInternals

class CStringInternals {
	public:
		CStringInternals()
			{
				mBuffer = nil;
				mStringRef = nil;
			}
		~CStringInternals()
			{
				if (mBuffer != nil)
					DisposeOfArray(mBuffer);
				if (mStringRef != nil)
					::CFRelease(mStringRef);
			}
			
		char*				mBuffer;
		CFMutableStringRef	mStringRef;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc declarations

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
	mInternals = new CStringInternals();
	mInternals->mStringRef = ::CFStringCreateMutable(kCFAllocatorDefault, 0);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(const CString& other) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CStringInternals();
	mInternals->mStringRef = ::CFStringCreateMutableCopy(kCFAllocatorDefault, 0, other.mInternals->mStringRef);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(const OSString& initialString) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CStringInternals();
	mInternals->mStringRef = ::CFStringCreateMutableCopy(kCFAllocatorDefault, 0, initialString);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(const char* initialString, CStringLength bufferLen, EStringEncoding encoding) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertNotNil(initialString);
	if (initialString == nil) {
		// Punt
		init();

		return;
	}

	mInternals = new CStringInternals();
	mInternals->mStringRef = ::CFStringCreateMutable(kCFAllocatorDefault, 0);
	if (bufferLen == kCStringDefaultMaxLength)
		::CFStringAppendCString(mInternals->mStringRef, initialString,
				sGetCFStringEncodingForCStringEncoding(encoding));
	else if (initialString != nil) {
		char*	buffer = new char[bufferLen + 1];
		::memmove(buffer, initialString, bufferLen);
		buffer[bufferLen] = 0;
		::CFStringAppendCString(mInternals->mStringRef, buffer, sGetCFStringEncodingForCStringEncoding(encoding));
		delete [] buffer;
	}
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(const UTF16Char* initialString, CStringLength length, EStringEncoding encoding) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertNotNil(initialString);
	if (initialString == nil) {
		// Punt
		init();

		return;
	}

	bool	encodingIsValid =
					(encoding == kStringEncodingUnicode) ||
					(encoding == kStringEncodingUTF16) ||
					(encoding == kStringEncodingUTF16BE) ||
					(encoding == kStringEncodingUTF16LE);
	AssertFailIf(!encodingIsValid);
	if (!encodingIsValid)
		return;

	// Setup
	mInternals = new CStringInternals();

	CFStringRef	stringRef =
						::CFStringCreateWithBytes(kCFAllocatorDefault, (UInt8*) initialString,
								length * sizeof(UTF16Char), sGetCFStringEncodingForCStringEncoding(encoding), false);
	mInternals->mStringRef = ::CFStringCreateMutableCopy(kCFAllocatorDefault, 0, stringRef);
	::CFRelease(stringRef);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(const UTF32Char* initialString, CStringLength length, EStringEncoding encoding) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertNotNil(initialString);
	if (initialString == nil) {
		// Punt
		init();

		return;
	}

	AssertFailIf((encoding != kStringEncodingUTF32BE) && (encoding != kStringEncodingUTF32LE));
	if ((encoding != kStringEncodingUTF32BE) && (encoding != kStringEncodingUTF32LE))
		return;

	mInternals = new CStringInternals();

	CFStringRef	stringRef =
						::CFStringCreateWithBytes(kCFAllocatorDefault, (UInt8*) initialString,
								length * sizeof(UTF32Char), sGetCFStringEncodingForCStringEncoding(encoding), false);
	mInternals->mStringRef = ::CFStringCreateMutableCopy(kCFAllocatorDefault, 0, stringRef);
	::CFRelease(stringRef);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(Float32 value, UInt32 fieldSize, UInt32 digitsAfterDecimalPoint, bool padWithZeros) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CStringInternals();
	mInternals->mStringRef = ::CFStringCreateMutable(kCFAllocatorDefault, 0);

	if (fieldSize == 0)
		::CFStringAppendFormat(mInternals->mStringRef, nil, CFSTR("%*.*f"), (int) fieldSize,
				(int) digitsAfterDecimalPoint, (double) value);
	else
		::CFStringAppendFormat(mInternals->mStringRef, nil, padWithZeros ? CFSTR("%0*.*f") : CFSTR("%*.*f"),
				(int) fieldSize, (int) digitsAfterDecimalPoint, (double) value);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(Float64 value, UInt32 fieldSize, UInt32 digitsAfterDecimalPoint, bool padWithZeros) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CStringInternals();
	mInternals->mStringRef = ::CFStringCreateMutable(kCFAllocatorDefault, 0);

	if (fieldSize == 0)
		::CFStringAppendFormat(mInternals->mStringRef, nil, CFSTR("%*.*f"), (int) fieldSize,
				(int) digitsAfterDecimalPoint, value);
	else
		::CFStringAppendFormat(mInternals->mStringRef, nil, padWithZeros ? CFSTR("%0*.*f") : CFSTR("%*.*f"),
				(int) fieldSize, (int) digitsAfterDecimalPoint, value);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(SInt8 value, UInt32 fieldSize, bool padWithZeros) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CStringInternals();
	mInternals->mStringRef = ::CFStringCreateMutable(kCFAllocatorDefault, 0);
	
	if (fieldSize == 0)
		::CFStringAppendFormat(mInternals->mStringRef, nil, CFSTR("%hi"), (short) value);
	else
		::CFStringAppendFormat(mInternals->mStringRef, nil, padWithZeros ? CFSTR("%.*hi") : CFSTR("%*hi"),
				(int) fieldSize, (short) value);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(SInt16 value, UInt32 fieldSize, bool padWithZeros) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CStringInternals();
	mInternals->mStringRef = ::CFStringCreateMutable(kCFAllocatorDefault, 0);
	
	if (fieldSize == 0)
		::CFStringAppendFormat(mInternals->mStringRef, nil, CFSTR("%hi"), value);
	else
		::CFStringAppendFormat(mInternals->mStringRef, nil, padWithZeros ? CFSTR("%.*hi") : CFSTR("%*hi"),
				(int) fieldSize, value);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(SInt32 value, UInt32 fieldSize, bool padWithZeros) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CStringInternals();
	mInternals->mStringRef = ::CFStringCreateMutable(kCFAllocatorDefault, 0);
	
	if (fieldSize == 0)
		::CFStringAppendFormat(mInternals->mStringRef, nil, CFSTR("%d"), (int) value);
	else
		::CFStringAppendFormat(mInternals->mStringRef, nil, padWithZeros ? CFSTR("%.*d") : CFSTR("%*d"),
				(int) fieldSize, (int) value);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(SInt64 value, UInt32 fieldSize, bool padWithZeros) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CStringInternals();
	mInternals->mStringRef = ::CFStringCreateMutable(kCFAllocatorDefault, 0);
	
	if (fieldSize == 0)
		::CFStringAppendFormat(mInternals->mStringRef, nil, CFSTR("%qi"), value);
	else
		::CFStringAppendFormat(mInternals->mStringRef, nil, padWithZeros ? CFSTR("%.*qi") : CFSTR("%*qi"),
				(int) fieldSize, value);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(UInt8 value, UInt32 fieldSize, bool padWithZeros, bool makeHex) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CStringInternals();
	mInternals->mStringRef = ::CFStringCreateMutable(kCFAllocatorDefault, 0);
	
	if (fieldSize == 0)
		::CFStringAppendFormat(mInternals->mStringRef, nil, CFSTR("%hu"), (unsigned short) value);
	else if (makeHex)
		::CFStringAppendFormat(mInternals->mStringRef, nil, padWithZeros ? CFSTR("%#.*x") : CFSTR("%#*x"),
				(int) fieldSize, (unsigned int) value);
	else
		::CFStringAppendFormat(mInternals->mStringRef, nil, padWithZeros ? CFSTR("%.*hu") : CFSTR("%*hu"),
				(int) fieldSize, (unsigned short) value);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(UInt16 value, UInt32 fieldSize, bool padWithZeros, bool makeHex) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CStringInternals();
	mInternals->mStringRef = ::CFStringCreateMutable(kCFAllocatorDefault, 0);
	
	if (fieldSize == 0)
		::CFStringAppendFormat(mInternals->mStringRef, nil, CFSTR("%hu"), value);
	else if (makeHex)
		::CFStringAppendFormat(mInternals->mStringRef, nil, padWithZeros ? CFSTR("%#.*x") : CFSTR("%#*x"),
				(int) fieldSize, (unsigned int) value);
	else
		::CFStringAppendFormat(mInternals->mStringRef, nil, padWithZeros ? CFSTR("%.*hu") : CFSTR("%*hu"),
				(int) fieldSize, value);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(UInt32 value, UInt32 fieldSize, bool padWithZeros, bool makeHex) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CStringInternals();
	mInternals->mStringRef = ::CFStringCreateMutable(kCFAllocatorDefault, 0);
	
	if (fieldSize == 0)
		::CFStringAppendFormat(mInternals->mStringRef, nil, CFSTR("%u"), (unsigned int) value);
	else if (makeHex)
		::CFStringAppendFormat(mInternals->mStringRef, nil, padWithZeros ? CFSTR("%#.*x") : CFSTR("%#*x"),
				(int) fieldSize, (unsigned int) value);
	else
		::CFStringAppendFormat(mInternals->mStringRef, nil, padWithZeros ? CFSTR("%.*u") : CFSTR("%*u"),
				(int) fieldSize, (unsigned int) value);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(UInt64 value, UInt32 fieldSize, bool padWithZeros, bool makeHex) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CStringInternals();
	mInternals->mStringRef = ::CFStringCreateMutable(kCFAllocatorDefault, 0);
	
	if (fieldSize == 0)
		::CFStringAppendFormat(mInternals->mStringRef, nil, CFSTR("%qu"), value);
	else if (makeHex)
		::CFStringAppendFormat(mInternals->mStringRef, nil, padWithZeros ? CFSTR("%#.*qx") : CFSTR("%#*qx"),
				(int) fieldSize, value);
	else
		::CFStringAppendFormat(mInternals->mStringRef, nil, padWithZeros ? CFSTR("%.*qu") : CFSTR("%*qu"),
				(int) fieldSize, value);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(OSType osType, bool isOSType, bool includeQuotes) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CStringInternals();
	mInternals->mStringRef = ::CFStringCreateMutable(kCFAllocatorDefault, 0);
	
	osType = EndianU32_NtoB(osType);
	::CFStringAppendFormat(mInternals->mStringRef, nil, includeQuotes ? CFSTR("\'%4.4s\'") : CFSTR("%4.4s"),
			(char*) &osType);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(const void* pointer)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CStringInternals();
	mInternals->mStringRef = ::CFStringCreateMutable(kCFAllocatorDefault, 0);

	::CFStringAppendFormat(mInternals->mStringRef, nil, CFSTR("%p"), pointer);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(const CData& data, EStringEncoding encoding)
//----------------------------------------------------------------------------------------------------------------------
{
	CFDataRef	dataRef = ::CFDataCreate(kCFAllocatorDefault, (const UInt8*) data.getBytePtr(), data.getSize());
	CFStringRef	stringRef =
						::CFStringCreateFromExternalRepresentation(kCFAllocatorDefault, dataRef,
								sGetCFStringEncodingForCStringEncoding(encoding));
	::CFRelease(dataRef);
	if (stringRef != nil) {
		mInternals = new CStringInternals();
		mInternals->mStringRef = ::CFStringCreateMutableCopy(kCFAllocatorDefault, 0, stringRef);
		::CFRelease(stringRef);
	} else {
		LogIfError(kParamError, "creating CFStringRef from external representation");

		init();
	}
}

//----------------------------------------------------------------------------------------------------------------------
CString::~CString()
//----------------------------------------------------------------------------------------------------------------------
{
	DisposeOf(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
const char* CString::getCString(EStringEncoding encoding) const
//----------------------------------------------------------------------------------------------------------------------
{
	DisposeOfArray(mInternals->mBuffer);

	CFIndex	len =
					::CFStringGetMaximumSizeForEncoding(::CFStringGetLength(mInternals->mStringRef),
							sGetCFStringEncodingForCStringEncoding(encoding)) + 1;
	mInternals->mBuffer = new char[len];
	::CFStringGetCString(mInternals->mStringRef, mInternals->mBuffer, len,
			sGetCFStringEncodingForCStringEncoding(encoding));
 	
	return mInternals->mBuffer;
}

//----------------------------------------------------------------------------------------------------------------------
CStringLength CString::getLength(EStringEncoding encoding) const
//----------------------------------------------------------------------------------------------------------------------
{
	if (encoding == kStringEncodingCurrent)
		return (CStringLength) ::CFStringGetLength(mInternals->mStringRef);
	else {
		CFIndex	byteCount;
		::CFStringGetBytes(mInternals->mStringRef, CFRangeMake(0, ::CFStringGetLength(mInternals->mStringRef)),
				sGetCFStringEncodingForCStringEncoding(encoding), 0, false, nil, 0, &byteCount);
		
		return (CStringLength) byteCount;
	}
}

//----------------------------------------------------------------------------------------------------------------------
CString& CString::setLength(CStringLength length)
//----------------------------------------------------------------------------------------------------------------------
{
	if (getLength() > length)
		replaceCharacters(length, kCStringDefaultMaxLength);

	return *this;
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

		// Adjust encoding
		if (encoding == kStringEncodingCurrent)
			encoding = kStringEncodingTextDefault;
		
		// Calculate max characters to get
		CFIndex	length = ::CFStringGetLength(mInternals->mStringRef);
		if (length > (CFIndex) bufferLen)
			// Each character takes at least one byte
			length = (CFIndex) bufferLen;
		
		// Take off characters at the end until we can fit in our buffer
		CFIndex	byteCount;
		do {
			::CFStringGetBytes(mInternals->mStringRef, CFRangeMake(0, length),
					sGetCFStringEncodingForCStringEncoding(encoding), 0, false, nil, 0, &byteCount);
			if ((CStringLength) byteCount > bufferLen)
				length--;
		} while ((CStringLength) byteCount > bufferLen);

		// Get characters
		::CFStringGetBytes(mInternals->mStringRef, CFRangeMake(0, length),
				sGetCFStringEncodingForCStringEncoding(encoding), 0, false, (UInt8*) buffer, bufferLen, &byteCount);

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
	
	return (CStringLength) ::CFStringGetBytes(mInternals->mStringRef, CFRangeMake(0, bufferLen),
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
	::CFStringGetBytes(mInternals->mStringRef, CFRangeMake(index, 1),
			sGetCFStringEncodingForCStringEncoding(kStringEncodingUTF32Native), 0, false, (UInt8*) &_char,
			sizeof(UTF32Char), nil);
	
	return _char;
}

//----------------------------------------------------------------------------------------------------------------------
Float32 CString::getFloat32() const
//----------------------------------------------------------------------------------------------------------------------
{
	return (Float32) ::CFStringGetDoubleValue(mInternals->mStringRef);
}

//----------------------------------------------------------------------------------------------------------------------
Float64 CString::getFloat64() const
//----------------------------------------------------------------------------------------------------------------------
{
	return ::CFStringGetDoubleValue(mInternals->mStringRef);
}

//----------------------------------------------------------------------------------------------------------------------
SInt8 CString::getSInt8(UInt8 base) const
//----------------------------------------------------------------------------------------------------------------------
{
	return (SInt8) ::CFStringGetIntValue(mInternals->mStringRef);
}

//----------------------------------------------------------------------------------------------------------------------
SInt16 CString::getSInt16(UInt8 base) const
//----------------------------------------------------------------------------------------------------------------------
{
	return (SInt16) ::CFStringGetIntValue(mInternals->mStringRef);
}

//----------------------------------------------------------------------------------------------------------------------
SInt32 CString::getSInt32(UInt8 base) const
//----------------------------------------------------------------------------------------------------------------------
{
	return ::CFStringGetIntValue(mInternals->mStringRef);
}

//----------------------------------------------------------------------------------------------------------------------
SInt64 CString::getSInt64(UInt8 base) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Convert to char buffer
	CFIndex	len =
					::CFStringGetMaximumSizeForEncoding(::CFStringGetLength(mInternals->mStringRef),
							kCFStringEncodingMacRoman) + 1;

	char*	buffer = new char[len];
	::CFStringGetCString(mInternals->mStringRef, buffer, len, kCFStringEncodingMacRoman);
	buffer[len - 1] = 0;

	SInt64	sInt64 = ::strtoll(buffer, nil, base);

	delete [] buffer;

	return sInt64;
}

//----------------------------------------------------------------------------------------------------------------------
UInt8 CString::getUInt8(UInt8 base) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Convert to char* buffer
	CFIndex	len =
					::CFStringGetMaximumSizeForEncoding(::CFStringGetLength(mInternals->mStringRef),
							kCFStringEncodingMacRoman) + 1;
	
	char*	buffer = new char[len];
	::CFStringGetCString(mInternals->mStringRef, buffer, len, kCFStringEncodingMacRoman);
	buffer[len - 1] = 0;

	UInt8	uInt8 = (UInt8) ::strtoul(buffer, nil, base);

	delete[] buffer;

	return uInt8;
}

//----------------------------------------------------------------------------------------------------------------------
UInt16 CString::getUInt16(UInt8 base) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Convert to char* buffer
	CFIndex	len =
					::CFStringGetMaximumSizeForEncoding(::CFStringGetLength(mInternals->mStringRef),
							kCFStringEncodingMacRoman) + 1;
	
	char*	buffer = new char[len];
	::CFStringGetCString(mInternals->mStringRef, buffer, len, kCFStringEncodingMacRoman);
	buffer[len - 1] = 0;

	UInt16	uInt16 = (UInt16) ::strtoul(buffer, nil, base);

	delete[] buffer;

	return uInt16;
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CString::getUInt32(UInt8 base) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Convert to char* buffer
	CFIndex	len =
					::CFStringGetMaximumSizeForEncoding(::CFStringGetLength(mInternals->mStringRef),
							kCFStringEncodingMacRoman) + 1;
	
	char*	buffer = new char[len];
	::CFStringGetCString(mInternals->mStringRef, buffer, len, kCFStringEncodingMacRoman);
	buffer[len - 1] = 0;

	UInt32	uInt32 = (UInt32) ::strtoul(buffer, nil, base);

	delete[] buffer;

	return uInt32;
}

//----------------------------------------------------------------------------------------------------------------------
OSType CString::getOSType() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Convert to char* buffer
	CFIndex	len =
					::CFStringGetMaximumSizeForEncoding(::CFStringGetLength(mInternals->mStringRef),
							kCFStringEncodingMacRoman) + 1;
	
	char*	buffer = new char[len];
	::CFStringGetCString(mInternals->mStringRef, buffer, len, kCFStringEncodingMacRoman);
	buffer[len - 1] = 0;

	OSType	osType = EndianU32_BtoN(*((UInt32*)buffer));

	delete[] buffer;

	return osType;
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CString::getUInt64(UInt8 base) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Convert to char* buffer
	CFIndex	len =
					::CFStringGetMaximumSizeForEncoding(::CFStringGetLength(mInternals->mStringRef),
							kCFStringEncodingMacRoman) + 1;
	
	char*	buffer = new char[len];
	::CFStringGetCString(mInternals->mStringRef, buffer, len, kCFStringEncodingMacRoman);
	buffer[len - 1] = 0;

	UInt64	uInt64 = ::strtoull(buffer, nil, base);

	delete[] buffer;

	return uInt64;
}

//----------------------------------------------------------------------------------------------------------------------
CData CString::getData(EStringEncoding encoding) const
//----------------------------------------------------------------------------------------------------------------------
{
	CFDataRef	dataRef =
						::CFStringCreateExternalRepresentation(kCFAllocatorDefault, mInternals->mStringRef,
								sGetCFStringEncodingForCStringEncoding(encoding), 0);
	if (dataRef != nil) {
		// Create data
		CData	data(::CFDataGetBytePtr(dataRef), (CDataSize) ::CFDataGetLength(dataRef));
		::CFRelease(dataRef);
		
		return data;
	} else
		LogIfErrorAndReturnValue(kParamError, "creating external representation", CData());
}

//----------------------------------------------------------------------------------------------------------------------
CString CString::getSubString(CStringCharIndex startIndex, CStringLength charCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	if (((UInt64) startIndex + (UInt64) charCount) > (UInt64) ::CFStringGetLength(mInternals->mStringRef))
		charCount = (CStringLength) (::CFStringGetLength(mInternals->mStringRef) - startIndex);
		
	CFStringRef	stringRef =
						::CFStringCreateWithSubstring(kCFAllocatorDefault, mInternals->mStringRef,
								CFRangeMake(startIndex, charCount));
	CString	string(stringRef);
	::CFRelease(stringRef);
	
	return string;
}

//----------------------------------------------------------------------------------------------------------------------
CString& CString::replaceSubStrings(const CString& subStringToReplace, const CString& replacementString)
//----------------------------------------------------------------------------------------------------------------------
{
	::CFStringFindAndReplace(mInternals->mStringRef, subStringToReplace.mInternals->mStringRef,
			replacementString.mInternals->mStringRef, ::CFRangeMake(0, ::CFStringGetLength(mInternals->mStringRef)), 0);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CString& CString::replaceCharacters(CStringCharIndex startIndex, CStringLength charCount,
		const CString& replacementString)
//----------------------------------------------------------------------------------------------------------------------
{
	if (((UInt64) startIndex + (UInt64) charCount) > (UInt64) ::CFStringGetLength(mInternals->mStringRef))
		charCount = (CStringLength) (::CFStringGetLength(mInternals->mStringRef) - startIndex);

	::CFStringReplace(mInternals->mStringRef, ::CFRangeMake(startIndex, charCount),
			replacementString.mInternals->mStringRef);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
SStringRange CString::findSubString(const CString& subString, CStringCharIndex startIndex, CStringLength charCount)
		const
//----------------------------------------------------------------------------------------------------------------------
{
	CFRange	range = {0, 0};
	::CFStringFindWithOptions(mInternals->mStringRef, subString.mInternals->mStringRef,
			CFRangeMake(startIndex, charCount), 0, &range);
	
	return {(CStringCharIndex) range.location, (CStringLength) range.length};
}

//----------------------------------------------------------------------------------------------------------------------
CString& CString::makeLowercase()
//----------------------------------------------------------------------------------------------------------------------
{
	::CFStringLowercase(mInternals->mStringRef, nil);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CString& CString::makeUppercase()
//----------------------------------------------------------------------------------------------------------------------
{
	::CFStringUppercase(mInternals->mStringRef, nil);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CString& CString::removeLeadingAndTrailingWhitespace()
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove any leading spaces
	while (::CFStringHasPrefix(mInternals->mStringRef, CFSTR(" ")) ||
			::CFStringHasPrefix(mInternals->mStringRef, CFSTR("\t")) ||
			::CFStringHasPrefix(mInternals->mStringRef, CFSTR("\n")) ||
			::CFStringHasPrefix(mInternals->mStringRef, CFSTR("\r")))
		::CFStringDelete(mInternals->mStringRef, CFRangeMake(0, 1));
	
	// Remove any trailing spaces
	while (::CFStringHasSuffix(mInternals->mStringRef, CFSTR(" ")) ||
			::CFStringHasSuffix(mInternals->mStringRef, CFSTR("\t")) ||
			::CFStringHasSuffix(mInternals->mStringRef, CFSTR("\n")) ||
			::CFStringHasSuffix(mInternals->mStringRef, CFSTR("\r")))
		::CFStringDelete(mInternals->mStringRef, CFRangeMake(getLength() - 1, 1));

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CString& CString::removeAllWhitespace()
//----------------------------------------------------------------------------------------------------------------------
{
	::CFStringFindAndReplace(mInternals->mStringRef, CFSTR(" "), CFSTR(""),
			::CFRangeMake(0, ::CFStringGetLength(mInternals->mStringRef)), 0);
	::CFStringFindAndReplace(mInternals->mStringRef, CFSTR("\t"), CFSTR(""),
			::CFRangeMake(0, ::CFStringGetLength(mInternals->mStringRef)), 0);
	::CFStringFindAndReplace(mInternals->mStringRef, CFSTR("\n"), CFSTR(""),
			::CFRangeMake(0, ::CFStringGetLength(mInternals->mStringRef)), 0);
	::CFStringFindAndReplace(mInternals->mStringRef, CFSTR("\r"), CFSTR(""),
			::CFRangeMake(0, ::CFStringGetLength(mInternals->mStringRef)), 0);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CString& CString::removeLeadingAndTrailingQuotes()
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove any leading quotes
	if (::CFStringHasPrefix(mInternals->mStringRef, CFSTR("\"")))
		::CFStringDelete(mInternals->mStringRef, CFRangeMake(0, 1));
	
	// Remove any trailing quotes
	if (::CFStringHasSuffix(mInternals->mStringRef, CFSTR("\"")))
		::CFStringDelete(mInternals->mStringRef, CFRangeMake(getLength() - 1, 1));

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CString& CString::makeLegalFilename(UInt32 options)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get length of string
	CStringLength	length = (CStringLength) ::CFStringGetLength(mInternals->mStringRef);
	
	// Extract characters
	UniChar*	buffer = new UniChar[length];
	::CFStringGetCharacters(mInternals->mStringRef, ::CFRangeMake(0, length), buffer);
	
	// Replace "illegal" ones with '_'
	UniChar*	p = buffer;
	for (CStringLength i = 0; i < length; i++, p++) {
		if ((*p < 0x20) || (*p == ':') || (*p == 0x7F))
			*p = '_';
		
		if ((options & kStringMakeLegalFilenameOptionsDisallowSpaces) && (*p == ' '))
			*p = '_';
	}
	
	::CFStringDelete(mInternals->mStringRef, ::CFRangeMake(0, length));
	::CFStringAppendCharacters(mInternals->mStringRef, buffer, length);
	DisposeOfArray(buffer);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CString CString::getCommonBeginning(const CString& other) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Get characters
	CFStringRef	stringRef = mInternals->mStringRef;
	CFIndex		length1 = ::CFStringGetLength(stringRef);
	UniChar*	characters1 = new UniChar[length1];
	::CFStringGetCharacters(stringRef, CFRangeMake(0, length1), characters1);
	
	stringRef = other.mInternals->mStringRef;
	CFIndex		length2 = ::CFStringGetLength(stringRef);
	UniChar*	characters2 = new UniChar[length2];
	::CFStringGetCharacters(stringRef, CFRangeMake(0, length2), characters2);
	
	if (length2 > length1)
		length1 = length2;
	
	CString				outString;
	CFMutableStringRef	outStringRef = outString.mInternals->mStringRef;
	
	for (CFIndex i = 0; i < length1; i++) {
		if (characters1[i] == characters2[i])
			::CFStringAppendCharacters(outStringRef, &characters1[i], 1);
		else
			break;
	}

	delete [] characters1;
	delete [] characters2;

	return outString;
}

//----------------------------------------------------------------------------------------------------------------------
TArray<CString> CString::breakUp(const CString& delimiterString, bool respectQuotes) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNArray<CString>	array;

	if (respectQuotes) {
		// Quotes around a string with the delimiter is not a real delimiter
		bool	inQuotes = false;
		CString	tempString;
		for (CStringCharIndex i = 0; i < getLength();) {
			if (getCharacterAtIndex(i) == '\"') {
				// Found quote
				inQuotes = !inQuotes;
				i++;
			} else if (!inQuotes && (getSubString(i).beginsWith(delimiterString))) {
				// Found delimiter
				array += tempString;
				tempString = mEmpty;
				i += delimiterString.getLength();
			} else {
				// Found another character
				tempString += getSubString(i, 1);
				i++;
			}
		}
		
		if (!tempString.isEmpty())
			array += tempString;
	} else {
		// Just do break up
		CFArrayRef	arrayRef =
							::CFStringCreateArrayBySeparatingStrings(kCFAllocatorDefault, mInternals->mStringRef,
									delimiterString.mInternals->mStringRef);

		for (CFIndex i = 0; i < ::CFArrayGetCount(arrayRef); i++)
			array += CString((CFStringRef) ::CFArrayGetValueAtIndex(arrayRef, i));
		::CFRelease(arrayRef);
	}
	
	return array;
}

// MARK: Percent escapes methods

//----------------------------------------------------------------------------------------------------------------------
CString& CString::convertToPercentEscapes()
//----------------------------------------------------------------------------------------------------------------------
{
	// Perform system conversion
	CFStringRef	stringRef =
						::CFURLCreateStringByAddingPercentEscapes(kCFAllocatorDefault, mInternals->mStringRef, nil,
								nil, kCFStringEncodingUTF8);

	if (stringRef != nil) {
		::CFRelease(mInternals->mStringRef);
		mInternals->mStringRef = ::CFStringCreateMutableCopy(kCFAllocatorDefault, 0, stringRef);
		::CFRelease(stringRef);
	}
	
	// Convert other characters
	::CFStringFindAndReplace(mInternals->mStringRef, CFSTR(";"), CFSTR("%3B"),
			::CFRangeMake(0, ::CFStringGetLength(mInternals->mStringRef)), 0);
	::CFStringFindAndReplace(mInternals->mStringRef, CFSTR("@"), CFSTR("%40"),
			::CFRangeMake(0, ::CFStringGetLength(mInternals->mStringRef)), 0);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CString& CString::convertFromPercentEscapes()
//----------------------------------------------------------------------------------------------------------------------
{
	// Convert other characters
	::CFStringFindAndReplace(mInternals->mStringRef, CFSTR("%3B"), CFSTR(";"),
			::CFRangeMake(0, ::CFStringGetLength(mInternals->mStringRef)), 0);
	::CFStringFindAndReplace(mInternals->mStringRef, CFSTR("%40"), CFSTR("@"),
			::CFRangeMake(0, ::CFStringGetLength(mInternals->mStringRef)), 0);
	
	// Perform system conversion
	CFStringRef	stringRef =
						::CFURLCreateStringByReplacingPercentEscapes(kCFAllocatorDefault, mInternals->mStringRef,
								CFSTR(""));
	AssertNotNil(stringRef);
	if (stringRef == nil)
		return *this;

	::CFRelease(mInternals->mStringRef);
	mInternals->mStringRef = ::CFStringCreateMutableCopy(kCFAllocatorDefault, 0, stringRef);
	::CFRelease(stringRef);
	
	return *this;
}

// MARK: Comparison methods

//----------------------------------------------------------------------------------------------------------------------
ECompareResult CString::compareTo(const CString& string, EStringCompareFlags flags) const
//----------------------------------------------------------------------------------------------------------------------
{
	ECompareResult	compareResult =
							(ECompareResult) ::CFStringCompare(mInternals->mStringRef, string.mInternals->mStringRef,
									sGetCFOptionFlagsForCStringOptionFlags(flags));

	return compareResult;
}

//----------------------------------------------------------------------------------------------------------------------
bool CString::equals(const CString& string, EStringCompareFlags flags) const
//----------------------------------------------------------------------------------------------------------------------
{
	return ::CFStringCompare(mInternals->mStringRef, string.mInternals->mStringRef,
			sGetCFOptionFlagsForCStringOptionFlags(flags)) == kCFCompareEqualTo;
}

//----------------------------------------------------------------------------------------------------------------------
bool CString::beginsWith(const CString& string) const
//----------------------------------------------------------------------------------------------------------------------
{
	return ::CFStringHasPrefix(mInternals->mStringRef, string.mInternals->mStringRef);
}

//----------------------------------------------------------------------------------------------------------------------
bool CString::endsWith(const CString& string) const
//----------------------------------------------------------------------------------------------------------------------
{
	return ::CFStringHasSuffix(mInternals->mStringRef, string.mInternals->mStringRef);
}

//----------------------------------------------------------------------------------------------------------------------
bool CString::contains(const CString& string, EStringCompareFlags flags) const
//----------------------------------------------------------------------------------------------------------------------
{
	CFRange	range =
					::CFStringFind(mInternals->mStringRef, string.mInternals->mStringRef,
							sGetCFOptionFlagsForCStringOptionFlags(flags));

	return range.location != kCFNotFound;
}

// MARK: Convenience operators

//----------------------------------------------------------------------------------------------------------------------
CString& CString::operator=(const CString& other)
//----------------------------------------------------------------------------------------------------------------------
{
	::CFStringReplaceAll(mInternals->mStringRef, other.mInternals->mStringRef);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CString& CString::operator+=(const CString& other)
//----------------------------------------------------------------------------------------------------------------------
{
	// Append
	::CFStringAppend(mInternals->mStringRef, other.mInternals->mStringRef);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CString CString::operator+(const CString& other) const
//----------------------------------------------------------------------------------------------------------------------
{
	CString	string(*this);
	string += other;
	
	return string;
}

// MARK: Utility methods

//----------------------------------------------------------------------------------------------------------------------
CString CString::make(const char* format, va_list args)
//----------------------------------------------------------------------------------------------------------------------
{
	CFStringRef	stringRef = ::CFStringCreateWithCString(kCFAllocatorDefault, format, kCFStringEncodingMacRoman);

	CString	string;
	::CFStringAppendFormatAndArguments(string.mInternals->mStringRef, nil, stringRef, args);
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
	mInternals = new CStringInternals();
	mInternals->mStringRef = ::CFStringCreateMutable(kCFAllocatorDefault, 0);
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
		case kStringEncodingInvalid:	return kCFStringEncodingInvalidId;
		case kStringEncodingUTF8:		return kCFStringEncodingUTF8;
		case kStringEncodingISOLatin:	return kCFStringEncodingISOLatin1;
		case kStringEncodingUnicode:	return kCFStringEncodingUnicode;
		case kStringEncodingUTF16:		return kCFStringEncodingUTF16;
		case kStringEncodingUTF16BE:	return kCFStringEncodingUTF16BE;
		case kStringEncodingUTF16LE:	return kCFStringEncodingUTF16LE;
		case kStringEncodingUTF32:		return kCFStringEncodingUTF32;
		case kStringEncodingUTF32BE:	return kCFStringEncodingUTF32BE;
		case kStringEncodingUTF32LE:	return kCFStringEncodingUTF32LE;
		case kStringEncodingMacRoman:	return kCFStringEncodingMacRoman;
		default:						return kCFStringEncodingUTF8;
	}
}
