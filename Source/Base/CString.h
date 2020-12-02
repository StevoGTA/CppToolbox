//----------------------------------------------------------------------------------------------------------------------
//	CString.h			©2003 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CArray.h"
#include "CHashing.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Native strings
#if TARGET_OS_IOS || TARGET_OS_MACOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
	#define OSStringType	CFStringRef
	#define OSStringVar(s)	CFStringRef s
	#define	OSSTR(s)		CFSTR(s)
#elif TARGET_OS_WINDOWS
	#define OSStringType	const TCHAR*
	#define OSStringVar(s)	TCHAR s[]
	#define OSSTR(s)		_TEXT(s)
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Item count and Range

typedef	UInt32	CStringCharIndex;
typedef	UInt32	CStringLength;

struct SStringRange {
	// Lifecycle methods
	SStringRange(CStringCharIndex start, CStringLength length) : mStart(start), mLength(length) {}

	// Properties
	CStringCharIndex	mStart;
	CStringLength		mLength;	// 0 if not found
};

const	CStringLength	kCStringDefaultMaxLength = 0xffffffff;

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Comparison

enum EStringCompareFlags {
	kStringCompareFlagsNone				= 0,
	kStringCompareFlagsCaseInsensitive	= 1 << 0,	// FOO.TXT == foo.txt
	kStringCompareFlagsBackwards		= 1 << 1,	// Starting from the end of the string
	kStringCompareFlagsNonliteral		= 1 << 2,	// Loose equivalence is performed (o-umlaut == o, umlaut)
	kStringCompareFlagsLocalized		= 1 << 3,	// User's default locale is used for the comparison
	kStringCompareFlagsNumerically		= 1 << 4,	// Numeric comparison; i.e. Foo2.txt < Foo7.txt < Foo25.txt
	
	kStringCompareFlagsDefault =
			kStringCompareFlagsNonliteral |
			kStringCompareFlagsLocalized,
	kStringCompareFlagsSortDefault =
			kStringCompareFlagsCaseInsensitive |
			kStringCompareFlagsNonliteral |
			kStringCompareFlagsLocalized |
			kStringCompareFlagsNumerically,
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Encoding

enum EStringEncoding {
	kStringEncodingASCII,		// 0..127 (values greater than 127 are treated as corresponding Unicode value)
	kStringEncodingMacRoman,
	kStringEncodingUTF8,
	kStringEncodingISOLatin,	// ISO 8859-1
	kStringEncodingUnicode,

	kStringEncodingUTF16,		// UTF 16 w/ BOM
	kStringEncodingUTF16BE,		// UTF 16 w/o BOM and explicit BE order
	kStringEncodingUTF16LE,		// UTF 16 w/o BOM and explicit LE order
	kStringEncodingUTF32,		// UTF 32 w/ BOM
	kStringEncodingUTF32BE,		// UTF 32 w/o BOM and explicit BE order
	kStringEncodingUTF32LE,		// UTF 32 w/o BOM and explicit LE order

	kStringEncodingTextDefault = kStringEncodingUTF8,

#if TARGET_RT_BIG_ENDIAN
	kStringEncodingUTF16Native = kStringEncodingUTF16BE,
	kStringEncodingUTF32Native = kStringEncodingUTF32BE,
#else
	kStringEncodingUTF16Native = kStringEncodingUTF16LE,
	kStringEncodingUTF32Native = kStringEncodingUTF32LE,
#endif
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Character Sets

enum EStringCharacterSet {
	kStringCharacterSetControl,					// Control character set (Unicode General Category Cc and Cf
	kStringCharacterSetWhitespace,				// Whitespace character set (Unicode General Category Zs and U0009 CHARACTER TABULATION
	kStringCharacterSetWhitespaceAndNewline,	// Whitespace and Newline character set (Unicode General Category Z*, U000A ~ U000D, and U0085)
	kStringCharacterSetDecimalDigit,			// Decimal digit character set
	kStringCharacterSetLetter,					// Letter character set (Unicode General Category L* & M*)
	kStringCharacterSetLowercaseLetter,			// Lowercase character set (Unicode General Category Ll)
	kStringCharacterSetUppercaseLetter,			// Uppercase character set (Unicode General Category Lu and Lt)
	kStringCharacterSetNonBase,					// Non-base character set (Unicode General Category M*)
	kStringCharacterSetDecomposable,			// Canonically decomposable character set
	kStringCharacterSetAlphaNumeric,			// Alpha Numeric character set (Unicode General Category L*, M*, & N*)
	kStringCharacterSetPunctuation,				// Punctuation character set (Unicode General Category P*)
	kStringCharacterSetIllegal,					// Illegal character set
	kStringCharacterSetCapitalizedLetter,		// Titlecase character set (Unicode General Category Lt)
	kStringCharacterSetSymbol,					// Symbol character set (Unicode General Category S*)
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Display Stuff

enum EStringSpecialFormattingOptions {
	// Bytes - Decimal
	kStringSpecialFormattingOptionsBytesDecimal							= 1 << 0,
	kStringSpecialFormattingOptionsBytesDecimalDoEasyRead				= 1 << 1,
	kStringSpecialFormattingOptionsBytesDecimalDoOrAddExact				= 1 << 2,
	kStringSpecialFormattingOptionsBytesDecimalDoOrAddExactUseCommas	= 1 << 3,
	kStringSpecialFormattingOptionsBytesDecimalDoOrAddExactAddLabel		= 1 << 4,
	
