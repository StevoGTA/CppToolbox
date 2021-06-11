//----------------------------------------------------------------------------------------------------------------------
//	CJSON.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CJSON.h"

#include "SError.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CString	sErrorDomain(OSSTR("CJSON"));
static	SError	sInvalidItemTypeError(sErrorDomain, 1, CString(OSSTR("Invalid Item Type")));
static	SError	sInvalidTokenError(sErrorDomain, 2, CString(OSSTR("Invalid Token")));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local method declarations

static	OI<SError>				sAddArrayOfDictionaries(CData& data, const TArray<CDictionary>& array);
static	OI<SError>				sAddArrayOfStrings(CData& data, const TArray<CString>& array);
static	OI<SError>				sAddDictionary(CData& data, const CDictionary& dictionary);
static	void					sAddString(CData& data, const CString& string);

static	TIResult<CDictionary>	sReadDictionary(const SInt8*& charPtr);
static	TIResult<CString>		sReadString(const SInt8*& charPtr);
static	TIResult<SValue>		sReadValue(const SInt8*& charPtr);
static	void					sSkipWhitespace(const SInt8*& charPtr);
static	OI<SError>				sValidateToken(const SInt8*& charPtr, char token, bool advance = false);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CJSON

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
TIResult<CDictionary> CJSON::dictionaryFrom(const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	// Read dictionary
	const	SInt8*	charPtr = (const SInt8*) data.getBytePtr();

	return sReadDictionary(charPtr);
}

//----------------------------------------------------------------------------------------------------------------------
TIResult<CData> CJSON::dataFrom(const CDictionary& dictionary)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CData	data;

	// Add dictionary
	OI<SError>	error = sAddDictionary(data, dictionary);
	if (error.hasInstance())
		// Error
		return TIResult<CData>(*error);

	return TIResult<CData>(data);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: Local method definitions

