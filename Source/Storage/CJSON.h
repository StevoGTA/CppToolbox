//----------------------------------------------------------------------------------------------------------------------
//	CJSON.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDictionary.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CJSON

class CJSON {
	// DataResult
	public:
		struct DataResult {
			// Lifecycle methods
			DataResult(const CData& data) : mData(OI<CData>(data)) {}
			DataResult(const SError& error) : mError(OI<SError>(error)) {}

			// Properties
			OI<CData>	mData;
			OI<SError>	mError;
		};

	// DictionaryResult
	public:
		struct DictionaryResult {
			// Lifecycle methods
			DictionaryResult(const CDictionary& dictionary) : mDictionary(OI<CDictionary>(dictionary)) {}
			DictionaryResult(const SError& error) : mError(OI<SError>(error)) {}

			// Properties
			OI<CDictionary>	mDictionary;
			OI<SError>		mError;
		};

	// Methods
	public:
		// Class methods
		static	DictionaryResult	dictionaryFrom(const CData& data);
		static	DataResult			dataFrom(const CDictionary& dictionary);
};
