//----------------------------------------------------------------------------------------------------------------------
//	CString.h			©2003 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CArray.h"
#include "CHashable.h"
#include "TBuffer.h"
#include "TRange.h"

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
	// CompareToOptions
	public:
		enum CompareToOptions {
			kCompareToOptionsNone				= 0,
			kCompareToOptionsCaseInsensitive	= 1 << 0,	// FOO.TXT == foo.txt
			kCompareToOptionsNonliteral			= 1 << 1,	// Loose equivalence is performed (o-umlaut == o, umlaut)
			kCompareToOptionsNumerically		= 1 << 2,	// Numeric comparison; i.e. Foo2.txt < Foo7.txt < Foo25.txt

			kCompareToOptionsDefault =
					kCompareToOptionsNonliteral,
			kCompareToOptionsSortDefault =
					kCompareToOptionsCaseInsensitive |
					kCompareToOptionsNonliteral |
					kCompareToOptionsNumerically,
		};

	// ContainsOptions
	public:
		enum ContainsOptions {
			kContainsOptionsNone			= 0,
			kContainsOptionsCaseInsensitive	= 1 << 0,	// FOO.TXT == foo.txt
			kContainsOptionsNonliteral		= 1 << 1,	// Loose equivalence is performed (o-umlaut == o, umlaut)

			kContainsOptionsDefault =
					kContainsOptionsNonliteral,
		};

	// Encoding
	public:
		enum Encoding {
			kEncodingASCII,		// 0..127 (values greater than 127 are treated as corresponding Unicode value)
			kEncodingMacRoman,
			kEncodingUTF8,
			kEncodingISOLatin,	// ISO 8859-1

			kEncodingUTF16,		// UTF 16 w/ BOM
			kEncodingUTF16BE,	// UTF 16 w/o BOM and explicit BE order
			kEncodingUTF16LE,	// UTF 16 w/o BOM and explicit LE order

			kEncodingUTF32BE,	// UTF 32 w/o BOM and explicit BE order
			kEncodingUTF32LE,	// UTF 32 w/o BOM and explicit LE order

#if TARGET_RT_BIG_ENDIAN
			kEncodingUTF16Native = kEncodingUTF16BE,
			kEncodingUTF32Native = kEncodingUTF32BE,
#else
			kEncodingUTF16Native = kEncodingUTF16LE,
			kEncodingUTF32Native = kEncodingUTF32LE,
#endif
		};

	// CharacterSet
	public:
		enum CharacterSet {
			kCharacterSetControl,				// Control character set (Unicode General Category Cc and Cf
			kCharacterSetWhitespace,			// Whitespace character set (Unicode General Category Zs and U0009
												//	CHARACTER TABULATION
			kCharacterSetWhitespaceAndNewline,	// Whitespace and Newline character set (Unicode General Category Z*,
												//	U000A ~ U000D, and U0085)
			kCharacterSetDecimalDigit,			// 0 - 9
			kCharacterSetFloatingPoint,			// 0 - 9 and .
			kCharacterSetLetter,				// Letter character set (Unicode General Category L* & M*)
			kCharacterSetLowercaseLetter,		// Lowercase character set (Unicode General Category Ll)
			kCharacterSetUppercaseLetter,		// Uppercase character set (Unicode General Category Lu and Lt)
			kCharacterSetAlphaNumeric,			// Alpha Numeric character set (Unicode General Category L*, M*, & N*)
			kCharacterSetPunctuation,			// Punctuation character set (Unicode General Category P*)
		};

	// SpecialFormattingOptions
	public:
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
			void	hashInto(CHashable::HashCollector& hashableHashCollector) const
						{ hashableHashCollector.add(mBuffer); }

			char*	operator*() const
						{ return mBuffer; }

			// Properties
			private:
				char*	mBuffer;
				UInt32*	mReferenceCount;
		};

	// Methods
	public:
											// Lifecycle methods
											CString();
											CString(const CString& other);
											CString(OSStringVar(initialString));
											CString(const char chars[]);
											CString(const void* ptr, UInt32 byteCount, Encoding encoding);
											CString(const TBuffer<UTF32Char>& buffer);

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

											CString(const CData& data, Encoding encoding);

											CString(const TArray<CString>& components,
													const CString& separator = CString(OSSTR(", ")));

											CString(const CString& localizationGroup, const CString& localizationKey,
													const CDictionary& localizationInfo);
											CString(const CString& localizationGroup, const CString& localizationKey);

											~CString();

											// CEquatable methods
						bool				operator==(const CEquatable& other) const
												{ return equals((const CString&) other); }

											// CHashable methods
						void				hashInto(CHashable::HashCollector& hashableHashCollector) const
												{ getUTF8String().hashInto(hashableHashCollector); }

											// Instance methods
						OSStringType		getOSString() const;
				const	C					getUTF8String() const;
						Length				getLength() const;
						bool				isEmpty() const
												{ return getLength() == 0; }

						Float32				getFloat32() const;
						void				getValue(Float32& value) const
												{ value = getFloat32(); }
						OV<Float32>			getOVFloat32() const;
						Float64				getFloat64() const;
						void				getValue(Float64& value) const
												{ value = getFloat64(); }
						OV<Float64>			getOVFloat64() const;
						SInt8				getSInt8(UInt8 base = 10) const;
						void				getValue(SInt8& value, UInt8 base = 10) const
												{ value = getSInt8(base); }
						OV<SInt8>			getOVSInt8() const;
						SInt16				getSInt16(UInt8 base = 10) const;
						void				getValue(SInt16& value, UInt8 base = 10) const
												{ value = getSInt16(base); }
						OV<SInt16>			getOVSInt16() const;
						SInt32				getSInt32(UInt8 base = 10) const;
						void				getValue(SInt32& value, UInt8 base = 10) const
												{ value = getSInt32(base); }
						OV<SInt32>			getOVSInt32() const;
						SInt64				getSInt64(UInt8 base = 10) const;
						void				getValue(SInt64& value, UInt8 base = 10) const
												{ value = getSInt64(base); }
						OV<SInt64>			getOVSInt64() const;
						UInt8				getUInt8(UInt8 base = 10) const;
						void				getValue(UInt8& value, UInt8 base = 10) const
												{ value = getUInt8(base); }
						OV<UInt8>			getOVUInt8() const;
						UInt16				getUInt16(UInt8 base = 10) const;
						void				getValue(UInt16& value, UInt8 base = 10) const
												{ value = getUInt16(base); }
						OV<UInt16>			getOVUInt16() const;
						UInt32				getUInt32(UInt8 base = 10) const;
						void				getValue(UInt32& value, UInt8 base = 10) const
												{ value = getUInt32(base); }
						OV<UInt32>			getOVUInt32() const;
						OSType				getOSType() const;
						UInt64				getUInt64(UInt8 base = 10) const;
						void				getValue(UInt64& value, UInt8 base = 10) const
												{ value = getUInt64(base); }
						OV<UInt64>			getOVUInt64() const;
						UInt64				getAsByteCount() const;
						
						TBuffer<char>		getUTF8Chars() const;
						TBuffer<UTF32Char>	getUTF32Chars() const;
						OV<CData>			getData(Encoding encoding) const;
						CData				getUTF8Data() const;

						CString				getSubString(CharIndex startIndex, OV<Length> length = OV<Length>()) const;
						CString				getSubString(CharIndex startIndex, Length length) const
												{ return getSubString(startIndex, OV<Length>(length)); }
						CString				replacingSubStrings(const CString& subStringToReplace,
													const CString& replacementString = CString::mEmpty) const;
						CString				replacingCharacters(CharIndex startIndex = 0,
													OV<Length> length = OV<Length>(),
													const CString& replacementString = CString::mEmpty) const;

						OV<SRange32>		findSubString(const CString& subString, CharIndex startIndex = 0,
													const OV<Length>& length = OV<Length>()) const;
						
						CString				lowercased() const;
						CString				uppercased() const;
						CString				capitalizingFirstLetter() const;
						CString				removingLeadingAndTrailingWhitespace() const;
						CString				removingAllWhitespace() const;
						CString				removingLeadingAndTrailingQuotes() const;
	
						bool				isValidEmailAddress() const;
						CString				getCommonPrefix(const CString& other) const;
					
						TArray<CString>		components(const CString& separator, bool includeEmptyComponents = true)
													const;
						TArray<CString>		componentsRespectingQuotes(const CString& separator) const;

						bool				compareTo(const CString& other,
													CompareToOptions compareToOptions = kCompareToOptionsDefault) const;
						bool				hasPrefix(const CString& other) const;
						bool				hasSuffix(const CString& other) const;
						bool				contains(const CString& other,
													ContainsOptions containsOptions = kContainsOptionsDefault) const;
						bool				equals(const CString& other,
													ContainsOptions containsOptions = kContainsOptionsDefault) const;
						bool				containsOnly(CharacterSet characterSet) const;

						bool				operator==(const CString& other) const
												{ return equals(other); }
						CString&			operator=(const CString& other);
						CString&			operator+=(const CString& other);
						CString				operator+(const CString& other) const;
												
											// Class methods
		static			bool				compare(const CString& string1, const CString& string2,
													void* compareToOptions);

		static			CString				lowercase(const CString* string, void* userData);
		static			CString				uppercase(const CString* string, void* userData);

		static			CString				fromPascal(const UInt8* ptr)
												{ return CString(ptr + 1, ptr[0], kEncodingMacRoman); }
		static			CString				fromPascal(const CData& data);
		static			CString				make(OSStringType format, ...);
		static			CString				make(OSStringType format, va_list args);

		static			bool				isEmpty(const CString& string, void* userData);
		static			bool				isNotEmpty(const CString& string, void* userData);