	kStringSpecialFormattingOptionsBytesDecimalDefault =
			kStringSpecialFormattingOptionsBytesDecimal |
			kStringSpecialFormattingOptionsBytesDecimalDoEasyRead |
			kStringSpecialFormattingOptionsBytesDecimalDoOrAddExact |
			kStringSpecialFormattingOptionsBytesDecimalDoOrAddExactUseCommas |
			kStringSpecialFormattingOptionsBytesDecimalDoOrAddExactAddLabel,

	kStringSpecialFormattingOptionsBytesDecimalMask =
			kStringSpecialFormattingOptionsBytesDecimal |
			kStringSpecialFormattingOptionsBytesDecimalDoEasyRead |
			kStringSpecialFormattingOptionsBytesDecimalDoOrAddExact |
			kStringSpecialFormattingOptionsBytesDecimalDoOrAddExactUseCommas |
			kStringSpecialFormattingOptionsBytesDecimalDoOrAddExactAddLabel,

	// Bytes - Binary
	kStringSpecialFormattingOptionsBytesBinary							= 1 << 5,
	kStringSpecialFormattingOptionsBytesBinaryDoEasyRead				= 1 << 6,
	kStringSpecialFormattingOptionsBytesBinaryDoOrAddExact				= 1 << 7,
	kStringSpecialFormattingOptionsBytesBinaryDoOrAddExactUseCommas		= 1 << 8,
	kStringSpecialFormattingOptionsBytesBinaryDoOrAddExactAddLabel		= 1 << 9,
	
	kStringSpecialFormattingOptionsBytesBinaryDefault =
			kStringSpecialFormattingOptionsBytesBinary |
			kStringSpecialFormattingOptionsBytesBinaryDoEasyRead |
			kStringSpecialFormattingOptionsBytesBinaryDoOrAddExact |
			kStringSpecialFormattingOptionsBytesBinaryDoOrAddExactUseCommas |
			kStringSpecialFormattingOptionsBytesBinaryDoOrAddExactAddLabel,

	kStringSpecialFormattingOptionsBytesBinaryMask =
			kStringSpecialFormattingOptionsBytesBinary |
			kStringSpecialFormattingOptionsBytesBinaryDoEasyRead |
			kStringSpecialFormattingOptionsBytesBinaryDoOrAddExact |
			kStringSpecialFormattingOptionsBytesBinaryDoOrAddExactUseCommas |
			kStringSpecialFormattingOptionsBytesBinaryDoOrAddExactAddLabel,

	// Currency - Dollars (value is cents)
	kStringSpecialFormattingOptionsCurrencyDollars						= 1 << 10,
	kStringSpecialFormattingOptionsCurrencyDollarsAddDollarsign			= 1 << 11,
	kStringSpecialFormattingOptionsCurrencyDollarsUseCommas				= 1 << 12,
	kStringSpecialFormattingOptionsCurrencyDollarsAddCents				= 1 << 13,

	kStringSpecialFormattingOptionsCurrencyDollarsDefault =
			kStringSpecialFormattingOptionsCurrencyDollars |
			kStringSpecialFormattingOptionsCurrencyDollarsAddDollarsign |
			kStringSpecialFormattingOptionsCurrencyDollarsUseCommas |
			kStringSpecialFormattingOptionsCurrencyDollarsAddCents,

