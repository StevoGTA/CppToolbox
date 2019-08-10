//----------------------------------------------------------------------------------------------------------------------
//	CStringSource.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDataProvider.h"
#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CStringSource

class CStringSourceInternals;
class CStringSource {
	// Methods
	public:
				// Lifecycle methods
				CStringSource(const CDataProvider* dataProvider);	// Will take ownership of CDataProvider
				CStringSource(const CStringSource& other);
				~CStringSource();

				// Instance methods
		UInt64	getSize() const;

		CString	readStringToEOL(UError& outError) const;

		void	reset() const;

	// Properties
	private:
		CStringSourceInternals*	mInternals;
};
