//----------------------------------------------------------------------------------------------------------------------
//	CDotUnderscoreReader.h			©2022 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDataSource.h"
#include "CDictionary.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CDotUnderscoreReader

class CDotUnderscoreAttribute;
class CDotUnderscoreReaderInternals;
class CDotUnderscoreReader {
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
														const TNDictionary<TNArray<CDotUnderscoreAttribute> >&
																attributeMap,
														const OI<CData>& resourceFork);

	// Properties
	private:
		CDotUnderscoreReaderInternals*	mInternals;
};