	kStringSpecialFormattingOptionsCurrencyDollarsMask =
			kStringSpecialFormattingOptionsCurrencyDollars |
			kStringSpecialFormattingOptionsCurrencyDollarsAddDollarsign |
			kStringSpecialFormattingOptionsCurrencyDollarsUseCommas |
			kStringSpecialFormattingOptionsCurrencyDollarsAddCents,
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SCString
struct SCString : SReferenceCountable {
					// Lifecycle methods
					SCString(CStringLength length) : SReferenceCountable()
						{
							mBuffer = new char[std::max<CStringLength>(length, 1)];
							mBuffer[0] = 0;
						}
					SCString(const SCString& other) : SReferenceCountable(other), mBuffer(other.mBuffer) {}
					~SCString()
						{
							// Check if last reference
							if (removeReference() == 0)
								// Last reference
								DeleteArray(mBuffer);
						}

					// Instance methods
	const	char*	operator*() const
						{ return mBuffer; }

			void	hashInto(CHasher& hasher) const
						{ hasher.add(mBuffer); }

	// Properties
	char*	mBuffer;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CString

class CData;

class CString : public CHashable {
	// Methods
	public:
										// Lifecycle methods
										CString();
										CString(const CString& other, OV<CStringLength> length = OV<CStringLength>());
										CString(const OSStringVar(initialString),
												OV<CStringLength> length = OV<CStringLength>());
										CString(const char* chars, CStringLength charsCount = kCStringDefaultMaxLength,
												EStringEncoding encoding = kStringEncodingTextDefault);
										CString(const UTF16Char* chars, CStringLength charsCount,
												EStringEncoding encoding = kStringEncodingUTF16Native);
										CString(const UTF32Char* chars, CStringLength charsCount,
												EStringEncoding encoding = kStringEncodingUTF32Native);

										CString(Float32 value, UInt32 fieldSize, UInt32 digitsAfterDecimalPoint,
												bool padWithZeros = false);
										CString(Float64 value, UInt32 fieldSize, UInt32 digitsAfterDecimalPoint,
												bool padWithZeros = false);
										CString(SInt8 value, UInt32 fieldSize = 0, bool padWithZeros = false);
										CString(SInt16 value, UInt32 fieldSize = 0, bool padWithZeros = false);
										CString(SInt32 value, UInt32 fieldSize = 0, bool padWithZeros = false);
										CString(SInt64 value, UInt32 fieldSize = 0, bool padWithZeros = false);
										CString(UInt8 value, UInt32 fieldSize = 0, bool padWithZeros = false,
												bool makeHex = false);
										CString(UInt16 value, UInt32 fieldSize = 0, bool padWithZeros = false,
												bool makeHex = false);
										CString(UInt32 value, UInt32 fieldSize = 0, bool padWithZeros = false,
												bool makeHex = false);
										CString(UInt64 value, UInt32 fieldSize = 0, bool padWithZeros = false,
												bool makeHex = false);
										CString(OSType osType, bool isOSType, bool includeQuotes = true);
										CString(UInt64 value, EStringSpecialFormattingOptions options);
										CString(const void* pointer);

										CString(const CData& data,
												EStringEncoding encoding = kStringEncodingTextDefault);

										~CString();

										// CEquatable methods
						bool			operator==(const CEquatable& other) const
											{ return equals((const CString&) other); }

										// CHashable methods
						void			hashInto(CHasher& hasher) const
											{ getCString().hashInto(hasher); }

										// Instance methods
						OSStringType	getOSString() const;
				const	SCString		getCString(EStringEncoding encoding = kStringEncodingTextDefault) const;
						CStringLength	getLength() const;
						CStringLength	getLength(EStringEncoding encoding, SInt8 lossCharacter = '\0',
												bool forExternalStorageOrTransmission = false) const;
						bool			isEmpty() const
											{ return getLength() == 0; }

						CStringLength	get(char* buffer, CStringLength bufferLen, bool addNull = true,
												EStringEncoding encoding = kStringEncodingTextDefault) const;
						CStringLength	get(UTF16Char* buffer, CStringLength bufferLen,
												EStringEncoding encoding = kStringEncodingUTF16Native) const;

						UTF32Char		getCharacterAtIndex(CStringCharIndex index) const;

