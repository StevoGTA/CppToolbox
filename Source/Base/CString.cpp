//----------------------------------------------------------------------------------------------------------------------
//	CString.cpp			©2018 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

const	UInt64	kDisplayAsKBThreshhold = 3997;
const	UInt64	kDisplayAsMBThreshHold = 1000 * 1000;
const	UInt64	kDisplayAsGBThreshHold = 1000 * 1000 * 1000;

const	UInt64	kDisplayAsKiBThreshhold = 4097;
const	UInt64	kDisplayAsMiBThreshHold = 1000 * 1024;
const	UInt64	kDisplayAsGiBThreshHold = 1024 * 1024 * 1024;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CString

// MARK: Properties

const	CString	CString::mEmpty;

const	CString	CString::mColon(OSSTR(":"));
const	CString	CString::mComma(OSSTR(","));
const	CString	CString::mDoubleQuotes(OSSTR("\""));
const	CString	CString::mEqualSign(OSSTR("="));
const	CString	CString::mPeriod(OSSTR("."));
const	CString	CString::mSpace(OSSTR(" "));
const	CString	CString::mSpaceX4(OSSTR("    "));
const	CString	CString::mTab(OSSTR("\t"));

const	CString	CString::mNewline(OSSTR("\n"));
const	CString	CString::mLinefeed(OSSTR("\r"));
const	CString	CString::mNewlineLinefeed(OSSTR("\n\r"));

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS)
	const	CString	CString::mPlatformDefaultNewline(OSSTR("\n"));
#elif defined(TARGET_OS_LINUX)
	const	CString	CString::mPlatformDefaultNewline(OSSTR("\n"));
#elif defined(TARGET_OS_WINDOWS)
	const	CString	CString::mPlatformDefaultNewline(OSSTR("\r\n"));
