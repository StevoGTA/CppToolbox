//----------------------------------------------------------------------------------------------------------------------
//	CSQLiteInnerJoin.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CSQLiteInnerJoin.h"

#include "CSQLiteTable.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSQLiteInnerJoin::Internals

class CSQLiteInnerJoin::Internals {
	public:
		Internals() {}

	CString	mString;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSQLiteInnerJoin

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CSQLiteInnerJoin::CSQLiteInnerJoin(const CSQLiteTable& table, const CSQLiteTableColumn& tableColumn,
		const CSQLiteTable& otherTable, const OR<CSQLiteTableColumn>& otherTableColumn)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new Internals();
	addAnd(table, tableColumn, otherTable, otherTableColumn);
}

//----------------------------------------------------------------------------------------------------------------------
CSQLiteInnerJoin::CSQLiteInnerJoin(const CSQLiteTable& table, const CSQLiteTableColumn& tableColumn,
		const CSQLiteTable& otherTable, const CSQLiteTableColumn& otherTableColumn)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new Internals();
	addAnd(table, tableColumn, otherTable, OR<CSQLiteTableColumn>((CSQLiteTableColumn&) otherTableColumn));
}

//----------------------------------------------------------------------------------------------------------------------
CSQLiteInnerJoin::~CSQLiteInnerJoin()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
const CString& CSQLiteInnerJoin::getString() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mString;
}

//----------------------------------------------------------------------------------------------------------------------
CSQLiteInnerJoin& CSQLiteInnerJoin::addAnd(const CSQLiteTable& table, const CSQLiteTableColumn& tableColumn,
		const CSQLiteTable& otherTable, const OR<CSQLiteTableColumn>& otherTableColumn)
//----------------------------------------------------------------------------------------------------------------------
{
	// Append
	mInternals->mString +=
			CString(OSSTR(" INNER JOIN `")) + otherTable.getName() + CString(OSSTR("` ON ")) +
					CString(OSSTR("`")) + otherTable.getName() + CString(OSSTR("`.`")) +
					(otherTableColumn.hasReference() ? otherTableColumn->getName() : tableColumn.getName()) +
					CString(OSSTR("` = `")) + table.getName() + CString(OSSTR("`.`")) + tableColumn.getName() +
					CString(OSSTR("`"));

	return *this;
}
