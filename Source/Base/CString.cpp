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

CString	CString::mEmpty;

CString	CString::mCommaCharacter(",");
CString	CString::mSpaceCharacter(" ");
CString	CString::mTabCharacter("\t");

CString	CString::sNewlineCharacter("\n");
CString	CString::sLinefeedCharacter("\r");
CString	CString::sNewlineCharacters("\n\r");

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CString

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CString::CString(UInt64 value, EStringSpecialFormattingOptions options) : CHashable()
//----------------------------------------------------------------------------------------------------------------------
{
	// Init
	init();

	if (options & kStringSpecialFormattingOptionsBytesDecimal) {
		// Bytes - Decimal
		if (options & kStringSpecialFormattingOptionsBytesDecimalDoEasyRead) {
			if (value > kDisplayAsGBThreshHold)
				*this = CString((Float64) value / (1000.0 * 1000.0 * 1000.0), 0, 2) + CString("GB");
			else if (value > kDisplayAsMBThreshHold)
				*this = CString((Float64) value / (1000.0 * 1000.0), 0, 2) + CString("MB");
			else if (value > kDisplayAsKBThreshhold)
				*this = CString((Float64) value / 1000.0, 0, 2) + CString("KB");
			else {
				if (options & kStringSpecialFormattingOptionsBytesDecimalDoOrAddExact)
					replaceCharacters();
				else
					*this = CString(value) + CString(" bytes");
			}
		}

		if (options & kStringSpecialFormattingOptionsBytesDecimalDoOrAddExact) {
			// Display exact byte count
			if ((options & kStringSpecialFormattingOptionsBytesDecimalDoEasyRead) && (value > kDisplayAsKBThreshhold))
				*this += CString(" (");

			UInt64	valueCopy = value;
			bool	displayZeros = false;
			if (valueCopy > 1000000000000LL) {
				*this += CString(valueCopy / 1000000000000LL, 3);
				if (options & kStringSpecialFormattingOptionsBytesDecimalDoOrAddExactUseCommas)
					*this += CString(",");
				valueCopy %= 1000000000000LL;
				displayZeros = true;
			}

			if (valueCopy > 1000000000) {
				*this += CString(valueCopy / 1000000000, displayZeros ? 3 : 0, displayZeros);
				if (options & kStringSpecialFormattingOptionsBytesDecimalDoOrAddExactUseCommas)
					*this += CString(",");
				valueCopy %= 1000000000;
				displayZeros = true;
			} else if (displayZeros) {
				*this += CString("000");
				if (options & kStringSpecialFormattingOptionsBytesDecimalDoOrAddExactUseCommas)
					*this += CString(",");
			}

			if (valueCopy > 1000000) {
				*this += CString(valueCopy / 1000000, displayZeros ? 3 : 0, displayZeros);
				if (options & kStringSpecialFormattingOptionsBytesDecimalDoOrAddExactUseCommas)
					*this += CString(",");
				valueCopy %= 1000000;
				displayZeros = true;
			} else if (displayZeros) {
				*this += CString("000");
				if (options & kStringSpecialFormattingOptionsBytesDecimalDoOrAddExactUseCommas)
					*this += CString(",");
			}

			if (valueCopy > 1000) {
				*this += CString(valueCopy / 1000, displayZeros ? 3 : 0, displayZeros);
				if (options & kStringSpecialFormattingOptionsBytesDecimalDoOrAddExactUseCommas)
					*this += CString(",");
				valueCopy %= 1000;
				displayZeros = true;
			} else if (displayZeros) {
				*this += CString("000");
				if (options & kStringSpecialFormattingOptionsBytesDecimalDoOrAddExactUseCommas)
					*this += CString(",");
			}

			*this += CString(valueCopy, displayZeros ? 3 : 0, displayZeros);
			if (options & kStringSpecialFormattingOptionsBytesDecimalDoOrAddExactAddLabel)
				*this += CString(" bytes");

			if ((options & kStringSpecialFormattingOptionsBytesDecimalDoEasyRead) && (value > kDisplayAsKBThreshhold))
				*this += CString(")");
		}
	} else if (options & kStringSpecialFormattingOptionsBytesBinary) {
		// Bytes - Binary
		if (options & kStringSpecialFormattingOptionsBytesBinaryDoEasyRead) {
			if (value > kDisplayAsGiBThreshHold)
				*this = CString((Float64) value / (1024.0 * 1024.0 * 1024.0), 0, 2) + CString("GiB");
			else if (value > kDisplayAsMiBThreshHold)
				*this = CString((Float64) value / (1024.0 * 1024.0), 0, 2) + CString("MiB");
			else if (value > kDisplayAsKiBThreshhold)
				*this = CString((Float64) value / 1024.0, 0, 2) + CString("KiB");
			else {
				if (options & kStringSpecialFormattingOptionsBytesBinaryDoOrAddExact)
					replaceCharacters();
				else
					*this = CString(value) + CString(" bytes");
			}
		}

		if (options & kStringSpecialFormattingOptionsBytesBinaryDoOrAddExact) {
			// Display exact byte count
			if ((options & kStringSpecialFormattingOptionsBytesBinaryDoEasyRead) && (value > kDisplayAsKiBThreshhold))
				*this += CString(" (");

			UInt64	valueCopy = value;
			bool	displayZeros = false;
			if (valueCopy > 1000000000000LL) {
				*this += CString(valueCopy / 1000000000000LL, 3);
				if (options & kStringSpecialFormattingOptionsBytesBinaryDoOrAddExactUseCommas)
					*this += CString(",");
				valueCopy %= 1000000000000LL;
				displayZeros = true;
			}

			if (valueCopy > 1000000000) {
				*this += CString(valueCopy / 1000000000, displayZeros ? 3 : 0, displayZeros);
				if (options & kStringSpecialFormattingOptionsBytesBinaryDoOrAddExactUseCommas)
					*this += CString(",");
				valueCopy %= 1000000000;
				displayZeros = true;
			} else if (displayZeros) {
				*this += CString("000");
				if (options & kStringSpecialFormattingOptionsBytesBinaryDoOrAddExactUseCommas)
					*this += CString(",");
			}

			if (valueCopy > 1000000) {
				*this += CString(valueCopy / 1000000, displayZeros ? 3 : 0, displayZeros);
				if (options & kStringSpecialFormattingOptionsBytesBinaryDoOrAddExactUseCommas)
					*this += CString(",");
				valueCopy %= 1000000;
				displayZeros = true;
			} else if (displayZeros) {
				*this += CString("000");
				if (options & kStringSpecialFormattingOptionsBytesBinaryDoOrAddExactUseCommas)
					*this += CString(",");
			}

			if (valueCopy > 1000) {
				*this += CString(valueCopy / 1000, displayZeros ? 3 : 0, displayZeros);
				if (options & kStringSpecialFormattingOptionsBytesBinaryDoOrAddExactUseCommas)
					*this += CString(",");
				valueCopy %= 1000;
				displayZeros = true;
			} else if (displayZeros) {
				*this += CString("000");
				if (options & kStringSpecialFormattingOptionsBytesBinaryDoOrAddExactUseCommas)
					*this += CString(",");
			}

			*this += CString(valueCopy, displayZeros ? 3 : 0, displayZeros);
			if (options & kStringSpecialFormattingOptionsBytesBinaryDoOrAddExactAddLabel)
				*this += CString(" bytes");

			if ((options & kStringSpecialFormattingOptionsBytesBinaryDoEasyRead) && (value > kDisplayAsKiBThreshhold))
				*this += CString(")");
		}
	} else if (options & kStringSpecialFormattingOptionsCurrencyDollars) {
		// Currency - Dollars
		if (options & kStringSpecialFormattingOptionsCurrencyDollarsAddDollarsign)
			*this += CString("$");

		if (options & kStringSpecialFormattingOptionsCurrencyDollarsUseCommas) {
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
					*this += CString(",");

				digitsCount--;
			} while (digitsCount > 0);
		} else
			*this += CString(value / 100);

		if (options & kStringSpecialFormattingOptionsCurrencyDollarsAddCents)
			*this += CString(".") + CString(value % 100, 2, true);
	}
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
UInt64 CString::getAsBytes() const
//----------------------------------------------------------------------------------------------------------------------
{
	CString	string(*this);
	string.makeUppercase();

	if (string.contains(CString("GIB"), kStringCompareFlagsCaseInsensitive))
		return (UInt64) (getFloat64() * 1024.0 * 1024.0 * 1024.0);
	if (string.contains(CString("GB"), kStringCompareFlagsCaseInsensitive))
		return (UInt64) (getFloat64() * 1000.0 * 1000.0 * 1000.0);
	else if (string.contains(CString("MIB"), kStringCompareFlagsCaseInsensitive))
		return (UInt64) (getFloat64() * 1024.0 * 1024.0);
	else if (string.contains(CString("MB"), kStringCompareFlagsCaseInsensitive))
		return (UInt64) (getFloat64() * 1000.0 * 1000.0);
	else if (string.contains(CString("KIB"), kStringCompareFlagsCaseInsensitive))
		return (UInt64) (getFloat64() * 1024.0);
	else if (string.contains(CString("KB"), kStringCompareFlagsCaseInsensitive))
		return (UInt64) (getFloat64() * 1000.0);
	else
		return getUInt64();
}