#if defined(TARGET_OS_WINDOWS)
		static			void				setupLocalization(const CData& stringsFileData);
#endif

	protected:
											// Internal methods
						void				init();

	// Properties
	public:
		static	const	CString		mEmpty;

		static	const	CString		mColon;
		static	const	CString		mComma;
		static	const	CString		mDoubleQuote;
		static	const	CString		mEqualSign;
		static	const	CString		mHyphen;
		static	const	CString		mNull;
		static	const	CString		mParenthesisClose;
		static	const	CString		mParenthesisOpen;
		static	const	CString		mPercent;
		static	const	CString		mPeriod;
		static	const	CString		mSemiColon;
		static	const	CString		mSlash;
		static	const	CString		mSpace;
		static	const	CString		mSpaceX4;
		static	const	CString		mTab;
		static	const	CString		mUnderscore;

		static	const	CString		mLinefeed;
		static	const	CString		mLineFeedNewline;
		static	const	CString		mNewline;
		static	const	CString		mNewlineLinefeed;
		static	const	CString		mPlatformDefaultNewline;
		
	private:
#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS) || defined(TARGET_OS_WATCHOS)
						CFStringRef					mStringRef;
#elif defined(TARGET_OS_WINDOWS)
						std::basic_string<TCHAR>	mString;
#endif
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Macros

#define CSTRING_LAZYILY_LOCALIZED(localizationGroup, localizationKey)				\
	static	CString*	sString = nil;												\
	if (sString == nil)																\
		sString = new CString(localizationGroup, CString(OSSTR(localizationKey)));	\
	return *sString;
