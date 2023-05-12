//----------------------------------------------------------------------------------------------------------------------
//	CTextReader.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDataSource.h"
#include "CString.h"
#include "TWrappers.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CTextReader

class CTextReader {
	// Classes
	private:
		class Internals;

	// Methods
	public:
							// Lifecycle methods
							CTextReader(const I<CRandomAccessDataSource>& randomAccessDataSource);
							CTextReader(const CTextReader& other);
							~CTextReader();

							// Instance methods
		UInt64				getByteCount() const;

		TVResult<CString>	readStringToEOL();

	// Properties
	private:
		Internals*	mInternals;
};