//----------------------------------------------------------------------------------------------------------------------
bool CString::isValidEmailAddress() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Must have at least one "@"
	TArray<CString>	array1 = breakUp(CString("@"));
	bool			isValid = array1.getCount() >= 2;
	if (isValid) {
		// Before the last "@", can be pretty much anything.
		TArray<CString>	array2 = array1[array1.getCount() - 1].breakUp(CString("."));

		// After the last "@", must have at least one "."
		isValid = array2.getCount() >= 2;

		// Each segment separated by "." must have at least one character
		for (CArrayItemIndex i = 0; isValid && (i < array2.getCount()); i++)
			isValid = !array2[i].isEmpty();

		if (isValid) {
			// The last segment must be an established domain or 2 letter country code
			CString	string = array2.getLast();
			isValid =
					(string == CString("aero")) ||
					(string == CString("biz")) ||
					(string == CString("com")) ||
					(string == CString("edu")) ||
					(string == CString("gov")) ||
					(string == CString("jobs")) ||
					(string == CString("info")) ||
					(string == CString("mil")) ||
					(string == CString("mobi")) ||
					(string == CString("museum")) ||
					(string == CString("name")) ||
					(string == CString("net")) ||
					(string == CString("org")) ||
					(string.getLength() == 2);
		}
	}

	return isValid;
}

//----------------------------------------------------------------------------------------------------------------------
ECompareResult CString::compare(CString* const string1, CString* const string2, void* compareFlags)
//----------------------------------------------------------------------------------------------------------------------
{
	return string1->compareTo(*string2, *((EStringCompareFlags*) compareFlags));
}