//----------------------------------------------------------------------------------------------------------------------
OI<SError> sAddArrayOfDictionaries(CData& data, const TArray<CDictionary>& array)
//----------------------------------------------------------------------------------------------------------------------
{
	// Start
	data.appendBytes("[", 1);

	// Iterate array
	for (TIteratorD<CDictionary> iterator = array.getIterator(); iterator.hasValue(); iterator.advance()) {
		// Check if first
		if (!iterator.isFirstValue())
			// Add comma
			data.appendBytes(",", 1);

		// Add value
		sAddDictionary(data, *iterator);
	}

	// End
	data.appendBytes("]", 1);

	return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> sAddArrayOfStrings(CData& data, const TArray<CString>& array)
//----------------------------------------------------------------------------------------------------------------------
{
	// Start
	data.appendBytes("[", 1);

	// Iterate array
	for (TIteratorD<CString> iterator = array.getIterator(); iterator.hasValue(); iterator.advance()) {
		// Check if first
		if (!iterator.isFirstValue())
			// Add comma
			data.appendBytes(",", 1);

		// Add value
		sAddString(data, *iterator);
	}

	// End
	data.appendBytes("]", 1);

	return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> sAddDictionary(CData& data, const CDictionary& dictionary)
//----------------------------------------------------------------------------------------------------------------------
{
	// Start
	data.appendBytes("{", 1);

	// Iterate dictionary
	for (TIteratorS<CDictionary::Item> iterator = dictionary.getIterator(); iterator.hasValue(); iterator.advance()) {
		// Check if first
		if (!iterator.isFirstValue())
			// Add comma
			data.appendBytes(",", 1);

		// Add key
		sAddString(data, iterator->mKey);

		// Add colon
		data.appendBytes(":", 1);

		// Add value
		OI<SError>	error;
		switch (iterator->mValue.getType()) {
			case SValue::kEmpty:
				// Empty (null)
				data.appendBytes("null", 4);
				break;

			case SValue::kArrayOfDictionaries:
				// Array of dictionaries
				error = sAddArrayOfDictionaries(data, iterator->mValue.getArrayOfDictionaries());
				ReturnErrorIfError(error);
				break;

			case SValue::kArrayOfStrings:
				// Array of strings
				error = sAddArrayOfStrings(data, iterator->mValue.getArrayOfStrings());
				ReturnErrorIfError(error);
				break;

			case SValue::kBool:
				// Add bool
				if (iterator->mValue.getBool())
					// True
					data.appendBytes("true", 4);
				else
					// False
					data.appendBytes("false", 5);
				break;

			case SValue::kDictionary:
				// Add dictionary
				error = sAddDictionary(data, iterator->mValue.getDictionary());
				ReturnErrorIfError(error);
				break;

			case SValue::kString:
				// String
				sAddString(data, iterator->mValue.getString());
				break;

			case SValue::kFloat32:
				// Float32
				data += CString(iterator->mValue.getFloat32()).getData();
				break;

			case SValue::kFloat64:
				// Float64
				data += CString(iterator->mValue.getFloat64()).getData();
				break;

			case SValue::kSInt8:
				// SInt8
				data += CString(iterator->mValue.getSInt8()).getData();
				break;

			case SValue::kSInt16:
				// SInt16
				data += CString(iterator->mValue.getSInt16()).getData();
				break;

			case SValue::kSInt32:
				// SInt32
				data += CString(iterator->mValue.getSInt32()).getData();
				break;

			case SValue::kSInt64:
				// SInt64
				data += CString(iterator->mValue.getSInt64()).getData();
				break;

			case SValue::kUInt8:
				// UInt8
				data += CString(iterator->mValue.getUInt8()).getData();
				break;

			case SValue::kUInt16:
				// UInt16
				data += CString(iterator->mValue.getUInt16()).getData();
				break;

			case SValue::kUInt32:
				// UInt32
				data += CString(iterator->mValue.getUInt32()).getData();
				break;

			case SValue::kUInt64:
				// UInt64
				data += CString(iterator->mValue.getUInt64()).getData();
				break;

			case SValue::kData:
			case SValue::kOpaque:
				// Invalid
				return OI<SError>(sInvalidItemTypeError);
		}
	}

	// End
	data.appendBytes("}", 1);

	return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
void sAddString(CData& data, const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	data.appendBytes("\"", 1);
	data +=
			string
					.replacingSubStrings(CString(OSSTR("\\")), CString(OSSTR("\\\\")))
					.replacingSubStrings(CString(OSSTR("\t")), CString(OSSTR("\\t")))
					.replacingSubStrings(CString(OSSTR("\r")), CString(OSSTR("\\r")))
					.replacingSubStrings(CString(OSSTR("\n")), CString(OSSTR("\\n")))
					.replacingSubStrings(CString(OSSTR("\f")), CString(OSSTR("\\f")))
					.replacingSubStrings(CString(OSSTR("\b")), CString(OSSTR("\\b")))
					.replacingSubStrings(CString(OSSTR("/")), CString(OSSTR("\\/")))
					.replacingSubStrings(CString(OSSTR("\"")), CString(OSSTR("\\\"")))
					.getData(CString::kEncodingUTF8);
	data.appendBytes("\"", 1);
}

//----------------------------------------------------------------------------------------------------------------------
TIResult<CDictionary> sReadDictionary(const SInt8*& charPtr)
//----------------------------------------------------------------------------------------------------------------------
{
	// Validate start token
	OI<SError>	error = sValidateToken(charPtr, '{', true);
	ReturnValueIfError(error, TIResult<CDictionary>(*error));

	// Skip whitespace
	sSkipWhitespace(charPtr);

	// Iterate entries
	CDictionary	dictionary;
	while (true) {
		// Inspect token
		if (*charPtr == '\"') {
			// Read key
			TIResult<CString>	keyResult = sReadString(charPtr);
			ReturnValueIfError(keyResult.getError(), TIResult<CDictionary>(*keyResult.getError()));

			// Skip whitespace
			sSkipWhitespace(charPtr);

			// Validate token
			error = sValidateToken(charPtr, ':', true);
			ReturnValueIfError(error, TIResult<CDictionary>(*error));

			// Read value
			TIResult<SValue>	valueResult = sReadValue(charPtr);
			ReturnValueIfError(valueResult.getError(), TIResult<CDictionary>(*valueResult.getError()));

			// Check if got value
			if (valueResult.getValue().hasInstance())
				// Store
				dictionary.set(*keyResult.getValue(), *valueResult.getValue());

			// Skip whitespace
			sSkipWhitespace(charPtr);

			// Check next token
			if (*charPtr == ',') {
				// More items
				charPtr++;

				sSkipWhitespace(charPtr);
			} else if (*charPtr != '}')
				// Invalid token
				return TIResult<CDictionary>(sInvalidTokenError);
		} else if (*charPtr == '}') {
			// End
			charPtr++;

			return TIResult<CDictionary>(dictionary);
		} else
			// Invalid token
			return TIResult<CDictionary>(sInvalidTokenError);
	}
}

//----------------------------------------------------------------------------------------------------------------------
TIResult<CString> sReadString(const SInt8*& charPtr)
//----------------------------------------------------------------------------------------------------------------------
{
	// Validate opening "
	OI<SError>	error = sValidateToken(charPtr, '\"', true);
	ReturnValueIfError(error, TIResult<CString>(*error));

	// Scan looking for closing "
	const	SInt8*	startCharPtr = charPtr;
	while (true) {
		// Check character
		if ((*charPtr == '\\') && (*(charPtr + 1) == '\"'))
			// Not the quotes we're looking for
			charPtr += 2;
		else if (*charPtr == '\"') {
			// End quote
			charPtr++;

			return TIResult<CString>(
					CString((char*) startCharPtr, (CString::Length) (charPtr - startCharPtr - 1),
									CString::kEncodingUTF8)
							.replacingSubStrings(CString(OSSTR("\\\"")), CString(OSSTR("\"")))
							.replacingSubStrings(CString(OSSTR("\\/")), CString(OSSTR("/")))
							.replacingSubStrings(CString(OSSTR("\\b")), CString(OSSTR("\b")))
							.replacingSubStrings(CString(OSSTR("\\f")), CString(OSSTR("\f")))
							.replacingSubStrings(CString(OSSTR("\\n")), CString(OSSTR("\n")))
							.replacingSubStrings(CString(OSSTR("\\r")), CString(OSSTR("\r")))
							.replacingSubStrings(CString(OSSTR("\\t")), CString(OSSTR("\t")))
							.replacingSubStrings(CString(OSSTR("\\\\")), CString(OSSTR("\\"))));
		} else
			// One more char
			charPtr++;
	}
}

//----------------------------------------------------------------------------------------------------------------------
TIResult<SValue> sReadValue(const SInt8*& charPtr)
//----------------------------------------------------------------------------------------------------------------------
{
	// Skip whitespace
	sSkipWhitespace(charPtr);

	// Check token
	if (*charPtr == '\"') {
		// String
		TIResult<CString>	result = sReadString(charPtr);
		ReturnValueIfError(result.getError(), TIResult<SValue>(*result.getError()));

		// Skip whitespace
		sSkipWhitespace(charPtr);

		return TIResult<SValue>(SValue(*result.getValue()));
	} else if (*charPtr == '{') {
		// Dictionary
		TIResult<CDictionary>	result = sReadDictionary(charPtr);
		ReturnValueIfError(result.getError(), TIResult<SValue>(*result.getError()));

		// Skip whitespace
		sSkipWhitespace(charPtr);

		return TIResult<SValue>(SValue(*result.getValue()));
	} else if (*charPtr == '[') {
		// Array
		charPtr++;

		// Skip whitespace
		sSkipWhitespace(charPtr);

		if (*charPtr == '{') {
			// Array of dictionaries
			TNArray<CDictionary>	array;
			while (true) {
				// Read dictionary
				TIResult<CDictionary>	result = sReadDictionary(charPtr);
				ReturnValueIfError(result.getError(), TIResult<SValue>(*result.getError()));
				array += *result.getValue();

				// Skip whitespace
				sSkipWhitespace(charPtr);

				// Check token
				if (*charPtr == ',') {
					// More values
					charPtr++;

					// Skip whitespace
					sSkipWhitespace(charPtr);
				} else if (*charPtr == ']') {
					// End of array
					charPtr++;

					// Skip whitespace
					sSkipWhitespace(charPtr);

					return TIResult<SValue>(SValue(array));
				} else
					// Invalid token
					return TIResult<SValue>(sInvalidTokenError);
			}
		} else if (*charPtr == '\"') {
			// Array of strings
			TNArray<CString>	array;
			while (true) {
				// Read string
				TIResult<CString>	result = sReadString(charPtr);
				ReturnValueIfError(result.getError(), TIResult<SValue>(*result.getError()));
				array += *result.getValue();

				// Skip whitespace
				sSkipWhitespace(charPtr);

				// Check token
				if (*charPtr == ',') {
					// More values
					charPtr++;

					// Skip whitespace
					sSkipWhitespace(charPtr);
				} else if (*charPtr == ']') {
					// End of array
					charPtr++;

					// Skip whitespace
					sSkipWhitespace(charPtr);

					return TIResult<SValue>(SValue(array));
				} else
					// Invalid token
					return TIResult<SValue>(sInvalidTokenError);
			}
		} else if (*charPtr == ']') {
			// Empty array
			charPtr++;

			// Skip whitespace
			sSkipWhitespace(charPtr);

			return TIResult<SValue>(SValue(TNArray<CDictionary>()));
		} else
			// Invalid token
			return TIResult<SValue>(sInvalidTokenError);
	} else if (::memcmp(charPtr, "true", 4) == 0) {
		// True
		charPtr += 4;

		// Skip whitespace
		sSkipWhitespace(charPtr);

		return TIResult<SValue>(SValue(true));
	} else if (::memcmp(charPtr, "false", 5) == 0) {
		// False
		charPtr += 5;

		// Skip whitespace
		sSkipWhitespace(charPtr);

		return TIResult<SValue>(SValue(false));
	} else if (::memcmp(charPtr, "null", 4) == 0) {
		// Null
		charPtr += 4;

		// Skip whitespace
		sSkipWhitespace(charPtr);

		return TIResult<SValue>(SValue::mEmpty);
	} else {
		// Number
		const	SInt8*	startCharPtr = charPtr;
				bool	isFloat = false;
		while ((*charPtr != '}') && (*charPtr != ',') && (*charPtr != ']') &&
				(*charPtr != ' ') && (*charPtr != '\t') && (*charPtr != '\n') && (*charPtr != '\r')) {
			// Still in number
			isFloat |= (*charPtr == '.');
			charPtr++;
		}

		// Skip whitespace
		sSkipWhitespace(charPtr);

		if (isFloat)
			// Float
			return TIResult<SValue>(CString((char*) startCharPtr, (UInt32) (charPtr - startCharPtr)).getFloat64());
		else
			// Integer
			return TIResult<SValue>(CString((char*) startCharPtr, (UInt32) (charPtr - startCharPtr)).getSInt64());
	}
}

//----------------------------------------------------------------------------------------------------------------------
void sSkipWhitespace(const SInt8*& charPtr)
//----------------------------------------------------------------------------------------------------------------------
{
	// Skip whitespace
	while ((*charPtr == ' ') || (*charPtr == '\t') || (*charPtr == '\n') || (*charPtr == '\r')) charPtr++;
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> sValidateToken(const SInt8*& charPtr, char token, bool advance)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get char
	char	foundToken = advance ? *(charPtr++) : *charPtr;

	return (foundToken == token) ? OI<SError>() : OI<SError>(sInvalidTokenError);
}
