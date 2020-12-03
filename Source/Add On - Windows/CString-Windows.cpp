//----------------------------------------------------------------------------------------------------------------------
//	CString-Windows.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CString.h"

//#include "CppToolboxAssert.h"
#include "CData.h"
//#include "CLogServices.h"
#include "SError.h"
#include "TBuffer.h"

#undef Delete
#include <Windows.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local proc declarations

 static	UINT	sGetCodePageForCStringEncoding(EStringEncoding encoding);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CString

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CString::CString() : CHashable(), mString()
//----------------------------------------------------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(const CString& other, OV<CStringLength> length) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check for length
	if (length.hasValue())
		// Have length
		mString = std::basic_string<TCHAR>(other.mString, length.getValue());
	else
		// Don't have length
		mString = other.mString;
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(OSStringVar(initialString), OV<CStringLength> length) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check for length
	if (length.hasValue())
		// Have length
		mString = std::basic_string<TCHAR>(initialString, length.getValue());
	else
		// Don't have length
		mString = std::basic_string<TCHAR>(initialString);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(const char* chars, CStringLength charsCount, EStringEncoding encoding) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertNotNil(chars);

	// Check if have size
	if (charsCount == kCStringDefaultMaxLength)
		// Count chars
		charsCount = (CStringLength) ::strlen(chars);

	// Create string
	mString.resize(charsCount);

	// Convert
	int	count =
				MultiByteToWideChar(sGetCodePageForCStringEncoding(encoding), 0, chars, charsCount, &mString[0],
						charsCount);
	AssertFailIf(count == 0);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(const UTF16Char* chars, CStringLength charsCount, EStringEncoding encoding) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertNotNil(chars);

	AssertFailUnimplemented();
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(const UTF32Char* chars, CStringLength charsCount, EStringEncoding encoding) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertNotNil(chars);
	AssertFailIf((encoding != kStringEncodingUTF32BE) && (encoding != kStringEncodingUTF32LE));

	AssertFailUnimplemented();
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(Float32 value, UInt32 fieldSize, UInt32 digitsAfterDecimalPoint, bool padWithZeros) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(Float64 value, UInt32 fieldSize, UInt32 digitsAfterDecimalPoint, bool padWithZeros) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(SInt8 value, UInt32 fieldSize, bool padWithZeros) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(SInt16 value, UInt32 fieldSize, bool padWithZeros) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(SInt32 value, UInt32 fieldSize, bool padWithZeros) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(SInt64 value, UInt32 fieldSize, bool padWithZeros) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(UInt8 value, UInt32 fieldSize, bool padWithZeros, bool makeHex) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mString.resize(100);

	// Check field size
	int	count;
	if (fieldSize == 0)
		count = _stprintf_s(&mString[0], 100, _TEXT("%u"), value);
	else {
		// Setup
		TCHAR*	format;
		if (makeHex)
			format = padWithZeros ? _TEXT("%#.*x") : _TEXT("%#*x");
		else
			format = padWithZeros ? _TEXT("%.*u") : _TEXT("%*u");

		count = _stprintf_s(&mString[0], 100, format, fieldSize, value);
	}

	// Update
	mString.resize(count);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(UInt16 value, UInt32 fieldSize, bool padWithZeros, bool makeHex) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mString.resize(100);

	// Check field size
	int	count;
	if (fieldSize == 0)
		count = _stprintf_s(&mString[0], 100, _TEXT("%u"), value);
	else {
		// Setup
		TCHAR*	format;
		if (makeHex)
			format = padWithZeros ? _TEXT("%#.*x") : _TEXT("%#*x");
		else
			format = padWithZeros ? _TEXT("%.*u") : _TEXT("%*u");

		count = _stprintf_s(&mString[0], 100, format, fieldSize, value);
	}

	// Update
	mString.resize(count);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(UInt32 value, UInt32 fieldSize, bool padWithZeros, bool makeHex) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mString.resize(100);

	// Check field size
	int	count;
	if (fieldSize == 0)
		count = _stprintf_s(&mString[0], 100, _TEXT("%lu"), value);
	else {
		// Setup
		TCHAR*	format;
		if (makeHex)
			format = padWithZeros ? _TEXT("%#.*lx") : _TEXT("%#*lx");
		else
			format = padWithZeros ? _TEXT("%.*lu") : _TEXT("%*lu");

		count = _stprintf_s(&mString[0], 100, format, fieldSize, value);
	}

	// Update
	mString.resize(count);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(UInt64 value, UInt32 fieldSize, bool padWithZeros, bool makeHex) : CHashable(), mString()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mString.resize(100);

	// Check field size
	int	count;
	if (fieldSize == 0)
		count = _stprintf_s(&mString[0], 100, _TEXT("%llu"), value);
	else {
		// Setup
		TCHAR*	format;
		if (makeHex)
			format = padWithZeros ? _TEXT("%#.*llx") : _TEXT("%#*llx");
		else
			format = padWithZeros ? _TEXT("%.*llu") : _TEXT("%*llu");

		count = _stprintf_s(&mString[0], 100, format, fieldSize, value);
	}

	// Update
	mString.resize(count);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(OSType osType, bool isOSType, bool includeQuotes) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	osType = EndianU32_NtoB(osType);

	mString.resize(4);
	MultiByteToWideChar(sGetCodePageForCStringEncoding(kStringEncodingASCII), 0, (char*) &osType, 4, &mString[0], 4);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(const void* pointer)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mString.resize(100);

	// Check field size
	int	count = _stprintf_s(&mString[0], 100, _TEXT("%p"), pointer);

	// Update
	mString.resize(count);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(const CData& data, EStringEncoding encoding)
