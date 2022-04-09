//----------------------------------------------------------------------------------------------------------------------
//	CString-Windows.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CString.h"

#include "CData.h"
#include "SError.h"
#include "TBuffer.h"

#include <vector>

#undef Delete
#include <Windows.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local proc declarations

 static	UINT	sGetCodePageForCStringEncoding(CString::Encoding encoding);

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
CString::CString(const CString& other, OV<Length> length) : CHashable()
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
CString::CString(OSStringVar(initialString), OV<Length> length) : CHashable()
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
CString::CString(const char* chars, Length charsCount, Encoding encoding) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertNotNil(chars);

	// Check if have size
	if (charsCount == ~0)
		// Count chars
		charsCount = (Length) ::strlen(chars);

	// Create string
	mString.resize(charsCount);

	// Convert
	int	count =
				::MultiByteToWideChar(sGetCodePageForCStringEncoding(encoding), 0, chars, charsCount, &mString[0],
						charsCount);
	AssertFailIf(count == 0);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(const UTF16Char* chars, Length charsCount, Encoding encoding) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertNotNil(chars);

	AssertFailUnimplemented();
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(const UTF32Char* chars, Length charsCount, Encoding encoding) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertNotNil(chars);
	AssertFailIf((encoding != kEncodingUTF32BE) && (encoding != kEncodingUTF32LE));

	AssertFailUnimplemented();
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(Float32 value, UInt32 fieldSize, UInt32 digitsAfterDecimalPoint, bool padWithZeros) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mString.resize(100);

	// Check field size
	int	count;
	if (fieldSize == 0)
		count = ::_stprintf_s(&mString[0], 100, _TEXT("%0.*f"), digitsAfterDecimalPoint, value);
	else
		count =
				::_stprintf_s(&mString[0], 100, padWithZeros ? _TEXT("%0*.*f") : _TEXT("%*.*f"), fieldSize,
						digitsAfterDecimalPoint, value);

	// Update
	mString.resize(count);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(Float64 value, UInt32 fieldSize, UInt32 digitsAfterDecimalPoint, bool padWithZeros) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mString.resize(100);

	// Check field size
	int	count;
	if (fieldSize == 0)
		count = ::_stprintf_s(&mString[0], 100, _TEXT("%0.*f"), digitsAfterDecimalPoint, value);
	else
		count =
				::_stprintf_s(&mString[0], 100, padWithZeros ? _TEXT("%0*.*f") : _TEXT("%*.*f"), fieldSize,
						digitsAfterDecimalPoint, value);

	// Update
	mString.resize(count);
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
	// Setup
	mString.resize(100);

	// Check field size
	int	count;
	if (fieldSize == 0)
		count = ::_stprintf_s(&mString[0], 100, _TEXT("%ld"), value);
	else
		count = ::_stprintf_s(&mString[0], 100, padWithZeros ? _TEXT("%.*ld") : _TEXT("%*ld"), fieldSize, value);

	// Update
	mString.resize(count);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(SInt64 value, UInt32 fieldSize, bool padWithZeros) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mString.resize(100);

	// Check field size
	int	count;
	if (fieldSize == 0)
		count = ::_stprintf_s(&mString[0], 100, _TEXT("%lld"), value);
	else
		count = ::_stprintf_s(&mString[0], 100, padWithZeros ? _TEXT("%.*lld") : _TEXT("%*lld"), fieldSize, value);

	// Update
	mString.resize(count);
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
		count = ::_stprintf_s(&mString[0], 100, _TEXT("%u"), value);
	else {
		// Setup
		TCHAR*	format;
		if (makeHex)
			format = padWithZeros ? _TEXT("%#.*x") : _TEXT("%#*x");
		else
			format = padWithZeros ? _TEXT("%.*u") : _TEXT("%*u");

		count = ::_stprintf_s(&mString[0], 100, format, fieldSize, value);
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
		count = ::_stprintf_s(&mString[0], 100, _TEXT("%u"), value);
	else {
		// Setup
		TCHAR*	format;
		if (makeHex)
			format = padWithZeros ? _TEXT("%#.*x") : _TEXT("%#*x");
		else
			format = padWithZeros ? _TEXT("%.*u") : _TEXT("%*u");

		count = ::_stprintf_s(&mString[0], 100, format, fieldSize, value);
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
		count = ::_stprintf_s(&mString[0], 100, _TEXT("%lu"), value);
	else {
		// Setup
		TCHAR*	format;
		if (makeHex)
			format = padWithZeros ? _TEXT("%#.*lx") : _TEXT("%#*lx");
		else
			format = padWithZeros ? _TEXT("%.*lu") : _TEXT("%*lu");

		count = ::_stprintf_s(&mString[0], 100, format, fieldSize, value);
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
		count = ::_stprintf_s(&mString[0], 100, _TEXT("%llu"), value);
	else {
		// Setup
		TCHAR*	format;
		if (makeHex)
			format = padWithZeros ? _TEXT("%#.*llx") : _TEXT("%#*llx");
		else
			format = padWithZeros ? _TEXT("%.*llu") : _TEXT("%*llu");

		count = ::_stprintf_s(&mString[0], 100, format, fieldSize, value);
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
	::MultiByteToWideChar(sGetCodePageForCStringEncoding(kEncodingASCII), 0, (char*) &osType, 4, &mString[0], 4);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(const void* pointer) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mString.resize(100);

	// Check field size
	int	count = ::_stprintf_s(&mString[0], 100, _TEXT("%p"), pointer);

	// Update
	mString.resize(count);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(const TArray<CString>& components, const CString& separator) : CHashable(), mString()
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate array
	for (CArray::ItemIndex i = 0; i < components.getCount(); i++) {
		// Check if need to add separator
		if (i > 0)
			// Add separator
			mString += separator.mString;

		// Append this component
		mString += components[i].mString;
	}
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(const CData& data, Encoding encoding) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Ensure we have something to convert
	if (data.getByteCount() > 0) {
		// Setup
		mString.resize((size_t) data.getByteCount());

		// Convert
		int	count =
					::MultiByteToWideChar(sGetCodePageForCStringEncoding(encoding), 0, (char*) data.getBytePtr(),
							(int) data.getByteCount(), &mString[0], (int) data.getByteCount());
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
const CString::C CString::getCString(Encoding encoding) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	int	length = (int) mString.length();
	C	c(length * 4 + 1);

	// Check if need any conversion
	if (length > 0) {
		// Convert
		int	count =
					::WideCharToMultiByte(sGetCodePageForCStringEncoding(encoding), 0, mString.c_str(), length,
							*c, length * 4, NULL, NULL);
		(*c)[count] = 0;
	}

	return c;
}

//----------------------------------------------------------------------------------------------------------------------
CString::Length CString::getLength() const
//----------------------------------------------------------------------------------------------------------------------
{
	return (Length) mString.length();
}

//----------------------------------------------------------------------------------------------------------------------
CString::Length CString::getLength(Encoding encoding, SInt8 lossCharacter, bool forExternalStorageOrTransmission)
		const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return 0;
}

//----------------------------------------------------------------------------------------------------------------------
CString::Length CString::get(char* buffer, Length bufferLen, bool addNull, Encoding encoding) const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return 0;
}

//----------------------------------------------------------------------------------------------------------------------
CString::Length CString::get(UTF16Char* buffer, Length bufferLen, Encoding encoding) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertNotNil(buffer);
	if (buffer == nil)
		return 0;

	AssertFailIf((encoding != kEncodingUTF16BE) && (encoding != kEncodingUTF16LE));
	if ((encoding != kEncodingUTF16BE) && (encoding != kEncodingUTF16LE))
		return 0;

	if (bufferLen > getLength())
		bufferLen = getLength();

	AssertFailUnimplemented();

	return 0;
}

//----------------------------------------------------------------------------------------------------------------------
UTF32Char CString::getCharacterAtIndex(CharIndex index) const
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
CData CString::getData(Encoding encoding, SInt8 lossCharacter) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check length
	int	length = (int) mString.length();
	if (length > 0) {
		// Setup
		CData	data((CData::ByteCount) length * 4);

		// Convert
		int	count =
					::WideCharToMultiByte(sGetCodePageForCStringEncoding(encoding), 0, mString.c_str(), length,
							(LPSTR) data.getMutableBytePtr(), length * 4, NULL, NULL);
		data.setByteCount(count);

		return data;
	} else
		// Empty
		return CData::mEmpty;
}

