//----------------------------------------------------------------------------------------------------------------------
//	CSQLiteOrderBy.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CSQLiteOrderBy.h"

#include "CSQLiteTable.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSQLiteOrderBy::Internals

class CSQLiteOrderBy::Internals {
	public:
		Internals() {}

		CString	mString;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSQLiteOrderBy

// MARK: Lifecycle methods
//----------------------------------------------------------------------------------------------------------------------
CSQLiteOrderBy::CSQLiteOrderBy(const CSQLiteTable& table, CSQLiteTableColumn& tableColumn, Order order)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new Internals();
	mInternals->mString =
			CString(OSSTR(" ORDER BY `")) + table.getName() + CString(OSSTR("`.`")) + tableColumn.getName() +
					CString(OSSTR("`")) + CString((order == kAscending) ? OSSTR("ASC") : OSSTR("DESC"));
}

//----------------------------------------------------------------------------------------------------------------------
CSQLiteOrderBy::CSQLiteOrderBy(CSQLiteTableColumn& tableColumn, Order order)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new Internals();
	mInternals->mString =
			CString(OSSTR(" ORDER BY `")) + tableColumn.getName() + CString(OSSTR("`")) +
					CString((order == kAscending) ? OSSTR("ASC") : OSSTR("DESC"));
}

//----------------------------------------------------------------------------------------------------------------------
CSQLiteOrderBy::~CSQLiteOrderBy()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
const CString& CSQLiteOrderBy::getString() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mString;
}
