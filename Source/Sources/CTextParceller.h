//----------------------------------------------------------------------------------------------------------------------
//	CTextParceller.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDataSource.h"
#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CTextParceller

class CTextParcellerInternals;
class CTextParceller {
	// Methods
	public:
				// Lifecycle methods
				CTextParceller(const CDataSource* dataSource);	// Will take ownership of CDataSource
				CTextParceller(const CTextParceller& other);
				~CTextParceller();

				// Instance methods
		UInt64	getSize() const;

		CString	readStringToEOL(UError& outError) const;

		void	reset() const;

	// Properties
	private:
		CTextParcellerInternals*	mInternals;
};
