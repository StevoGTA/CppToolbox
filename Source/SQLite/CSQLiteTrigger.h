//----------------------------------------------------------------------------------------------------------------------
//	CSQLiteTrigger.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CSQLiteTableColumn.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSQLiteTrigger

class CSQLiteTriggerInternals;
class CSQLiteTrigger {
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
		CSQLiteTriggerInternals*	mInternals;
};
