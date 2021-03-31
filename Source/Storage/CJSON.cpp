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

static	OI<CDictionary>			sReadDictionary(const SInt8*& charPtr, OI<SError>& outError);
static	OI<CString>				sReadString(const SInt8*& charPtr, OI<SError>& outError);
static	OI<CDictionary::Value>	sReadValue(const SInt8*& charPtr, OI<SError>& outError);
static	void					sSkipWhitespace(const SInt8*& charPtr);
static	OI<SError>				sValidateToken(const SInt8*& charPtr, char token, bool advance = false);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CJSON

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CJSON::DictionaryResult CJSON::dictionaryFrom(const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	// Read dictionary
			OI<SError>		error;
	const	SInt8*			charPtr = (const SInt8*) data.getBytePtr();
			OI<CDictionary>	dictionary = sReadDictionary(charPtr, error);

	return dictionary.hasInstance() ? DictionaryResult(*dictionary) : DictionaryResult(*error);
}

//----------------------------------------------------------------------------------------------------------------------
CJSON::DataResult CJSON::dataFrom(const CDictionary& dictionary)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CData	data;

	// Add dictionary
	OI<SError>	error = sAddDictionary(data, dictionary);
	if (error.hasInstance())
		// Error
		return DataResult(*error);

	return DataResult(data);
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
			case CDictionary::Value::kArrayOfDictionaries:
				// Array of dictionaries
				error = sAddArrayOfDictionaries(data, iterator->mValue.getArrayOfDictionaries());
				ReturnErrorIfError(error);
				break;

			case CDictionary::Value::kArrayOfStrings:
				// Array of strings
				error = sAddArrayOfStrings(data, iterator->mValue.getArrayOfStrings());
				ReturnErrorIfError(error);
				break;

			case CDictionary::Value::kBool:
				// Add bool
				if (iterator->mValue.getBool())
					// True
					data.appendBytes("true", 4);
				else
					// False
					data.appendBytes("false", 5);
				break;

			case CDictionary::Value::kDictionary:
				// Add dictionary
				error = sAddDictionary(data, iterator->mValue.getDictionary());
				ReturnErrorIfError(error);
				break;

			case CDictionary::Value::kString:
				// String
				sAddString(data, iterator->mValue.getString());
				break;

			case CDictionary::Value::kFloat32:
				// Float32
				data += CString(iterator->mValue.getFloat32()).getData();
				break;

			case CDictionary::Value::kFloat64:
				// Float64
				data += CString(iterator->mValue.getFloat64()).getData();
				break;

			case CDictionary::Value::kSInt8:
				// SInt8
				data += CString(iterator->mValue.getSInt8()).getData();
				break;

			case CDictionary::Value::kSInt16:
				// SInt16
				data += CString(iterator->mValue.getSInt16()).getData();
				break;

			case CDictionary::Value::kSInt32:
				// SInt32
				data += CString(iterator->mValue.getSInt32()).getData();
				break;

			case CDictionary::Value::kSInt64:
				// SInt64
				data += CString(iterator->mValue.getSInt64()).getData();
				break;

			case CDictionary::Value::kUInt8:
				// UInt8
				data += CString(iterator->mValue.getUInt8()).getData();
				break;

			case CDictionary::Value::kUInt16:
				// UInt16
				data += CString(iterator->mValue.getUInt16()).getData();
				break;

			case CDictionary::Value::kUInt32:
				// UInt32
				data += CString(iterator->mValue.getUInt32()).getData();
				break;

			case CDictionary::Value::kUInt64:
				// UInt64
				data += CString(iterator->mValue.getUInt64()).getData();
				break;

			case CDictionary::Value::kData:
			case CDictionary::Value::kItemRef:
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
OI<CDictionary> sReadDictionary(const SInt8*& charPtr, OI<SError>& outError)
//----------------------------------------------------------------------------------------------------------------------
{
	// Validate start token
	outError = sValidateToken(charPtr, '{', true);
	ReturnValueIfError(outError, OI<CDictionary>());

	// Skip whitespace
	sSkipWhitespace(charPtr);

	// Iterate entries
	CDictionary	dictionary;
	while (true) {
		// Inspect token
		if (*charPtr == '\"') {
			// Read key
			OI<CString>	key = sReadString(charPtr, outError);
			ReturnValueIfError(outError, OI<CDictionary>());

			// Skip whitespace
			sSkipWhitespace(charPtr);

			// Validate token
			outError = sValidateToken(charPtr, ':', true);
			ReturnValueIfError(outError, OI<CDictionary>());

			// Read value
			OI<CDictionary::Value>	value = sReadValue(charPtr, outError);
			ReturnValueIfError(outError, OI<CDictionary>());

			// Check if got value
			if (value.hasInstance())
				// Store
				dictionary.set(*key, *value);

			// Skip whitespace
			sSkipWhitespace(charPtr);

			// Check next token
			if (*charPtr == ',') {
				// More items
				charPtr++;

				sSkipWhitespace(charPtr);
			} else if (*charPtr != '}') {
				// Invalid token
				outError =  OI<SError>(sInvalidTokenError);

				return OI<CDictionary>();
			}
		} else if (*charPtr == '}') {
			// End
			charPtr++;

			return OI<CDictionary>(dictionary);
		} else {
			// Invalid token
			outError =  OI<SError>(sInvalidTokenError);

			return OI<CDictionary>();
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
OI<CString> sReadString(const SInt8*& charPtr, OI<SError>& outError)
//----------------------------------------------------------------------------------------------------------------------
{
	// Validate opening "
	outError = sValidateToken(charPtr, '\"', true);
	ReturnValueIfError(outError, OI<CString>());

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

			return OI<CString>(
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
OI<CDictionary::Value> sReadValue(const SInt8*& charPtr, OI<SError>& outError)
//----------------------------------------------------------------------------------------------------------------------
{
	// Skip whitespace
	sSkipWhitespace(charPtr);

	// Check token
	OI<CDictionary::Value>	value;
	if (*charPtr == '\"') {
		// String
		OI<CString>	string = sReadString(charPtr, outError);
		ReturnValueIfError(outError, value);

		value = OI<CDictionary::Value>(CDictionary::Value(*string));
	} else if (*charPtr == '{') {
		// Dictionary
		OI<CDictionary>	dictionary = sReadDictionary(charPtr, outError);
		ReturnValueIfError(outError, value);

		value =
				dictionary.hasInstance() ?
						OI<CDictionary::Value>(CDictionary::Value(*dictionary)) : OI<CDictionary::Value>();
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
				OI<CDictionary>	dictionary = sReadDictionary(charPtr, outError);
				ReturnValueIfError(outError, value);
				array += *dictionary;

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

					return OI<CDictionary::Value>(CDictionary::Value(array));
				} else {
					// Invalid token
					outError =  OI<SError>(sInvalidTokenError);
					break;
				}
			}
		} else if (*charPtr == '\"') {
			// Array of strings
			TNArray<CString>	array;
			while (true) {
				// Read string
				OI<CString>	string = sReadString(charPtr, outError);
				ReturnValueIfError(outError, value);
				array += *string;

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

					return OI<CDictionary::Value>(CDictionary::Value(array));
				} else {
					// Invalid token
					outError =  OI<SError>(sInvalidTokenError);
					break;
				}
			}
		} else if (*charPtr == ']') {
			// Empty array
			charPtr++;
			value = OI<CDictionary::Value>(CDictionary::Value(TNArray<CDictionary>()));
		} else
			// Invalid token
			outError =  OI<SError>(sInvalidTokenError);
	} else if (::memcmp(charPtr, "true", 4) == 0) {
		// True
		charPtr += 4;
		value = OI<CDictionary::Value>(CDictionary::Value(true));
	} else if (::memcmp(charPtr, "false", 5) == 0) {
		// False
		charPtr += 5;
		value = OI<CDictionary::Value>(CDictionary::Value(false));
	} else if (::memcmp(charPtr, "null", 4) == 0) {
		// Null
		charPtr += 4;
		value = OI<CDictionary::Value>();
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

		if (isFloat)
			// Float
			value = OI<CDictionary::Value>(CString((char*) startCharPtr, (UInt32) (charPtr - startCharPtr)).getFloat64());
		else
			// Integer
			value = OI<CDictionary::Value>(CString((char*) startCharPtr, (UInt32) (charPtr - startCharPtr)).getSInt64());
	}

	// Skip whitespace
	sSkipWhitespace(charPtr);

	return value;
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
