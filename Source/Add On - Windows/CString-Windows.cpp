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
CString::CString(OSStringVar(initialString)) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mString = std::basic_string<TCHAR>(initialString);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(const void* ptr, UInt32 byteCount, Encoding encoding) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Parameter check
	AssertNotNil(ptr);

	// Check length
	if (byteCount > 0) {
		// Check encoding
		if (encoding == kEncodingUTF16) {
			// Determine byte order
			AssertFailIf(byteCount < 2);

			// Check first 2 bytes
			const	UInt8*	bytePtr = (const UInt8*) ptr;
			if ((bytePtr[0] == 0xFE) && (bytePtr[1] == 0xFF)) {
				// Big-endian
				ptr = bytePtr + 2;
				byteCount -= 2;
				encoding = kEncodingUTF16BE;
			} else if ((bytePtr[0] == 0xFF) && (bytePtr[1] == 0xFE)) {
				// Little-endian
				ptr = bytePtr + 2;
				byteCount -= 2;
				encoding = kEncodingUTF16LE;
			} else
				// Unknown
				AssertFail();
		}

		// Resize
		mString.resize(byteCount);

		// Convert
		int	count =
				::MultiByteToWideChar(sGetCodePageForCStringEncoding(encoding), 0, (const char*) ptr, byteCount,
						&mString[0], byteCount);
		AssertFailIf(count == 0);
		mString.resize(count);
	}
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(const TBuffer<UTF32Char>& buffer) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check length
	UInt32	charCount = buffer.getCount();
	if (charCount > 0) {
		// Setup string
		mString.resize(charCount);

		// Convert
		int	count =
					::MultiByteToWideChar(sGetCodePageForCStringEncoding(kEncodingUTF32Native), 0,
							(const char*) *buffer, charCount, &mString[0], charCount);
		AssertFailIf(count == 0);
	}
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
	// Setup
	mString.resize(100);

	// Check field size
	int	count;
	if (fieldSize == 0)
		// Compose string without field size
		count = ::_stprintf_s(&mString[0], 100, _TEXT("%d"), value);
	else
		// Compose string with field size
		count = ::_stprintf_s(&mString[0], 100, padWithZeros ? _TEXT("%.*d") : _TEXT("%*d"), fieldSize, value);

	// Update
	mString.resize(count);
}

//----------------------------------------------------------------------------------------------------------------------
CString::CString(SInt16 value, UInt32 fieldSize, bool padWithZeros) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mString.resize(100);

	// Check field size
	int	count;
	if (fieldSize == 0)
		// Compose string without field size
		count = ::_stprintf_s(&mString[0], 100, _TEXT("%d"), value);
	else
		// Compose string with field size
		count = ::_stprintf_s(&mString[0], 100, padWithZeros ? _TEXT("%.*d") : _TEXT("%*d"), fieldSize, value);

	// Update
	mString.resize(count);
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
CString::CString(const TArray<CString>& components, const CString& separator) : CHashable()
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
const CString::C CString::getUTF8String() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	int	length = (int) mString.length();
	C	c(length * 4 + 1);

	// Check if need any conversion
	if (length > 0) {
		// Convert
		int	count =
					::WideCharToMultiByte(sGetCodePageForCStringEncoding(kEncodingUTF8), 0, mString.c_str(), length,
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
TBuffer<char> CString::getUTF8Chars() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	int	length = (int) mString.length();

	// Check length
	if (length > 0) {
		// Setup
		TBuffer<char>	buffer(length * 4);

		// Convert
		int	count =
					::WideCharToMultiByte(sGetCodePageForCStringEncoding(kEncodingUTF8), 0, mString.c_str(), length,
							(char*) *buffer, length * 4, NULL, NULL);

		return TBuffer<char>(buffer, count);
	} else
		// Empty
		return TBuffer<char>(0);
}

//----------------------------------------------------------------------------------------------------------------------
TBuffer<UTF32Char> CString::getUTF32Chars() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Collect characters
	TBuffer<UTF32Char>	buffer((UInt32) mString.length());
	::WideCharToMultiByte(sGetCodePageForCStringEncoding(kEncodingUTF32Native), 0, mString.c_str(),
			(int) mString.length(), (char*) *buffer, (int) mString.length() * 4, NULL, NULL);

	return buffer;
}

