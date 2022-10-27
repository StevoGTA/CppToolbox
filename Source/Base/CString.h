//----------------------------------------------------------------------------------------------------------------------
//	CString.h			©2003 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CArray.h"
#include "CHashing.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Native strings

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS)
	#define OSStringType	CFStringRef
	#define OSStringVar(s)	CFStringRef s
	#define	OSSTR(s)		CFSTR(s)
#elif defined(TARGET_OS_WINDOWS)
	#define OSStringType	const TCHAR*
	#define OSStringVar(s)	const TCHAR s[]
	#define OSSTR(s)		_T(s)
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CString

class CData;
class CDictionary;

class CString : public CHashable {
	// Enums
	public:
		enum CompareFlags {
			kCompareFlagsNone				= 0,
			kCompareFlagsCaseInsensitive	= 1 << 0,	// FOO.TXT == foo.txt
			kCompareFlagsBackwards			= 1 << 1,	// Starting from the end of the string
			kCompareFlagsNonliteral			= 1 << 2,	// Loose equivalence is performed (o-umlaut == o, umlaut)
			kCompareFlagsLocalized			= 1 << 3,	// User's default locale is used for the comparison
			kCompareFlagsNumerically		= 1 << 4,	// Numeric comparison; i.e. Foo2.txt < Foo7.txt < Foo25.txt

			kCompareFlagsDefault =
					kCompareFlagsNonliteral |
					kCompareFlagsLocalized,
			kCompareFlagsSortDefault =
					kCompareFlagsCaseInsensitive |
					kCompareFlagsNonliteral |
					kCompareFlagsLocalized |
					kCompareFlagsNumerically,
		};

		enum Encoding {
			kEncodingASCII,		// 0..127 (values greater than 127 are treated as corresponding Unicode value)
			kEncodingMacRoman,
			kEncodingUTF8,
			kEncodingISOLatin,	// ISO 8859-1
			kEncodingUnicode,

			kEncodingUTF16,		// UTF 16 w/ BOM
			kEncodingUTF16BE,	// UTF 16 w/o BOM and explicit BE order
			kEncodingUTF16LE,	// UTF 16 w/o BOM and explicit LE order
			kEncodingUTF32,		// UTF 32 w/ BOM
			kEncodingUTF32BE,	// UTF 32 w/o BOM and explicit BE order
			kEncodingUTF32LE,	// UTF 32 w/o BOM and explicit LE order

			kEncodingTextDefault = kEncodingUTF8,

#if TARGET_RT_BIG_ENDIAN
			kEncodingUTF16Native = kEncodingUTF16BE,
			kEncodingUTF32Native = kEncodingUTF32BE,
#else
			kEncodingUTF16Native = kEncodingUTF16LE,
			kEncodingUTF32Native = kEncodingUTF32LE,
#endif
		};

		enum CharacterSet {
			kCharacterSetControl,				// Control character set (Unicode General Category Cc and Cf
			kCharacterSetWhitespace,			// Whitespace character set (Unicode General Category Zs and U0009 CHARACTER TABULATION
			kCharacterSetWhitespaceAndNewline,	// Whitespace and Newline character set (Unicode General Category Z*, U000A ~ U000D, and U0085)
			kCharacterSetDecimalDigit,			// Decimal digit character set
			kCharacterSetLetter,				// Letter character set (Unicode General Category L* & M*)
			kCharacterSetLowercaseLetter,		// Lowercase character set (Unicode General Category Ll)
			kCharacterSetUppercaseLetter,		// Uppercase character set (Unicode General Category Lu and Lt)
			kCharacterSetNonBase,				// Non-base character set (Unicode General Category M*)
			kCharacterSetDecomposable,			// Canonically decomposable character set
			kCharacterSetAlphaNumeric,			// Alpha Numeric character set (Unicode General Category L*, M*, & N*)
			kCharacterSetPunctuation,			// Punctuation character set (Unicode General Category P*)
			kCharacterSetIllegal,				// Illegal character set
			kCharacterSetCapitalizedLetter,		// Titlecase character set (Unicode General Category Lt)
			kCharacterSetSymbol,				// Symbol character set (Unicode General Category S*)
		};

		enum SpecialFormattingOptions {
			// Bytes - Decimal
			kSpecialFormattingOptionsBytesDecimal						= 1 << 0,
			kSpecialFormattingOptionsBytesDecimalDoEasyRead				= 1 << 1,
			kSpecialFormattingOptionsBytesDecimalDoOrAddExact			= 1 << 2,
			kSpecialFormattingOptionsBytesDecimalDoOrAddExactUseCommas	= 1 << 3,
			kSpecialFormattingOptionsBytesDecimalDoOrAddExactAddLabel	= 1 << 4,

