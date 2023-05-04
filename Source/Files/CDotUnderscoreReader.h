//----------------------------------------------------------------------------------------------------------------------
//	CDotUnderscoreReader.h			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDataSource.h"
#include "CDictionary.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CDotUnderscoreReader

class CDotUnderscoreReader {
	// Classes
	private:
		class Attribute;
		class Internals;

	// Methods
	public:
												// Lifecycle methods
												~CDotUnderscoreReader();

												// Instance methods
				OR<CData>						getResourceFork() const;

												// Class methods
		static	TIResult<CDotUnderscoreReader>	from(const I<CRandomAccessDataSource>& randomAccessDataSource);

	private:
												// Lifecycle methods
												CDotUnderscoreReader(
														const TNDictionary<TNArray<Attribute> >& attributeMap,
														const OV<CData>& resourceFork);

	// Properties
	private:
		Internals*	mInternals;
};