#endif

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CString::CString(UInt64 value, SpecialFormattingOptions options) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Init
	init();

	if (options & kSpecialFormattingOptionsBytesDecimal) {
		// Bytes - Decimal
		if (options & kSpecialFormattingOptionsBytesDecimalDoEasyRead) {
			if (value > kDisplayAsGBThreshHold)
				*this = CString((Float64) value / (1000.0 * 1000.0 * 1000.0), 0, 2) + CString(OSSTR("GB"));
			else if (value > kDisplayAsMBThreshHold)
				*this = CString((Float64) value / (1000.0 * 1000.0), 0, 2) + CString(OSSTR("MB"));
			else if (value > kDisplayAsKBThreshhold)
				*this = CString((Float64) value / 1000.0, 0, 2) + CString(OSSTR("KB"));
			else {
				if (options & kSpecialFormattingOptionsBytesDecimalDoOrAddExact)
					*this = CString::mEmpty;
				else
					*this = CString(value) + CString(OSSTR(" bytes"));
			}
		}

		if (options & kSpecialFormattingOptionsBytesDecimalDoOrAddExact) {
			// Display exact byte count
			if ((options & kSpecialFormattingOptionsBytesDecimalDoEasyRead) && (value > kDisplayAsKBThreshhold))
				*this += CString(OSSTR(" ("));

			UInt64	valueCopy = value;
			bool	displayZeros = false;
			if (valueCopy > 1000000000000LL) {
				*this += CString(valueCopy / 1000000000000LL, 3);
				if (options & kSpecialFormattingOptionsBytesDecimalDoOrAddExactUseCommas)
					*this += CString(OSSTR(","));
				valueCopy %= 1000000000000LL;
				displayZeros = true;
			}

			if (valueCopy > 1000000000) {
				*this += CString(valueCopy / 1000000000, displayZeros ? 3 : 0, displayZeros);
				if (options & kSpecialFormattingOptionsBytesDecimalDoOrAddExactUseCommas)
					*this += CString(OSSTR(","));
				valueCopy %= 1000000000;
				displayZeros = true;
			} else if (displayZeros) {
				*this += CString(OSSTR("000"));
				if (options & kSpecialFormattingOptionsBytesDecimalDoOrAddExactUseCommas)
					*this += CString(OSSTR(","));
			}

			if (valueCopy > 1000000) {
				*this += CString(valueCopy / 1000000, displayZeros ? 3 : 0, displayZeros);
				if (options & kSpecialFormattingOptionsBytesDecimalDoOrAddExactUseCommas)
					*this += CString(OSSTR(","));
				valueCopy %= 1000000;
				displayZeros = true;
			} else if (displayZeros) {
				*this += CString(OSSTR("000"));
				if (options & kSpecialFormattingOptionsBytesDecimalDoOrAddExactUseCommas)
					*this += CString(OSSTR(","));
			}

			if (valueCopy > 1000) {
				*this += CString(valueCopy / 1000, displayZeros ? 3 : 0, displayZeros);
				if (options & kSpecialFormattingOptionsBytesDecimalDoOrAddExactUseCommas)
					*this += CString(OSSTR(","));
				valueCopy %= 1000;
				displayZeros = true;
			} else if (displayZeros) {
				*this += CString(OSSTR("000"));
				if (options & kSpecialFormattingOptionsBytesDecimalDoOrAddExactUseCommas)
					*this += CString(OSSTR(","));
			}

			*this += CString(valueCopy, displayZeros ? 3 : 0, displayZeros);
			if (options & kSpecialFormattingOptionsBytesDecimalDoOrAddExactAddLabel)
				*this += CString(OSSTR(" bytes"));

			if ((options & kSpecialFormattingOptionsBytesDecimalDoEasyRead) && (value > kDisplayAsKBThreshhold))
				*this += CString(OSSTR(")"));
		}
	} else if (options & kSpecialFormattingOptionsBytesBinary) {
		// Bytes - Binary
		if (options & kSpecialFormattingOptionsBytesBinaryDoEasyRead) {
			if (value > kDisplayAsGiBThreshHold)
				*this = CString((Float64) value / (1024.0 * 1024.0 * 1024.0), 0, 2) + CString(OSSTR("GiB"));
			else if (value > kDisplayAsMiBThreshHold)
				*this = CString((Float64) value / (1024.0 * 1024.0), 0, 2) + CString(OSSTR("MiB"));
			else if (value > kDisplayAsKiBThreshhold)
				*this = CString((Float64) value / 1024.0, 0, 2) + CString(OSSTR("KiB"));
			else {
				if (options & kSpecialFormattingOptionsBytesBinaryDoOrAddExact)
					*this = CString::mEmpty;
				else
					*this = CString(value) + CString(OSSTR(" bytes"));
			}
		}

		if (options & kSpecialFormattingOptionsBytesBinaryDoOrAddExact) {
			// Display exact byte count
			if ((options & kSpecialFormattingOptionsBytesBinaryDoEasyRead) && (value > kDisplayAsKiBThreshhold))
				*this += CString(OSSTR(" ("));

			UInt64	valueCopy = value;
			bool	displayZeros = false;
			if (valueCopy > 1000000000000LL) {
				*this += CString(valueCopy / 1000000000000LL, 3);
				if (options & kSpecialFormattingOptionsBytesBinaryDoOrAddExactUseCommas)
					*this += CString(OSSTR(","));
				valueCopy %= 1000000000000LL;
				displayZeros = true;
			}

			if (valueCopy > 1000000000) {
				*this += CString(valueCopy / 1000000000, displayZeros ? 3 : 0, displayZeros);
				if (options & kSpecialFormattingOptionsBytesBinaryDoOrAddExactUseCommas)
					*this += CString(OSSTR(","));
				valueCopy %= 1000000000;
				displayZeros = true;
			} else if (displayZeros) {
				*this += CString(OSSTR("000"));
				if (options & kSpecialFormattingOptionsBytesBinaryDoOrAddExactUseCommas)
					*this += CString(OSSTR(","));
			}

			if (valueCopy > 1000000) {
				*this += CString(valueCopy / 1000000, displayZeros ? 3 : 0, displayZeros);
				if (options & kSpecialFormattingOptionsBytesBinaryDoOrAddExactUseCommas)
					*this += CString(OSSTR(","));
				valueCopy %= 1000000;
				displayZeros = true;
			} else if (displayZeros) {
				*this += CString(OSSTR("000"));
				if (options & kSpecialFormattingOptionsBytesBinaryDoOrAddExactUseCommas)
					*this += CString(OSSTR(","));
			}

			if (valueCopy > 1000) {
				*this += CString(valueCopy / 1000, displayZeros ? 3 : 0, displayZeros);
				if (options & kSpecialFormattingOptionsBytesBinaryDoOrAddExactUseCommas)
					*this += CString(OSSTR(","));
				valueCopy %= 1000;
				displayZeros = true;
			} else if (displayZeros) {
				*this += CString(OSSTR("000"));
				if (options & kSpecialFormattingOptionsBytesBinaryDoOrAddExactUseCommas)
					*this += CString(OSSTR(","));
			}

			*this += CString(valueCopy, displayZeros ? 3 : 0, displayZeros);
			if (options & kSpecialFormattingOptionsBytesBinaryDoOrAddExactAddLabel)
				*this += CString(OSSTR(" bytes"));

			if ((options & kSpecialFormattingOptionsBytesBinaryDoEasyRead) && (value > kDisplayAsKiBThreshhold))
				*this += CString(OSSTR(")"));
		}
	} else if (options & kSpecialFormattingOptionsCurrencyDollars) {
		// Currency - Dollars
		if (options & kSpecialFormattingOptionsCurrencyDollarsAddDollarsign)
			*this += CString(OSSTR("$"));

		if (options & kSpecialFormattingOptionsCurrencyDollarsUseCommas) {
			// Use commas
			UInt64	dollars = value / 100, temp = 10;
			UInt32	digitsCount = 1;
			for (temp = 10; temp <= dollars; temp *= 10)
				digitsCount++;

			temp /= 10;
			do {
				UInt64	digit = dollars / temp;
				dollars -= digit * temp;
				temp /= 10;

				*this += CString(digit);
				if ((digitsCount > 1) && (((digitsCount - 1) % 3) == 0))
					*this += CString(OSSTR(","));

				digitsCount--;
			} while (digitsCount > 0);
		} else
			*this += CString(value / 100);

		if (options & kSpecialFormattingOptionsCurrencyDollarsAddCents)
			*this += CString(OSSTR(".")) + CString(value % 100, 2, true);
	}
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
SInt8 CString::getSInt8(UInt8 base) const
//----------------------------------------------------------------------------------------------------------------------
{
	return (SInt8) ::strtol(*getCString(), nil, base);
}

