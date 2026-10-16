//----------------------------------------------------------------------------------------------------------------------
//	CBinaryPropertyList.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDataSource.h"
#include "CDictionary.h"
#include "CFile.h"
#include "TWrappers.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CBinaryPropertyList

class CBinaryPropertyList {
	// Methods
	public:
										// Class methods
		static	TVResult<CDictionary>	dictionaryFrom(const I<CRandomAccessDataSource>& randomAccessDataSource,
												bool composeStandardDictionary = false);
		static	TVResult<CDictionary>	dictionaryFrom(const CData& data, bool composeStandardDictionary = false);

		static	TVResult<CData>			dataFrom(const CDictionary& dictionary);
		static	OV<SError>				write(const CDictionary& dictionary, const CFile& file);
};
