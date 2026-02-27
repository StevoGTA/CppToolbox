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

static	OV<SError>						sAddArrayOfDictionaries(CData& data, const TArray<CDictionary>& array);
static	OV<SError>						sAddArrayOfStrings(CData& data, const TArray<CString>& array);
static	OV<SError>						sAddDictionary(CData& data, const CDictionary& dictionary);
static	void							sAddString(CData& data, const CString& string);

static	TVResult<TArray<CDictionary> >	sReadArrayOfDictionaries(const SInt8*& charPtr);
static	TVResult<CDictionary>			sReadDictionary(const SInt8*& charPtr);
static	TVResult<CString>				sReadString(const SInt8*& charPtr);
static	TVResult<SValue>				sReadValue(const SInt8*& charPtr);
static	void							sSkipWhitespace(const SInt8*& charPtr);
static	OV<SError>						sValidateToken(const SInt8*& charPtr, char token, bool advance = false);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CJSON

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
TVResult<CData> CJSON::dataFrom(const TArray<CDictionary>& array)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CData	data;

	// Add array
	OV<SError>	error = sAddArrayOfDictionaries(data, array);
	ReturnValueIfError(error, TVResult<CData>(*error));

	return TVResult<CData>(data);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CData> CJSON::dataFrom(const CDictionary& dictionary)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CData	data;

	// Add dictionary
	OV<SError>	error = sAddDictionary(data, dictionary);
	ReturnValueIfError(error, TVResult<CData>(*error));

	return TVResult<CData>(data);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<TArray<CDictionary> > CJSON::arrayOfDictionariesFrom(const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	// Read array of dictionaries
	const	SInt8*	charPtr = (const SInt8*) data.getBytePtr();

	return sReadArrayOfDictionaries(charPtr);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CDictionary> CJSON::dictionaryFrom(const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	// Read dictionary
	const	SInt8*	charPtr = (const SInt8*) data.getBytePtr();

	return sReadDictionary(charPtr);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: Local method definitions

//----------------------------------------------------------------------------------------------------------------------
OV<SError> sAddArrayOfDictionaries(CData& data, const TArray<CDictionary>& array)
//----------------------------------------------------------------------------------------------------------------------
{
	// Start
	data.appendBytes("[", 1);

	// Iterate array
	for (TArray<CDictionary>::Iterator iterator = array.getIterator(); iterator; iterator++) {
		// Check if first
		if (!iterator.isFirst())
			// Add comma
			data.appendBytes(",", 1);

		// Add value
		sAddDictionary(data, *iterator);
	}

	// End
	data.appendBytes("]", 1);

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> sAddArrayOfStrings(CData& data, const TArray<CString>& array)
//----------------------------------------------------------------------------------------------------------------------
{
	// Start
	data.appendBytes("[", 1);

	// Iterate array
	for (TArray<CString>::Iterator iterator = array.getIterator(); iterator; iterator++) {
		// Check if first
		if (!iterator.isFirst())
			// Add comma
			data.appendBytes(",", 1);

		// Add value
		sAddString(data, *iterator);
	}

	// End
	data.appendBytes("]", 1);

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> sAddDictionary(CData& data, const CDictionary& dictionary)
//----------------------------------------------------------------------------------------------------------------------
{
	// Start
	data.appendBytes("{", 1);

	// Iterate dictionary
	for (CDictionary::Iterator iterator = dictionary.getIterator(); iterator; iterator++) {
		// Check if first
		if (!iterator.isFirst())
			// Add comma
			data.appendBytes(",", 1);

		// Add key
		sAddString(data, iterator.getKey());

		// Add colon
		data.appendBytes(":", 1);

		// Add value
		OV<SError>	error;
		switch (iterator.getValue().getType()) {
			case SValue::kTypeEmpty:
				// Empty (null)
				data.appendBytes("null", 4);
				break;

			case SValue::kTypeArrayOfDictionaries:
				// Array of dictionaries
				error = sAddArrayOfDictionaries(data, iterator.getValue().getArrayOfDictionaries());
				ReturnErrorIfError(error);
				break;

			case SValue::kTypeArrayOfStrings:
				// Array of strings
				error = sAddArrayOfStrings(data, iterator.getValue().getArrayOfStrings());
				ReturnErrorIfError(error);
				break;

			case SValue::kTypeBool:
				// Add bool
				if (iterator.getValue().getBool())
					// True
					data.appendBytes("true", 4);
				else
					// False
					data.appendBytes("false", 5);
				break;

			case SValue::kTypeDictionary:
				// Add dictionary
				error = sAddDictionary(data, iterator.getValue().getDictionary());
				ReturnErrorIfError(error);
				break;

			case SValue::kTypeString:
				// String
				sAddString(data, iterator.getValue().getString());
				break;

			case SValue::kTypeFloat32:
				// Float32
				data += CString(iterator.getValue().getFloat32()).getUTF8Data();
				break;

			case SValue::kTypeFloat64:
				// Float64
				data += CString(iterator.getValue().getFloat64()).getUTF8Data();
				break;

			case SValue::kTypeSInt8:
				// SInt8
				data += CString(iterator.getValue().getSInt8()).getUTF8Data();
				break;

			case SValue::kTypeSInt16:
				// SInt16
				data += CString(iterator.getValue().getSInt16()).getUTF8Data();
				break;

			case SValue::kTypeSInt32:
				// SInt32
				data += CString(iterator.getValue().getSInt32()).getUTF8Data();
				break;

			case SValue::kTypeSInt64:
				// SInt64
				data += CString(iterator.getValue().getSInt64()).getUTF8Data();
				break;

			case SValue::kTypeUInt8:
				// UInt8
				data += CString(iterator.getValue().getUInt8()).getUTF8Data();
				break;

			case SValue::kTypeUInt16:
				// UInt16
				data += CString(iterator.getValue().getUInt16()).getUTF8Data();
				break;

			case SValue::kTypeUInt32:
				// UInt32
				data += CString(iterator.getValue().getUInt32()).getUTF8Data();
				break;

			case SValue::kTypeUInt64:
				// UInt64
				data += CString(iterator.getValue().getUInt64()).getUTF8Data();
				break;

			case SValue::kTypeData:
			case SValue::kTypeOpaque:
				// Invalid
				return OV<SError>(sInvalidItemTypeError);
		}
	}

	// End
	data.appendBytes("}", 1);

	return OV<SError>();
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
					.getUTF8Data();
	data.appendBytes("\"", 1);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<TArray<CDictionary> > sReadArrayOfDictionaries(const SInt8*& charPtr)
//----------------------------------------------------------------------------------------------------------------------
{
	// Validate start token
	OV<SError>	error = sValidateToken(charPtr, '[', true);
	ReturnValueIfError(error, TVResult<TArray<CDictionary> >(*error));

	// Compose array
	TNArray<CDictionary>	array;
	while (true) {
		// Read dictionary
		TVResult<CDictionary>	result = sReadDictionary(charPtr);
		ReturnValueIfResultError(result, TVResult<TArray<CDictionary> >(result.getError()));
		array += *result;

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

			return TVResult<TArray<CDictionary> >(array);
		} else
			// Invalid token
			return TVResult<TArray<CDictionary> >(sInvalidTokenError);
	}
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CDictionary> sReadDictionary(const SInt8*& charPtr)
//----------------------------------------------------------------------------------------------------------------------
{
	// Validate start token
	OV<SError>	error = sValidateToken(charPtr, '{', true);
	ReturnValueIfError(error, TVResult<CDictionary>(*error));

	// Skip whitespace
	sSkipWhitespace(charPtr);

	// Iterate entries
	CDictionary	dictionary;
	while (true) {
		// Inspect token
		if (*charPtr == '\"') {
			// Read key
			TVResult<CString>	keyResult = sReadString(charPtr);
			ReturnValueIfResultError(keyResult, TVResult<CDictionary>(keyResult.getError()));

			// Skip whitespace
			sSkipWhitespace(charPtr);

			// Validate token
			error = sValidateToken(charPtr, ':', true);
			ReturnValueIfError(error, TVResult<CDictionary>(*error));

			// Read value
			TVResult<SValue>	valueResult = sReadValue(charPtr);
			ReturnValueIfResultError(valueResult, TVResult<CDictionary>(valueResult.getError()));

			// Check if got value
			if (valueResult.hasValue())
				// Store
				dictionary.set(*keyResult, *valueResult);

			// Skip whitespace
			sSkipWhitespace(charPtr);

			// Check next token
			if (*charPtr == ',') {
				// More items
				charPtr++;

				sSkipWhitespace(charPtr);
			} else if (*charPtr != '}')
				// Invalid token
				return TVResult<CDictionary>(sInvalidTokenError);
		} else if (*charPtr == '}') {
			// End
			charPtr++;

			return TVResult<CDictionary>(dictionary);
		} else
			// Invalid token
			return TVResult<CDictionary>(sInvalidTokenError);
	}
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CString> sReadString(const SInt8*& charPtr)
//----------------------------------------------------------------------------------------------------------------------
{
	// Validate opening "
	OV<SError>	error = sValidateToken(charPtr, '\"', true);
	ReturnValueIfError(error, TVResult<CString>(*error));

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

			return TVResult<CString>(
					CString((const void*) startCharPtr, (UInt32) (charPtr - startCharPtr - 1), CString::kEncodingUTF8)
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
TVResult<SValue> sReadValue(const SInt8*& charPtr)
//----------------------------------------------------------------------------------------------------------------------
{
	// Skip whitespace
	sSkipWhitespace(charPtr);

	// Check token
	if (*charPtr == '\"') {
		// String
		TVResult<CString>	result = sReadString(charPtr);
		ReturnValueIfResultError(result, TVResult<SValue>(result.getError()));

		// Skip whitespace
		sSkipWhitespace(charPtr);

		return TVResult<SValue>(SValue(*result));
	} else if (*charPtr == '{') {
		// Dictionary
		TVResult<CDictionary>	result = sReadDictionary(charPtr);
		ReturnValueIfResultError(result, TVResult<SValue>(result.getError()));

		// Skip whitespace
		sSkipWhitespace(charPtr);

		return TVResult<SValue>(SValue(*result));
	} else if (*charPtr == '[') {
		// Array
		charPtr++;

		// Skip whitespace
		sSkipWhitespace(charPtr);

		if (*charPtr == '{') {
			// Array of dictionaries
			TVResult<TArray<CDictionary> >	result = sReadArrayOfDictionaries(charPtr);
			ReturnValueIfResultError(result, TVResult<SValue>(result.getError()));

			return TVResult<SValue>(SValue(*result));
		} else if (*charPtr == '\"') {
			// Array of strings
			TNArray<CString>	array;
			while (true) {
				// Read string
				TVResult<CString>	result = sReadString(charPtr);
				ReturnValueIfResultError(result, TVResult<SValue>(result.getError()));
				array += *result;

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

					return TVResult<SValue>(SValue(array));
				} else
					// Invalid token
					return TVResult<SValue>(sInvalidTokenError);
			}
		} else if (*charPtr == ']') {
			// Empty array
			charPtr++;

			// Skip whitespace
			sSkipWhitespace(charPtr);

			return TVResult<SValue>(SValue(TNArray<CDictionary>()));
		} else
			// Invalid token
			return TVResult<SValue>(sInvalidTokenError);
	} else if (::memcmp(charPtr, "true", 4) == 0) {
		// True
		charPtr += 4;

		// Skip whitespace
		sSkipWhitespace(charPtr);

		return TVResult<SValue>(SValue(true));
	} else if (::memcmp(charPtr, "false", 5) == 0) {
		// False
		charPtr += 5;

		// Skip whitespace
		sSkipWhitespace(charPtr);

		return TVResult<SValue>(SValue(false));
	} else if (::memcmp(charPtr, "null", 4) == 0) {
		// Null
		charPtr += 4;

		// Skip whitespace
		sSkipWhitespace(charPtr);

		return TVResult<SValue>(SValue::mEmpty);
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

		// Compose string
		CString	string((const void*) startCharPtr, (UInt32) (charPtr - startCharPtr), CString::kEncodingUTF8);

		return TVResult<SValue>(isFloat ? SValue(string.getFloat64()) : SValue(string.getSInt64()));
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
OV<SError> sValidateToken(const SInt8*& charPtr, char token, bool advance)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get char
	char	foundToken = advance ? *(charPtr++) : *charPtr;

	return (foundToken == token) ? OV<SError>() : OV<SError>(sInvalidTokenError);
}
