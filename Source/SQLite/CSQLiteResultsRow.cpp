//----------------------------------------------------------------------------------------------------------------------
//	CSQLiteResultsRow.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CSQLiteResultsRow.h"

#include "CDictionary.h"
#include "CLogServices.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSQLiteResultsRowInternals

class CSQLiteResultsRowInternals {
	public:
		CSQLiteResultsRowInternals(sqlite3_stmt* statement) :
			mStatement(statement)
			{
				// Setup column name map
				for (int i = 0; i < sqlite3_column_count(mStatement); i++) {
					// Add to map
					CString	columnName(sqlite3_column_name(mStatement, i));
					mColumnNameMap.set(columnName, i);
				}
			}

		sqlite3_stmt*	mStatement;
		CDictionary		mColumnNameMap;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSQLiteResultsRow

// MARK: Lifecycle methods
//----------------------------------------------------------------------------------------------------------------------
CSQLiteResultsRow::CSQLiteResultsRow(sqlite3_stmt* statement)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CSQLiteResultsRowInternals(statement);
}

//----------------------------------------------------------------------------------------------------------------------
CSQLiteResultsRow::~CSQLiteResultsRow()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
bool CSQLiteResultsRow::moveToNext() const
//----------------------------------------------------------------------------------------------------------------------
{
	return sqlite3_step(mInternals->mStatement) == SQLITE_ROW;
}

//----------------------------------------------------------------------------------------------------------------------
OV<SInt64> CSQLiteResultsRow::getSInt64(const CSQLiteTableColumn& tableColumn) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	const	CString&	name = tableColumn.getName();
	AssertFailIf(tableColumn.getKind() != CSQLiteTableColumn::kInteger);

	SInt32	index = mInternals->mColumnNameMap.getSInt32(name, -1);
	AssertFailIf(index == -1);

	return (sqlite3_column_type(mInternals->mStatement, index) != SQLITE_NULL) ?
			OV<SInt64>(sqlite3_column_int64(mInternals->mStatement, index)) : OV<SInt64>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<Float64> CSQLiteResultsRow::getFloat64(const CSQLiteTableColumn& tableColumn) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	const	CString&	name = tableColumn.getName();
	AssertFailIf(tableColumn.getKind() != CSQLiteTableColumn::kReal);

	SInt32	index = mInternals->mColumnNameMap.getSInt32(name, -1);
	AssertFailIf(index == -1);

	return (sqlite3_column_type(mInternals->mStatement, index) != SQLITE_NULL) ?
			OV<Float64>(sqlite3_column_double(mInternals->mStatement, index)) : OV<Float64>();
}

//----------------------------------------------------------------------------------------------------------------------
OI<CString> CSQLiteResultsRow::getString(const CSQLiteTableColumn& tableColumn) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	const	CString&	name = tableColumn.getName();
	AssertFailIf(tableColumn.getKind() != CSQLiteTableColumn::kText);

	SInt32	index = mInternals->mColumnNameMap.getSInt32(name, -1);
	AssertFailIf(index == -1);

	// Get value
	const	unsigned	char*	text = sqlite3_column_text(mInternals->mStatement, index);

	return (text != nil) ? OI<CString>(new CString((const char*) text)) : OI<CString>();
}

//----------------------------------------------------------------------------------------------------------------------
OI<CData> CSQLiteResultsRow::getData(const CSQLiteTableColumn& tableColumn) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	const	CString&	name = tableColumn.getName();
	AssertFailIf(tableColumn.getKind() != CSQLiteTableColumn::kBlob);

	SInt32	index = mInternals->mColumnNameMap.getSInt32(name, -1);
	AssertFailIf(index == -1);

	// Get value
	const	void*	blob = sqlite3_column_blob(mInternals->mStatement, index);

	return (blob != nil) ?
			OI<CData>(new CData(blob, sqlite3_column_bytes(mInternals->mStatement, index))) : OI<CData>();
}
