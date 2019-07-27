//----------------------------------------------------------------------------------------------------------------------
//	CBinaryPropertyList.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDataSource.h"
#include "CDictionary.h"
#include "CFile.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Errors

const	UErrorDomain	kBinaryPropertyListErrorDomain			= MAKE_OSTYPE('B', 'i', 'P', 'L');
const	UError			kBinaryPropertyListUnknownFormatError	= MAKE_UError(kBinaryPropertyListErrorDomain, 1);

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CBinaryPropertyList

class CBinaryPropertyList : public CDictionary {
	// Methods
	public:
						// Class methods
	static	CDictionary	dictionaryFrom(const CDataSource& dataSource, UError& outError);
	static	UError		writeTo(const CFile& file);
};