//----------------------------------------------------------------------------------------------------------------------
CString CString::getSubString(CharIndex startIndex, Length charCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if need to limit char count
	if (((UInt64) startIndex + (UInt64) charCount) > (UInt64) mString.length())
		// Limit char count
		charCount = (Length) (mString.length() - startIndex);

	// Setup
	TBuffer<TCHAR>	buffer(charCount);
	size_t			count = mString._Copy_s(*buffer, charCount, charCount, startIndex);

	return CString(*buffer, OV<Length>((Length) count));
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
CString CString::replacingCharacters(CharIndex startIndex, Length charCount, const CString& replacementString) const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return CString::mEmpty;
}

//----------------------------------------------------------------------------------------------------------------------
CString::Range CString::findSubString(const CString& subString, CharIndex startIndex, Length charCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return Range(0, 0);
}

//----------------------------------------------------------------------------------------------------------------------
CString CString::lowercased() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CString	string(*this);

	// Transform
#if defined(_UNICODE)
	std::transform(string.mString.begin(), string.mString.end(), string.mString.begin(), ::towlower);
#else
	std::transform(string.mString.begin(), string.mString.end(), string.mString.begin(), ::tolower);
#endif

	return string;
}

//----------------------------------------------------------------------------------------------------------------------
CString CString::uppercased() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CString	string(*this);

	// Transform
#if defined(_UNICODE)
	std::transform(string.mString.begin(), string.mString.end(), string.mString.begin(), ::towupper);
#else
	std::transform(string.mString.begin(), string.mString.end(), string.mString.begin(), ::toupper);
#endif

	return string;
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
TArray<CString> CString::components(const CString& delimiterString) const
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
bool CString::compareTo(const CString& other, CString::CompareFlags compareFlags) const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return false;
}

//----------------------------------------------------------------------------------------------------------------------
bool CString::equals(const CString& other, CString::CompareFlags compareFlags) const
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
bool CString::contains(const CString& other, CString::CompareFlags compareFlags) const
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
CString CString::make(OSStringType format, va_list args)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	std::vector<TCHAR>::size_type	size = 256;
	std::vector<TCHAR>				buffer;

	// Try to write
	do {
		// Resize to given size
		buffer.resize(size + 1);

		// Try to write and get needed size
		size = ::vswprintf_s(&buffer[0], buffer.size(), format, args);
	} while ((size + 1) > buffer.size());

	// Compose CString
	CString	string;
	string.mString = std::basic_string<TCHAR>(&buffer[0]);

	return string;
}

//----------------------------------------------------------------------------------------------------------------------
bool CString::isCharacterInSet(UTF32Char utf32Char, CharacterSet characterSet)
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
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions

//----------------------------------------------------------------------------------------------------------------------
UINT sGetCodePageForCStringEncoding(CString::Encoding encoding)
{
	switch (encoding) {
		case CString::kEncodingASCII:		return 20127;
		case CString::kEncodingMacRoman:	return CP_MACCP;
		case CString::kEncodingUTF8:		return CP_UTF8;
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
