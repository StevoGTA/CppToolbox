//----------------------------------------------------------------------------------------------------------------------
//	CSQLiteResultsRow.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CSQLiteTableColumn.h"

#include <sqlite3.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSQLiteResultsRow

class CSQLiteResultsRow {
	// Procs
	public:
		typedef	OV<SError>	(*Proc)(const CSQLiteResultsRow& resultsRow, void* userData);

	// Classes
	private:
		class Internals;

	// Methods
	public:
					// Lifecycle methods
					CSQLiteResultsRow(sqlite3_stmt* statement);
					~CSQLiteResultsRow();

					// Instance methods
		bool		moveToNext() const;

		OV<SInt64>	getInteger(const CSQLiteTableColumn& tableColumn) const;
		OV<UInt32>	getUInt32(const CSQLiteTableColumn& tableColumn) const
						{
							// Setup
							OV<SInt64>	value = getInteger(tableColumn);

							return value.hasValue() ? OV<UInt32>((UInt32) *value) : OV<UInt32>();
						}
		OV<Float64>	getReal(const CSQLiteTableColumn& tableColumn) const;
		OV<CString>	getText(const CSQLiteTableColumn& tableColumn) const;
		OV<CData>	getBlob(const CSQLiteTableColumn& tableColumn) const;

	// Properties
	private:
		Internals*	mInternals;
};
