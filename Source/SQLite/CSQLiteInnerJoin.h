//----------------------------------------------------------------------------------------------------------------------
//	CSQLiteInnerJoin.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CSQLiteTableColumn.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSQLiteInnerJoin

class CSQLiteInnerJoinInternals;
class CSQLiteInnerJoin {
	// Methods
	public:
									// Lifecycle methods
									CSQLiteInnerJoin(const CSQLiteTable& table, const CSQLiteTableColumn& tableColumn,
											const CSQLiteTable& otherTable,
											const OR<CSQLiteTableColumn>& otherTableColumn = OR<CSQLiteTableColumn>());
									~CSQLiteInnerJoin();

									// Instance methods
		const	CString&			getString() const;

				CSQLiteInnerJoin&	addAnd(const CSQLiteTable& table, const CSQLiteTableColumn& tableColumn,
										const CSQLiteTable& otherTable,
										const OR<CSQLiteTableColumn>& otherTableColumn = OR<CSQLiteTableColumn>());

	// Properties
	private:
		CSQLiteInnerJoinInternals*	mInternals;
};