//----------------------------------------------------------------------------------------------------------------------
OV<CData> CString::getData(Encoding encoding) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check length
	int	length = (int) mString.length();
	if (length > 0) {
		// Setup
		CData	data((CData::ByteCount) length * 4);
		UINT	codePage = sGetCodePageForCStringEncoding((encoding == kEncodingUTF16) ? kEncodingUTF16LE : encoding);

		// Convert
		int	count =
					::WideCharToMultiByte(codePage, 0, mString.c_str(), length, (char*) data.getMutableBytePtr(),
							length * 4, NULL, NULL);
		data.setByteCount(count);

		if (encoding == kEncodingUTF16)
			// Add BOM
			data = CData((UInt8) 0xFF) + CData((UInt8) 0xFE) + data;

		return OV<CData>(data);
	} else
		// Empty
		return OV<CData>(CData::mEmpty);
}

//----------------------------------------------------------------------------------------------------------------------
CString CString::getSubString(CharIndex startIndex, OV<Length> length) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if need to limit length
	size_t	lengthUse =
					(length.hasValue() && (((size_t) startIndex + (size_t) *length) <= mString.length())) ?
							*length : mString.length() - startIndex;

	// Create string
	CString	string;
	string.mString = mString.substr(startIndex, lengthUse);

	return string;
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
OV<SRange32> CString::findSubString(const CString& subString, CharIndex startIndex, const OV<Length>& length) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Find
	size_t	pos = mString.find(subString.mString, startIndex);
	if (pos == std::string::npos)
		// Not found
		return OV<SRange32>();

	return (!length.hasValue() || ((pos + subString.mString.length()) <= (startIndex + *length))) ?
			OV<SRange32>(SRange32((UInt32) pos, (UInt32) subString.mString.length())) : OV<SRange32>();
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
	// Setup
	size_t	startPos = mString.find_first_not_of(L" \t");
	size_t	endPos = mString.find_last_not_of(L" \t");

	return ((startPos != std::basic_string<TCHAR>::npos) && (endPos != std::basic_string<TCHAR>::npos)) ?
		CString(mString.substr(startPos, endPos - startPos + 1).c_str()) : CString(mString.c_str());
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
bool CString::compareTo(const CString& other, CompareToOptions compareToOptions) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	DWORD	compareFlags = 0;
	if (compareFlags & kCompareToOptionsCaseInsensitive)
		// Case insensitive
		compareFlags |= LINGUISTIC_IGNORECASE;

	if (compareFlags & kCompareToOptionsNonliteral)
		// Non-literal
		compareFlags |= LINGUISTIC_IGNOREDIACRITIC | NORM_IGNOREKANATYPE;

	if (compareFlags & kCompareToOptionsNumerically)
		// Numeric
		compareFlags |= SORT_DIGITSASNUMBERS;

	return ::CompareStringEx(LOCALE_NAME_USER_DEFAULT, compareFlags, mString.c_str(), (int) mString.length(),
			other.mString.c_str(), (int) other.mString.length(), NULL, NULL, 0) != CSTR_GREATER_THAN;
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
bool CString::contains(const CString& other, ContainsOptions containsOptions) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	auto	length = other.mString.length();
	if (length > mString.length())
		// other is longer than us
		return false;

	// Setup
	DWORD	compareFlags = 0;
	if (containsOptions & kContainsOptionsCaseInsensitive)
		// Case insensitive
		compareFlags |= LINGUISTIC_IGNORECASE;

	if (containsOptions & kContainsOptionsNonliteral)
		// Non-literal
		compareFlags |= LINGUISTIC_IGNOREDIACRITIC | NORM_IGNOREKANATYPE;

	// Iterate through looking for string
	for (size_t i = 0; i <= mString.length() - length; i++) {
		// Compare
		if (::CompareStringEx(LOCALE_NAME_USER_DEFAULT, compareFlags, mString.substr(i, length).c_str(), (int) length,
				other.mString.c_str(), (int) length, NULL, NULL, 0) == CSTR_EQUAL)
			// Match!
			return true;
	}

	return false;
}

