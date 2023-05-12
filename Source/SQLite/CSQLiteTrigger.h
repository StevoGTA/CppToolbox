//----------------------------------------------------------------------------------------------------------------------
//	CSQLiteTrigger.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CSQLiteTableColumn.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSQLiteTrigger

class CSQLiteTrigger {
	// Classes
	private:
		class Internals;

	// Methods
	public:
				// Lifecycle methods
				CSQLiteTrigger(const CSQLiteTableColumn& updateTableColumn,
						const CSQLiteTableColumn& comparisonTableColumn);
				~CSQLiteTrigger();

				// Instance methods
		CString	getString(const CSQLiteTable& table) const;

	// Properties
	private:
		Internals*	mInternals;
};