//----------------------------------------------------------------------------------------------------------------------
{
	// Ensure we have something to convert
	if (data.getSize() > 0) {
		// Setup
		mString.resize(data.getSize());

		// Convert
		int	count =
					MultiByteToWideChar(sGetCodePageForCStringEncoding(encoding), 0, (char*) data.getBytePtr(),
							data.getSize(), &mString[0], data.getSize());
		AssertFailIf(count == 0);
	}
}

//----------------------------------------------------------------------------------------------------------------------
CString::~CString()
//----------------------------------------------------------------------------------------------------------------------
{}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
OSStringType CString::getOSString() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mString.c_str();
}

//----------------------------------------------------------------------------------------------------------------------
const SCString CString::getCString(EStringEncoding encoding) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	int			length = (int) mString.length();
	SCString	cString(length * 4 + 1);

	// Check if need any conversion
	if (length > 0) {
		// Convert
		int	count =
					WideCharToMultiByte(sGetCodePageForCStringEncoding(encoding), 0, mString.c_str(), length,
							cString.mBuffer, length * 4, NULL, NULL);
		cString.mBuffer[count] = 0;
	}

	return cString;
}

//----------------------------------------------------------------------------------------------------------------------
CStringLength CString::getLength() const
//----------------------------------------------------------------------------------------------------------------------
{
	return (CStringLength) mString.length();
}

//----------------------------------------------------------------------------------------------------------------------
CStringLength CString::getLength(EStringEncoding encoding, SInt8 lossCharacter, bool forExternalStorageOrTransmission)
		const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return 0;
}

//----------------------------------------------------------------------------------------------------------------------
CStringLength CString::get(char* buffer, CStringLength bufferLen, bool addNull, EStringEncoding encoding) const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
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

	AssertFailUnimplemented();

	return 0;
}

//----------------------------------------------------------------------------------------------------------------------
UTF32Char CString::getCharacterAtIndex(CStringCharIndex index) const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailIf(index > getLength());
	if (index > getLength())
		return 0;

	AssertFailUnimplemented();

	return 0;
}

//----------------------------------------------------------------------------------------------------------------------
Float32 CString::getFloat32() const
//----------------------------------------------------------------------------------------------------------------------
{
	return std::stof(mString);
}

//----------------------------------------------------------------------------------------------------------------------
Float64 CString::getFloat64() const
//----------------------------------------------------------------------------------------------------------------------
{
	return std::stod(mString);
}

//----------------------------------------------------------------------------------------------------------------------
CData CString::getData(EStringEncoding encoding, SInt8 lossCharacter) const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return CData::mEmpty;
}

//----------------------------------------------------------------------------------------------------------------------
CString CString::getSubString(CStringCharIndex startIndex, CStringLength charCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TBuffer<TCHAR>	buffer(charCount);
	size_t			count = mString._Copy_s(*buffer, charCount, charCount, startIndex);

	return CString(*buffer, OV<CStringLength>((CStringLength) count));
}

//----------------------------------------------------------------------------------------------------------------------
CString CString::replacingSubStrings(const CString& subStringToReplace, const CString& replacementString) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CString								string(*this);
	std::basic_string<TCHAR>::size_type	substringToReplaceLength = subStringToReplace.mString.length();

	// Iterate all substrings
	size_t	offset;
	while ((offset = string.mString.find(subStringToReplace.mString.c_str(), 0)) != std::basic_string<TCHAR>::npos) {
		// Replace this substring
		string.mString.replace(offset, substringToReplaceLength, replacementString.mString.c_str());
	}

	return string;
}

//----------------------------------------------------------------------------------------------------------------------
CString CString::replacingCharacters(CStringCharIndex startIndex, CStringLength charCount,
		const CString& replacementString) const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return CString::mEmpty;
}

//----------------------------------------------------------------------------------------------------------------------
SStringRange CString::findSubString(const CString& subString, CStringCharIndex startIndex, CStringLength charCount)
		const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return SStringRange(0, 0);
}

//----------------------------------------------------------------------------------------------------------------------
CString CString::lowercased() const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return CString::mEmpty;
}