//----------------------------------------------------------------------------------------------------------------------
bool CString::equals(const CString& other, ContainsOptions containsOptions) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	auto	length = other.mString.length();
	if (length != mString.length())
		// Length differs
		return false;

	// Setup
	DWORD	compareFlags = 0;
	if (containsOptions & kContainsOptionsCaseInsensitive)
		// Case insensitive
		compareFlags |= LINGUISTIC_IGNORECASE;

	if (containsOptions & kContainsOptionsNonliteral)
		// Non-literal
		compareFlags |= LINGUISTIC_IGNOREDIACRITIC | NORM_IGNOREKANATYPE;

	return ::CompareStringEx(LOCALE_NAME_USER_DEFAULT, compareFlags, mString.c_str(), (int) length,
				other.mString.c_str(), (int) length, NULL, NULL, 0) == CSTR_EQUAL;
}

//----------------------------------------------------------------------------------------------------------------------
bool CString::containsOnly(CharacterSet characterSet) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check character set
	switch (characterSet) {
	case kCharacterSetControl:
		// Control
		return std::all_of(mString.begin(), mString.end(), ::iscntrl);

	case kCharacterSetWhitespace:
		// Whitespace
		return std::all_of(mString.begin(), mString.end(), [](TCHAR ch) { return ch == L' ' || ch == L'\t'; });

	case kCharacterSetWhitespaceAndNewline:
		// Whitespace and newline
		return std::all_of(mString.begin(), mString.end(), ::isspace);

	case kCharacterSetDecimalDigit:
		// Decimal digit
		return std::all_of(mString.begin(), mString.end(), ::isdigit);

	case kCharacterSetLetter:
		// Letter
		return std::all_of(mString.begin(), mString.end(), ::isalpha);

	case kCharacterSetLowercaseLetter:
		// Lowercase letter
		return std::all_of(mString.begin(), mString.end(), ::islower);

	case kCharacterSetUppercaseLetter:
		// Uppercase letter
		return std::all_of(mString.begin(), mString.end(), ::isupper);

	case kCharacterSetAlphaNumeric:
		// Alpha numeric
		return std::all_of(mString.begin(), mString.end(), ::isalnum);

	case kCharacterSetPunctuation:
		// Puncuation
		return std::all_of(mString.begin(), mString.end(), ::ispunct);

	default:
		return false;
	}
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
void CString::setupLocalization(const CData& stringsFileData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate lines
	TArray<CString>	lines =
							CString(stringsFileData, kEncodingUTF8)
									.replacingSubStrings(CString(OSSTR("\\U00B5")), CString(OSSTR("\u00B5")))
									.components(CString::mPlatformDefaultNewline);
	for (TIteratorD<CString> iterator = lines.getIterator(); iterator.hasValue(); iterator.advance()) {
		// Process line
		TArray<CString>	components = iterator->components(CString(OSSTR(" = ")));
		if (components.getCount() == 2)
			// Have a localization line
			sLocalizationInfo.set(components[0].getSubString(1, components[0].getLength() - 2),
					components[1].getSubString(1, components[1].getLength() - 3)
							.replacingSubStrings(CString(OSSTR("\\\"")), CString::mDoubleQuote)
							.replacingSubStrings(CString(OSSTR("\\n")), CString::mNewline));
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
UINT sGetCodePageForCStringEncoding(CString::Encoding encoding)
//----------------------------------------------------------------------------------------------------------------------
{
	switch (encoding) {
		case CString::kEncodingASCII:		return 20127;
		case CString::kEncodingMacRoman:	return CP_MACCP;
		case CString::kEncodingUTF8:		return CP_UTF8;
		case CString::kEncodingISOLatin:	return 1252;
		case CString::kEncodingUTF16BE:		return 1201;
		case CString::kEncodingUTF16LE:		return 1200;
		case CString::kEncodingUTF32BE:		return 12001;
		case CString::kEncodingUTF32LE:		return 12000;

		default:
			AssertFailUnimplemented();

			return 0;
	}
}