			kSpecialFormattingOptionsBytesDecimalDefault =
					kSpecialFormattingOptionsBytesDecimal |
					kSpecialFormattingOptionsBytesDecimalDoEasyRead |
					kSpecialFormattingOptionsBytesDecimalDoOrAddExact |
					kSpecialFormattingOptionsBytesDecimalDoOrAddExactUseCommas |
					kSpecialFormattingOptionsBytesDecimalDoOrAddExactAddLabel,

			kSpecialFormattingOptionsBytesDecimalMask =
					kSpecialFormattingOptionsBytesDecimal |
					kSpecialFormattingOptionsBytesDecimalDoEasyRead |
					kSpecialFormattingOptionsBytesDecimalDoOrAddExact |
					kSpecialFormattingOptionsBytesDecimalDoOrAddExactUseCommas |
					kSpecialFormattingOptionsBytesDecimalDoOrAddExactAddLabel,

			// Bytes - Binary
			kSpecialFormattingOptionsBytesBinary						= 1 << 5,
			kSpecialFormattingOptionsBytesBinaryDoEasyRead				= 1 << 6,
			kSpecialFormattingOptionsBytesBinaryDoOrAddExact			= 1 << 7,
			kSpecialFormattingOptionsBytesBinaryDoOrAddExactUseCommas	= 1 << 8,
			kSpecialFormattingOptionsBytesBinaryDoOrAddExactAddLabel	= 1 << 9,

			kSpecialFormattingOptionsBytesBinaryDefault =
					kSpecialFormattingOptionsBytesBinary |
					kSpecialFormattingOptionsBytesBinaryDoEasyRead |
					kSpecialFormattingOptionsBytesBinaryDoOrAddExact |
					kSpecialFormattingOptionsBytesBinaryDoOrAddExactUseCommas |
					kSpecialFormattingOptionsBytesBinaryDoOrAddExactAddLabel,

			kSpecialFormattingOptionsBytesBinaryMask =
					kSpecialFormattingOptionsBytesBinary |
					kSpecialFormattingOptionsBytesBinaryDoEasyRead |
					kSpecialFormattingOptionsBytesBinaryDoOrAddExact |
					kSpecialFormattingOptionsBytesBinaryDoOrAddExactUseCommas |
					kSpecialFormattingOptionsBytesBinaryDoOrAddExactAddLabel,

			// Currency - Dollars (value is cents)
			kSpecialFormattingOptionsCurrencyDollars					= 1 << 10,
			kSpecialFormattingOptionsCurrencyDollarsAddDollarsign		= 1 << 11,
			kSpecialFormattingOptionsCurrencyDollarsUseCommas			= 1 << 12,
			kSpecialFormattingOptionsCurrencyDollarsAddCents			= 1 << 13,

			kSpecialFormattingOptionsCurrencyDollarsDefault =
					kSpecialFormattingOptionsCurrencyDollars |
					kSpecialFormattingOptionsCurrencyDollarsAddDollarsign |
					kSpecialFormattingOptionsCurrencyDollarsUseCommas |
					kSpecialFormattingOptionsCurrencyDollarsAddCents,

			kSpecialFormattingOptionsCurrencyDollarsMask =
					kSpecialFormattingOptionsCurrencyDollars |
					kSpecialFormattingOptionsCurrencyDollarsAddDollarsign |
					kSpecialFormattingOptionsCurrencyDollarsUseCommas |
					kSpecialFormattingOptionsCurrencyDollarsAddCents,
		};

	// Types
	public:
		typedef	UInt32	CharIndex;
		typedef	UInt32	Length;

	// Structs
	public:
		struct C {
					// Lifecycle methods
					C(Length length)
						{
							// Setup
							mBuffer = new char[std::max<Length>(length, 1)];
							mBuffer[0] = 0;

							mReferenceCount = new UInt32;
							*mReferenceCount = 1;
						}
					C(const C& other) :
						mBuffer(other.mBuffer), mReferenceCount(other.mReferenceCount)
						{ (*mReferenceCount)++; }
					~C()
						{
							// Check if need to cleanup
							if (--(*mReferenceCount) == 0) {
								// Cleanup
								DeleteArray(mBuffer);
								Delete(mReferenceCount);
							}
						}

					// Instance methods
			char*	operator*() const
						{ return mBuffer; }

			void	hashInto(CHasher& hasher) const
						{ hasher.add(mBuffer); }

			// Properties
			private:
				char*	mBuffer;
				UInt32*	mReferenceCount;
		};

		struct Range {
					// Lifecycle methods
					Range(CharIndex start, Length length) : mStart(start), mLength(length) {}

					// Instance methods
			bool	isValid() const
						{ return mLength != 0; }

			// Properties
			CharIndex	mStart;
			Length		mLength;	// 0 if not found
		};

	// Methods
	public:
										// Lifecycle methods
										CString();
										CString(const CString& other, OV<Length> length = OV<Length>());
										CString(OSStringVar(initialString), OV<Length> length = OV<Length>());
										CString(const char* chars, Length length,
												Encoding encoding = kEncodingTextDefault);
										CString(const char* chars, Encoding encoding = kEncodingTextDefault);
										CString(const UTF16Char* chars, Length length,
												Encoding encoding = kEncodingUTF16Native);
										CString(const UTF32Char* chars, Length length,
												Encoding encoding = kEncodingUTF32Native);

