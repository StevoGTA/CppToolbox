//----------------------------------------------------------------------------------------------------------------------
//	CJSON.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDictionary.h"
#include "TResult.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CJSON

class CJSON {
	// Methods
	public:
		// Class methods
		static	TVResult<CDictionary>	dictionaryFrom(const CData& data);
		static	TVResult<CData>			dataFrom(const CDictionary& dictionary);
};