//----------------------------------------------------------------------------------------------------------------------
CString CString::uppercased() const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return CString::mEmpty;
}

//----------------------------------------------------------------------------------------------------------------------
CString CString::removingLeadingAndTrailingWhitespace() const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return CString::mEmpty;
}

//----------------------------------------------------------------------------------------------------------------------
CString CString::removingAllWhitespace() const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return CString::mEmpty;
}

//----------------------------------------------------------------------------------------------------------------------
CString CString::removingLeadingAndTrailingQuotes() const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return CString::mEmpty;
}

//----------------------------------------------------------------------------------------------------------------------
CString CString::getCommonPrefix(const CString& other) const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return CString::mEmpty;
}

//----------------------------------------------------------------------------------------------------------------------
TArray<CString> CString::breakUp(const CString& delimiterString) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNArray<CString>	array;

	// Iterate all substrings
	size_t	startOffset = 0;
	size_t	endOffset = mString.find(delimiterString.mString.c_str(), startOffset);
	while (endOffset != std::basic_string<TCHAR>::npos) {
		// Add to array
		array += CString(mString.substr(startOffset, endOffset - startOffset).c_str());

		// Find next
		startOffset = endOffset + delimiterString.mString.length();
		endOffset = mString.find(delimiterString.mString.c_str(), startOffset);
	}

	// Add last
	array += CString(mString.substr(startOffset, mString.length() - startOffset).c_str());

	return array;
}

// MARK: Comparison methods

//----------------------------------------------------------------------------------------------------------------------
ECompareResult CString::compareTo(const CString& other, EStringCompareFlags flags) const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return kCompareResultEquivalent;
}

//----------------------------------------------------------------------------------------------------------------------
bool CString::equals(const CString& other, EStringCompareFlags flags) const
//----------------------------------------------------------------------------------------------------------------------
{
	return mString.compare(other.mString) == 0;
}

//----------------------------------------------------------------------------------------------------------------------
bool CString::hasPrefix(const CString& other) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	std::basic_string<TCHAR>::size_type	length = mString.length();
	std::basic_string<TCHAR>::size_type	otherLength = other.mString.length();

	return (length >= otherLength) && (mString.compare(0, otherLength, other.mString.c_str()) == 0);
}

//----------------------------------------------------------------------------------------------------------------------
bool CString::hasSuffix(const CString& other) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	std::basic_string<TCHAR>::size_type	length = mString.length();
	std::basic_string<TCHAR>::size_type	otherLength = other.mString.length();

	return (length >= otherLength) &&
			(mString.compare(length - otherLength, other.mString.length(), other.mString.c_str()) == 0);
}

//----------------------------------------------------------------------------------------------------------------------
bool CString::contains(const CString& other, EStringCompareFlags flags) const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return false;
}

// MARK: Convenience operators

//----------------------------------------------------------------------------------------------------------------------
CString& CString::operator=(const CString& other)
//----------------------------------------------------------------------------------------------------------------------
{
	// Copy
	mString = other.mString;

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CString& CString::operator+=(const CString& other)
//----------------------------------------------------------------------------------------------------------------------
{
	// Append
	mString += other.mString;

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CString CString::operator+(const CString& other) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Copy and append
	CString	string(*this);
	string.mString += other.mString;

	return string;
}

// MARK: Utility methods

//----------------------------------------------------------------------------------------------------------------------
CString CString::make(const char* format, va_list args)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return CString::mEmpty;
}

//----------------------------------------------------------------------------------------------------------------------
bool CString::isCharacterInSet(UTF32Char utf32Char, EStringCharacterSet characterSet)
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return false;
}

// MARK: Internal methods

//----------------------------------------------------------------------------------------------------------------------
void CString::init()
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions

//----------------------------------------------------------------------------------------------------------------------
UINT sGetCodePageForCStringEncoding(EStringEncoding encoding)
{
	switch (encoding) {
		case kStringEncodingASCII:		return 20127;
		//case kStringEncodingMacRoman:	return kCFStringEncodingMacRoman;
		case kStringEncodingUTF8:		return CP_UTF8;
		//case kStringEncodingISOLatin:	return kCFStringEncodingISOLatin1;
		//case kStringEncodingUnicode:	return kCFStringEncodingUnicode;

		//case kStringEncodingUTF16:		return kCFStringEncodingUTF16;
		//case kStringEncodingUTF16BE:	return kCFStringEncodingUTF16BE;
		//case kStringEncodingUTF16LE:	return kCFStringEncodingUTF16LE;
		//case kStringEncodingUTF32:		return kCFStringEncodingUTF32;
		//case kStringEncodingUTF32BE:	return kCFStringEncodingUTF32BE;
		//case kStringEncodingUTF32LE:	return kCFStringEncodingUTF32LE;

		default:
			AssertFailUnimplemented();

			return 0;
	}
}