										CString(Float32 value, UInt32 fieldSize = 0,
												UInt32 digitsAfterDecimalPoint = 10, bool padWithZeros = false);
										CString(Float64 value, UInt32 fieldSize = 0,
												UInt32 digitsAfterDecimalPoint = 10, bool padWithZeros = false);
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
										CString(UInt64 value, SpecialFormattingOptions specialFormattingOptions);
										CString(const void* pointer);

										CString(const TArray<CString>& components,
												const CString& separator = CString(OSSTR(", ")));

										CString(const CData& data, Encoding encoding = kEncodingTextDefault);

										CString(const CString& localizationGroup, const CString& localizationKey,
												const CDictionary& localizationInfo);
										CString(const CString& localizationGroup, const CString& localizationKey);

										~CString();

										// CEquatable methods
						bool			operator==(const CEquatable& other) const
											{ return equals((const CString&) other); }

										// CHashable methods
						void			hashInto(CHasher& hasher) const
											{ getCString().hashInto(hasher); }

										// Instance methods
						OSStringType	getOSString() const;
				const	C				getCString(Encoding encoding = kEncodingTextDefault) const;
						Length			getLength() const;
						Length			getLength(Encoding encoding, SInt8 lossCharacter = '\0',
												bool forExternalStorageOrTransmission = false) const;
						bool			isEmpty() const
											{ return getLength() == 0; }

						Length			get(char* buffer, Length bufferLen, bool addNull = true,
												Encoding encoding = kEncodingTextDefault) const;
						Length			get(UTF16Char* buffer, Length bufferLen,
												Encoding encoding = kEncodingUTF16Native) const;

						UTF32Char		getCharacterAtIndex(CharIndex charIndex) const;

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
						
						CData			getData(Encoding encoding = kEncodingTextDefault, SInt8 lossCharacter = '\0')
												const;
						
						CString			getSubString(CharIndex startIndex, OV<Length> length = OV<Length>()) const;
						CString			replacingSubStrings(const CString& subStringToReplace,
												const CString& replacementString = CString::mEmpty) const;
						CString			replacingCharacters(CharIndex startIndex = 0, OV<Length> length = OV<Length>(),
												const CString& replacementString = CString::mEmpty) const;

						Range			findSubString(const CString& subString, CharIndex startIndex = 0,
												OV<Length> length = OV<Length>()) const;
						
						CString			lowercased() const;
						CString			uppercased() const;
						CString			removingLeadingAndTrailingWhitespace() const;
						CString			removingAllWhitespace() const;
						CString			removingLeadingAndTrailingQuotes() const;

						bool			isValidEmailAddress() const;
						CString			getCommonPrefix(const CString& other) const;
					
						TArray<CString>	components(const CString& separator, bool includeEmptyComponents = true) const;
						TArray<CString>	componentsRespectingQuotes(const CString& separator) const;

						bool			compareTo(const CString& other,
												CompareFlags compareFlags = kCompareFlagsDefault) const;
						bool			equals(const CString& other, CompareFlags compareFlags = kCompareFlagsDefault)
												const;
						bool			hasPrefix(const CString& other) const;
						bool			hasSuffix(const CString& other) const;
						bool			contains(const CString& other, CompareFlags compareFlags = kCompareFlagsDefault)
												const;
												
						bool			operator==(const CString& other) const
											{ return equals(other); }
						CString&		operator=(const CString& other);
						CString&		operator+=(const CString& other);
						CString			operator+(const CString& other) const;
												
										// Class methods
		static			bool			compare(CString* const string1, CString* const string2, void* compareFlags);

		static			CString			make(OSStringType format, ...);
		static			CString			make(OSStringType format, va_list args);
		static			bool			isCharacterInSet(UTF32Char utf32Char, CharacterSet characterSet);

#if defined(TARGET_OS_WINDOWS)
		static			void			setupLocalization(const CData& stringsFileData);
#endif

	protected:
										// Internal methods
						void			init();

	// Properties
	public:
		static	const	CString		mEmpty;

		static	const	CString		mColon;
		static	const	CString		mComma;
		static	const	CString		mDoubleQuotes;
		static	const	CString		mEqualSign;
		static	const	CString		mPeriod;
		static	const	CString		mSpace;
		static	const	CString		mSpaceX4;
		static	const	CString		mTab;

		static	const	CString		mNewline;
		static	const	CString		mLinefeed;
		static	const	CString		mNewlineLinefeed;
		static	const	CString		mPlatformDefaultNewline;
		
	private:
#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS)
						CFStringRef					mStringRef;
#elif defined(TARGET_OS_WINDOWS)
						std::basic_string<TCHAR>	mString;
#endif
};
