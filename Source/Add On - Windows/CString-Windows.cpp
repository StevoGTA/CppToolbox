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
// MARK: Local data

static	CDictionary	sLocalizationInfo;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: Local proc declarations

static	bool	sIsDigit(int character);
static	int		sCompare(const std::basic_string<TCHAR>& string1, const std::basic_string<TCHAR>& string2,
						CString::CompareFlags compareFlags);
static	UINT	sGetCodePageForCStringEncoding(CString::Encoding encoding);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CString

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CString::CString() : CHashable(), mString()
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(const CString& other) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Make copy
	mString = other.mString;
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(const CString& other, Length length) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mString = std::basic_string<TCHAR>(other.mString, length);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(OSStringVar(initialString)) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mString = std::basic_string<TCHAR>(initialString);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(OSStringVar(initialString), Length length) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mString = std::basic_string<TCHAR>(initialString, length);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(const char* chars, Length length, Encoding encoding) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertNotNil(chars);

	// Create string
	mString.resize(length);

	// Check length
	if (length > 0) {
		// Convert
		int	count =
				::MultiByteToWideChar(sGetCodePageForCStringEncoding(encoding), 0, chars, length, &mString[0], length);
		AssertFailIf(count == 0);
	}
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(const char* chars, Encoding encoding) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertNotNil(chars);

	// Setup
	Length	length = (Length) ::strlen(chars);

	// Create string
	mString.resize(length);

	// Check length
	if (length > 0) {
		// Convert
		int	count =
				::MultiByteToWideChar(sGetCodePageForCStringEncoding(encoding), 0, chars, length, &mString[0], length);
		AssertFailIf(count == 0);
	}
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
		// Compose string without field size
		count = ::_stprintf_s(&mString[0], 100, _TEXT("%0.*f"), digitsAfterDecimalPoint, value);
	else
		// Compose string with field size
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
		// Compose string without field size
		count = ::_stprintf_s(&mString[0], 100, _TEXT("%0.*f"), digitsAfterDecimalPoint, value);
	else
		// Compose string with field size
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
		// Compose string without field size
		count = ::_stprintf_s(&mString[0], 100, _TEXT("%ld"), value);
	else
		// Compose string with field size
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
		// Compose string without field size
		count = ::_stprintf_s(&mString[0], 100, _TEXT("%lld"), value);
	else
		// Compose string with field size
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
		// Compose string without field size
		count = ::_stprintf_s(&mString[0], 100, _TEXT("%u"), value);
	else {
		// Setup
		TCHAR*	format;
		if (makeHex)
			// Make hex
			format = padWithZeros ? _TEXT("%#.*x") : _TEXT("%#*x");
		else
			// Make unsigned
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
		// Compose string without field size
		count = ::_stprintf_s(&mString[0], 100, _TEXT("%u"), value);
	else {
		// Setup
		TCHAR*	format;
		if (makeHex)
			// Make hex
			format = padWithZeros ? _TEXT("%#.*x") : _TEXT("%#*x");
		else
			// Make unsigned
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
		// Compose string without field size
		count = ::_stprintf_s(&mString[0], 100, _TEXT("%lu"), value);
	else {
		// Setup
		TCHAR*	format;
		if (makeHex)
			// Make hex
			format = padWithZeros ? _TEXT("%#.*lx") : _TEXT("%#*lx");
		else
			// Make unsigned
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
		// Compose string without field size
		count = ::_stprintf_s(&mString[0], 100, _TEXT("%llu"), value);
	else {
		// Setup
		TCHAR*	format;
		if (makeHex)
			// Make hex
			format = padWithZeros ? _TEXT("%#.*llx") : _TEXT("%#*llx");
		else
			// Make unsigned
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
CString::CString(const CString& localizationGroup, const CString& localizationKey, const CDictionary& localizationInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Retrieve from info
	mString =
			sLocalizationInfo.getString(localizationGroup + CString::mPeriod + localizationKey, localizationKey)
					.mString;

	// Replace values
	for (TIteratorS<CDictionary::Item> iterator = localizationInfo.getIterator(); iterator.hasValue();
			iterator.advance()) {
		// Compose value
		CString	replacement;
		switch (iterator->mValue.getType()) {
			case SValue::kTypeBool:
				// Bool
				replacement = iterator->mValue.getBool() ? CString(OSSTR("true")) : CString(OSSTR("false"));
				break;

			case SValue::kTypeString:	replacement = iterator->mValue.getString();				break;
			case SValue::kTypeFloat32:	replacement = CString(iterator->mValue.getFloat32());	break;
			case SValue::kTypeFloat64:	replacement = CString(iterator->mValue.getFloat64());	break;
			case SValue::kTypeSInt8:	replacement = CString(iterator->mValue.getSInt8());		break;
			case SValue::kTypeSInt16:	replacement = CString(iterator->mValue.getSInt16());	break;
			case SValue::kTypeSInt32:	replacement = CString(iterator->mValue.getSInt32());	break;
			case SValue::kTypeSInt64:	replacement = CString(iterator->mValue.getSInt64());	break;
			case SValue::kTypeUInt8:	replacement = CString(iterator->mValue.getUInt8());		break;
			case SValue::kTypeUInt16:	replacement = CString(iterator->mValue.getUInt16());	break;
			case SValue::kTypeUInt32:	replacement = CString(iterator->mValue.getUInt32());	break;
			case SValue::kTypeUInt64:	replacement = CString(iterator->mValue.getUInt64());	break;

			case SValue::kTypeEmpty:
			case SValue::kTypeArrayOfDictionaries:
			case SValue::kTypeArrayOfStrings:
			case SValue::kTypeOpaque:
			case SValue::kTypeData:
			case SValue::kTypeDictionary:
				// Unhandled
				replacement = CString(OSSTR("->UNHANDLED<-"));
				break;
		}

		// Replace
		size_t	offset;
		while ((offset = mString.find(iterator->mKey.mString.c_str(), 0)) != std::basic_string<TCHAR>::npos)
			// Replace this substring
			mString.replace(offset, iterator->mKey.mString.length(), replacement.mString.c_str());
	}
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(const CString& localizationGroup, const CString& localizationKey)
//----------------------------------------------------------------------------------------------------------------------
{
	// Retrieve from info
	mString =
			sLocalizationInfo.getString(localizationGroup + CString::mPeriod + localizationKey, localizationKey)
					.mString;
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
CString CString::getSubString(CharIndex startIndex, OV<Length> length) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if need to limit length
	size_t	lengthUse;
	if (length.hasValue() && (((size_t) startIndex + (size_t) *length) <= mString.length()))
		// Use requested
		lengthUse = *length;
	else
		// Limit to remainder of string
		lengthUse = mString.length() - startIndex;

	// Setup
	TBuffer<TCHAR>	buffer((UInt32) lengthUse);
	size_t			count = mString._Copy_s(*buffer, lengthUse, lengthUse, startIndex);

	return CString(*buffer, (Length) count);
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
	while ((offset = string.mString.find(subStringToReplace.mString.c_str(), 0)) != std::basic_string<TCHAR>::npos)
		// Replace this substring
		string.mString.replace(offset, substringToReplaceLength, replacementString.mString.c_str());

	return string;
}

//----------------------------------------------------------------------------------------------------------------------
CString CString::replacingCharacters(CharIndex startIndex, OV<Length> length, const CString& replacementString) const
//----------------------------------------------------------------------------------------------------------------------
{
	AssertFailUnimplemented();
return CString::mEmpty;
}

//----------------------------------------------------------------------------------------------------------------------
CString::Range CString::findSubString(const CString& subString, CharIndex startIndex, OV<Length> length) const
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
	// Setup
	size_t	startPos = mString.find_first_not_of(L" \t");
	size_t	endPos = mString.find_last_not_of(L" \t");

	return ((startPos != std::basic_string<TCHAR>::npos) && (endPos != std::basic_string<TCHAR>::npos)) ?
			CString(mString.substr(startPos, endPos - startPos + 1).c_str()) : CString(mString.c_str());
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
TArray<CString> CString::components(const CString& delimiterString, bool includeEmptyComponents) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNArray<CString>	array;

	// Iterate all substrings
	size_t	startOffset = 0;
	size_t	endOffset = mString.find(delimiterString.mString.c_str(), startOffset);
	while (endOffset != std::basic_string<TCHAR>::npos) {
		// Check if adding
		if (includeEmptyComponents || (endOffset > startOffset))
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
	// Return if this string is "less than" the other string
	return sCompare(mString, other.mString, compareFlags) == -1;
}

//----------------------------------------------------------------------------------------------------------------------
bool CString::equals(const CString& other, CString::CompareFlags compareFlags) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Return if this string is "equal to" the other string
	return sCompare(mString, other.mString, compareFlags) == 0;
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

//----------------------------------------------------------------------------------------------------------------------
void CString::setupLocalization(const CData& stringsFileData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate lines
	TArray<CString>	lines =
							CString(stringsFileData)
									.replacingSubStrings(CString(OSSTR("\\U00B5")), CString(OSSTR("\u00B5")))
									.components(CString::mNewline);
	for (TIteratorD<CString> iterator = lines.getIterator(); iterator.hasValue(); iterator.advance()) {
		// Process line
		TArray<CString>	components = iterator->components(CString(OSSTR(" = ")));
		if (components.getCount() == 2)
			// Have a localization line
			sLocalizationInfo.set(components[0].getSubString(1, components[0].getLength() - 2),
					components[1].getSubString(1, components[1].getLength() - 4)
							.replacingSubStrings(CString(OSSTR("\\\"")), CString::mDoubleQuotes));
	}
}

// MARK: Internal methods

//----------------------------------------------------------------------------------------------------------------------
void CString::init()
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions

//----------------------------------------------------------------------------------------------------------------------
bool sIsDigit(int character)
//----------------------------------------------------------------------------------------------------------------------
{
	return std::isdigit(character);
}

//----------------------------------------------------------------------------------------------------------------------
int sCompare(const std::basic_string<TCHAR>& string1, const std::basic_string<TCHAR>& string2,
		CString::CompareFlags compareFlags)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if comparing numerically
	if (compareFlags & CString::kCompareFlagsNumerically) {
		// Compare as chunks
		auto	iterator1 = string1.begin();
		auto	iterator2 = string2.begin();
		while ((iterator1 != string1.end()) && (iterator2 != string2.end())) {
			// Check if next compare character is digit
			if (sIsDigit(*iterator1) && sIsDigit(*iterator2)) {
				// Is digit
				auto number1 = std::stoi(std::basic_string<TCHAR>(iterator1, std::find_if_not(iterator1, string1.end(), sIsDigit)));
				auto number2 = std::stoi(std::basic_string<TCHAR>(iterator2, std::find_if_not(iterator2, string2.end(), sIsDigit)));
				if (number1 != number2)
					// Numbers are different
					return number1 < number2;

				// Advance iterators
				iterator1 = std::find_if_not(iterator1, string1.end(), sIsDigit);
				iterator2 = std::find_if_not(iterator2, string2.end(), sIsDigit);
			} else {
				// Not digit
#if defined(_UNICODE)
				auto	character1 = (compareFlags & CString::kCompareFlagsCaseInsensitive) ? towlower(*iterator1) : *iterator1;
				auto	character2 = (compareFlags & CString::kCompareFlagsCaseInsensitive) ? towlower(*iterator2) : *iterator2;
#else
				auto	character1 = (compareFlags & CString::kCompareFlagsCaseInsensitive) ? tolower(*iterator1) : *iterator1;
				auto	character2 = (compareFlags & CString::kCompareFlagsCaseInsensitive) ? tolower(*iterator2) : *iterator2;
#endif
				if (character1 != character2)
					// Characters are different
					return character1 < character2;

				// Advance iterators
				++iterator1;
				++iterator2;
			}
		}

		// Check result
		if (iterator1 == string1.end())
			// 1 before 2
			return -1;
		else if (iterator2 == string2.end())
			// 1 after 2
			return 1;
		else
			// Identical
			return 0;
	} else {
		// Compare as text
		auto	string1Use = string1;
		auto	string2Use = string2;

		// Check if comparing case insensitive
		if (compareFlags & CString::kCompareFlagsCaseInsensitive) {
			// Transform to lowercase
	#if defined(_UNICODE)
			std::transform(string1Use.begin(), string1Use.end(), string1Use.begin(), ::towlower);
			std::transform(string2Use.begin(), string2Use.end(), string2Use.begin(), ::towlower);
	#else
			std::transform(string1Use.begin(), string1Use.end(), string1Use.begin(), ::tolower);
			std::transform(string2Use.begin(), string2Use.end(), string2Use.begin(), ::tolower);
	#endif
		}

		return string1Use.compare(string2Use);
	}
}

//----------------------------------------------------------------------------------------------------------------------
UINT sGetCodePageForCStringEncoding(CString::Encoding encoding)
//----------------------------------------------------------------------------------------------------------------------
{
	switch (encoding) {
		case CString::kEncodingASCII:		return 20127;
		case CString::kEncodingMacRoman:	return CP_MACCP;
		case CString::kEncodingUTF8:		return CP_UTF8;
		case CString::kEncodingISOLatin:	return 1252;
		//case kStringEncodingUnicode:	return kCFStringEncodingUnicode;

		//case CString::kEncodingUTF16:		return kCFStringEncodingUTF16;
		case CString::kEncodingUTF16BE:		return 1201;
		case CString::kEncodingUTF16LE:		return 1200;
		//case CString::kEncodingUTF32:		return kCFStringEncodingUTF32;
		case CString::kEncodingUTF32BE:		return 12001;
		case CString::kEncodingUTF32LE:		return 12000;

		default:
			AssertFailUnimplemented();

			return 0;
	}
}
