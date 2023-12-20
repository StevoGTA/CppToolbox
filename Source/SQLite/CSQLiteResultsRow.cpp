//----------------------------------------------------------------------------------------------------------------------
//	CSQLiteResultsRow.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CSQLiteResultsRow.h"

#include "CDictionary.h"
#include "CLogServices.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSQLiteResultsRow::Internals

class CSQLiteResultsRow::Internals {
	public:
		Internals(sqlite3_stmt* statement) :
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
	mInternals = new Internals(statement);
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
OV<SInt64> CSQLiteResultsRow::getInteger(const CSQLiteTableColumn& tableColumn) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	const	CString&	name = tableColumn.getName();
	AssertFailIf(tableColumn.getKind() != CSQLiteTableColumn::kKindInteger);

	SInt32	index = mInternals->mColumnNameMap.getSInt32(name, -1);
	AssertFailIf(index == -1);

	return (sqlite3_column_type(mInternals->mStatement, index) != SQLITE_NULL) ?
			OV<SInt64>(sqlite3_column_int64(mInternals->mStatement, index)) : OV<SInt64>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<Float64> CSQLiteResultsRow::getReal(const CSQLiteTableColumn& tableColumn) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	const	CString&	name = tableColumn.getName();
	AssertFailIf(tableColumn.getKind() != CSQLiteTableColumn::kKindReal);

	SInt32	index = mInternals->mColumnNameMap.getSInt32(name, -1);
	AssertFailIf(index == -1);

	return (sqlite3_column_type(mInternals->mStatement, index) != SQLITE_NULL) ?
			OV<Float64>(sqlite3_column_double(mInternals->mStatement, index)) : OV<Float64>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<CString> CSQLiteResultsRow::getText(const CSQLiteTableColumn& tableColumn) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	const	CString&	name = tableColumn.getName();
	AssertFailIf(tableColumn.getKind() != CSQLiteTableColumn::kKindText);

	SInt32	index = mInternals->mColumnNameMap.getSInt32(name, -1);
	AssertFailIf(index == -1);

	// Get value
	const	unsigned	char*	text = sqlite3_column_text(mInternals->mStatement, index);

	return (text != nil) ? OV<CString>(CString((const char*) text)) : OV<CString>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<CData> CSQLiteResultsRow::getData(const CSQLiteTableColumn& tableColumn) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	const	CString&	name = tableColumn.getName();
	AssertFailIf(tableColumn.getKind() != CSQLiteTableColumn::kKindBlob);

	SInt32	index = mInternals->mColumnNameMap.getSInt32(name, -1);
	AssertFailIf(index == -1);

	// Get value
	const	void*	blob = sqlite3_column_blob(mInternals->mStatement, index);

	return (blob != nil) ?
			OV<CData>(CData(blob, sqlite3_column_bytes(mInternals->mStatement, index))) : OV<CData>();
}
