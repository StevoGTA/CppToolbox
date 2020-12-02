//----------------------------------------------------------------------------------------------------------------------
//	CBinaryPropertyList.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CByteParceller.h"
#include "CDictionary.h"
#include "CFile.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CBinaryPropertyList

class CBinaryPropertyList : public CDictionary {
	// Methods
	public:
						// Class methods
	static	CDictionary	dictionaryFrom(const CByteParceller& byteParceller, OI<SError>& outError);
	static	OI<SError>	writeTo(const CFile& file);
};
