//----------------------------------------------------------------------------------------------------------------------
//	CSQLiteResultsRow.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CSQLiteTableColumn.h"

#include <sqlite3.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSQLiteResultsRow

class CSQLiteResultsRowInternals;
class CSQLiteResultsRow {
	// Procs
	public:
		typedef	void	(*Proc)(const CSQLiteResultsRow& resultsRow, void* userData);

	// Methods
	public:
					// Lifecycle methods
					CSQLiteResultsRow(sqlite3_stmt* statement);
					~CSQLiteResultsRow();

					// Instance methods
		bool		moveToNext() const;

		OV<SInt64>	getSInt64(const CSQLiteTableColumn& tableColumn) const;
		OV<UInt32>	getUInt32(const CSQLiteTableColumn& tableColumn) const
						{
							// Setup
							OV<SInt64>	value = getSInt64(tableColumn);

							return value.hasValue() ? OV<UInt32>((UInt32) *value) : OV<UInt32>();
						}
		OV<Float64>	getFloat64(const CSQLiteTableColumn& tableColumn) const;
		OV<CString>	getString(const CSQLiteTableColumn& tableColumn) const;
		OV<CData>	getData(const CSQLiteTableColumn& tableColumn) const;

	// Properties
	private:
		CSQLiteResultsRowInternals*	mInternals;
};