						Float32			getFloat32() const;
						void			getValue(Float32& value) const
											{ value = getFloat32(); }
						Float64			getFloat64() const;
						void			getValue(Float64& value) const
											{ value = getFloat64(); }
						SInt8			getSInt8(UInt8 base = 10) const;
						void			getValue(SInt8& value, UInt8 base = 10) const
											{ value = getSInt8(base); }
						SInt16			getSInt16(UInt8 base = 10) const;
						void			getValue(SInt16& value, UInt8 base = 10) const
											{ value = getSInt16(base); }
						SInt32			getSInt32(UInt8 base = 10) const;
						void			getValue(SInt32& value, UInt8 base = 10) const
											{ value = getSInt32(base); }
						SInt64			getSInt64(UInt8 base = 10) const;
						void			getValue(SInt64& value, UInt8 base = 10) const
											{ value = getSInt64(base); }
						UInt8			getUInt8(UInt8 base = 10) const;
						void			getValue(UInt8& value, UInt8 base = 10) const
											{ value = getUInt8(base); }
						UInt16			getUInt16(UInt8 base = 10) const;
						void			getValue(UInt16& value, UInt8 base = 10) const
											{ value = getUInt16(base); }
						UInt32			getUInt32(UInt8 base = 10) const;
						void			getValue(UInt32& value, UInt8 base = 10) const
											{ value = getUInt32(base); }
						OSType			getOSType() const;
						UInt64			getUInt64(UInt8 base = 10) const;
						void			getValue(UInt64& value, UInt8 base = 10) const
											{ value = getUInt64(base); }
						UInt64			getAsByteCount() const;
						
						CData			getData(EStringEncoding encoding = kStringEncodingTextDefault,
												SInt8 lossCharacter = '\0') const;
						
						CString			getSubString(CStringCharIndex startIndex,
												CStringLength charCount = kCStringDefaultMaxLength) const;
						CString			replacingSubStrings(const CString& subStringToReplace,
												const CString& replacementString = CString::mEmpty) const;
						CString			replacingCharacters(CStringCharIndex startIndex = 0,
												CStringLength charCount = kCStringDefaultMaxLength,
												const CString& replacementString = CString::mEmpty) const;

						SStringRange	findSubString(const CString& subString, CStringCharIndex startIndex = 0,
												CStringLength charCount = kCStringDefaultMaxLength) const;
						
						CString			lowercased() const;
						CString			uppercased() const;
						CString			removingLeadingAndTrailingWhitespace() const;
						CString			removingAllWhitespace() const;
						CString			removingLeadingAndTrailingQuotes() const;

						bool			isValidEmailAddress() const;
						CString			getCommonPrefix(const CString& other) const;
					
						TArray<CString>	breakUp(const CString& delimiterString) const;
						TArray<CString>	breakUpRespectingQuotes(const CString& delimiterString) const;

						ECompareResult	compareTo(const CString& other,
												EStringCompareFlags flags = kStringCompareFlagsDefault) const;
						bool			equals(const CString& other,
												EStringCompareFlags flags = kStringCompareFlagsDefault) const;
						bool			hasPrefix(const CString& other) const;
						bool			hasSuffix(const CString& other) const;
						bool			contains(const CString& other,
												EStringCompareFlags flags = kStringCompareFlagsDefault) const;
												
						CString&		operator=(const CString& other);
						CString&		operator+=(const CString& other);
						CString			operator+(const CString& other) const;
												
										// Class methods
		static			ECompareResult	compare(CString* const string1, CString* const string2, void* compareFlags);

		static			CString			make(const char* format, ...);
		static			CString			make(const char* format, va_list args);
		static			bool			isCharacterInSet(UTF32Char utf32Char, EStringCharacterSet characterSet);

	protected:
										// Internal methods
						void			init();

	// Properties
	public:
		static			CString		mEmpty;

		static			CString		mComma;
		static			CString		mPeriod;
		static			CString		mSpace;
		static			CString		mSpaceX4;
		static			CString		mTab;

		static			CString		mNewline;
		static			CString		mLinefeed;
		static			CString		mNewlineLinefeed;
		static			CString		mPlatformDefaultNewline;
		
	private:
#if TARGET_OS_IOS || TARGET_OS_MACOS || TARGET_OS_TVOS || TARGET_OS_WATCHOS
						CFStringRef					mStringRef;
#elif TARGET_OS_WINDOWS
						std::basic_string<TCHAR>	mString;
#endif
};
