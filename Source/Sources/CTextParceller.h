//----------------------------------------------------------------------------------------------------------------------
//	CTextParceller.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDataSource.h"
#include "CString.h"
#include "TInstance.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CTextParceller

class CTextParcellerInternals;
class CTextParceller {
	// Methods
	public:
				// Lifecycle methods
				CTextParceller(const I<CDataSource>& dataSource);
				CTextParceller(const CTextParceller& other);
				~CTextParceller();

				// Instance methods
		UInt64	getSize() const;

		CString	readStringToEOL(UError& outError);

		void	reset() const;

	// Properties
	private:
		CTextParcellerInternals*	mInternals;
};