//----------------------------------------------------------------------------------------------------------------------
SInt16 CString::getSInt16(UInt8 base) const
//----------------------------------------------------------------------------------------------------------------------
{
	return (SInt16) ::strtol(*getCString(), nil, base);
}

//----------------------------------------------------------------------------------------------------------------------
SInt32 CString::getSInt32(UInt8 base) const
//----------------------------------------------------------------------------------------------------------------------
{
	return (SInt32) ::strtol(*getCString(), nil, base);
}

//----------------------------------------------------------------------------------------------------------------------
SInt64 CString::getSInt64(UInt8 base) const
//----------------------------------------------------------------------------------------------------------------------
{
	return ::strtoll(*getCString(), nil, base);
}

//----------------------------------------------------------------------------------------------------------------------
UInt8 CString::getUInt8(UInt8 base) const
//----------------------------------------------------------------------------------------------------------------------
{
	return (UInt8) ::strtoul(*getCString(), nil, base);
}

//----------------------------------------------------------------------------------------------------------------------
UInt16 CString::getUInt16(UInt8 base) const
//----------------------------------------------------------------------------------------------------------------------
{
	return (UInt16) ::strtoul(*getCString(), nil, base);
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CString::getUInt32(UInt8 base) const
//----------------------------------------------------------------------------------------------------------------------
{
	return (UInt32) ::strtoul(*getCString(), nil, base);
}

//----------------------------------------------------------------------------------------------------------------------
OSType CString::getOSType() const
//----------------------------------------------------------------------------------------------------------------------
{
	return EndianU32_BtoN(*((UInt32*) *getCString()));
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CString::getUInt64(UInt8 base) const
//----------------------------------------------------------------------------------------------------------------------
{
	return ::strtoull(*getCString(), nil, base);
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CString::getAsByteCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	CString	string = uppercased();

	if (string.contains(CString(OSSTR("GIB")), kCompareFlagsCaseInsensitive))
		return (UInt64) (getFloat64() * 1024.0 * 1024.0 * 1024.0);
	if (string.contains(CString(OSSTR("GB")), kCompareFlagsCaseInsensitive))
		return (UInt64) (getFloat64() * 1000.0 * 1000.0 * 1000.0);
	else if (string.contains(CString(OSSTR("MIB")), kCompareFlagsCaseInsensitive))
		return (UInt64) (getFloat64() * 1024.0 * 1024.0);
	else if (string.contains(CString(OSSTR("MB")), kCompareFlagsCaseInsensitive))
		return (UInt64) (getFloat64() * 1000.0 * 1000.0);
	else if (string.contains(CString(OSSTR("KIB")), kCompareFlagsCaseInsensitive))
		return (UInt64) (getFloat64() * 1024.0);
	else if (string.contains(CString(OSSTR("KB")), kCompareFlagsCaseInsensitive))
		return (UInt64) (getFloat64() * 1000.0);
	else
		return getUInt64();
}

//----------------------------------------------------------------------------------------------------------------------
bool CString::isValidEmailAddress() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Must have at least one "@"
	TArray<CString>	array1 = components(CString(OSSTR("@")));
	bool			isValid = array1.getCount() >= 2;
	if (isValid) {
		// Before the last "@", can be pretty much anything.
		TArray<CString>	array2 = array1[array1.getCount() - 1].components(CString(OSSTR(".")));

		// After the last "@", must have at least one "."
		isValid = array2.getCount() >= 2;

		// Each segment separated by "." must have at least one character
		for (CArray::ItemIndex i = 0; isValid && (i < array2.getCount()); i++)
			isValid = !array2[i].isEmpty();

		if (isValid) {
			// The last segment must be an established domain or 2 letter country code
			CString	string = array2.getLast();
			isValid =
					(string == CString(OSSTR("aero"))) ||
					(string == CString(OSSTR("biz"))) ||
					(string == CString(OSSTR("com"))) ||
					(string == CString(OSSTR("edu"))) ||
					(string == CString(OSSTR("gov"))) ||
					(string == CString(OSSTR("jobs"))) ||
					(string == CString(OSSTR("info"))) ||
					(string == CString(OSSTR("mil"))) ||
					(string == CString(OSSTR("mobi"))) ||
					(string == CString(OSSTR("museum"))) ||
					(string == CString(OSSTR("name"))) ||
					(string == CString(OSSTR("net"))) ||
					(string == CString(OSSTR("org"))) ||
					(string.getLength() == 2);
		}
	}

	return isValid;
}

//----------------------------------------------------------------------------------------------------------------------
TArray<CString> CString::componentsRespectingQuotes(const CString& separator) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNArray<CString>	array;

	// Quotes around a string with the separator is not a real separator
	bool	inQuotes = false;
	CString	tempString;
	for (CharIndex i = 0; i < getLength();) {
		if (getCharacterAtIndex(i) == '\"') {
			// Found quote
			inQuotes = !inQuotes;
			i++;
		} else if (!inQuotes && (getSubString(i).hasPrefix(separator))) {
			// Found separator
			array += tempString;
			tempString = mEmpty;
			i += separator.getLength();
		} else {
			// Found another character
			tempString += getSubString(i, 1);
			i++;
		}
	}
		
	if (!tempString.isEmpty())
		array += tempString;
	
	return array;
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
bool CString::compare(CString* const string1, CString* const string2, void* compareFlags)
//----------------------------------------------------------------------------------------------------------------------
{
	return string1->compareTo(*string2, *((CompareFlags*) compareFlags));
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CString CString::make(OSStringType format, ...)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	va_list	args;
	va_start(args, format);

	// Make
	CString	string = make(format, args);

	// Cleanup
	va_end(args);

	return string;
}
