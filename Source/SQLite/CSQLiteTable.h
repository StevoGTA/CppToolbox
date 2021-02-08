//----------------------------------------------------------------------------------------------------------------------
//	CSQLiteTable.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CSQLiteInnerJoin.h"
#include "CSQLiteOrderBy.h"
#include "CSQLiteStatementPerformer.h"
#include "CSQLiteTrigger.h"
#include "CSQLiteWhere.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSQLiteTable

class CSQLiteTableInternals;
class CSQLiteTable {
	// Options
	public:
		enum Options {
			kNone			= 0,
			kWithoutRowID	= 1 << 0,
		};

	// TableAndTableColumn
	public:
		struct TableAndTableColumn {
			// Lifecycle methods
			TableAndTableColumn(const CSQLiteTable& table, const CSQLiteTableColumn& tableColumn) :
				mTable(table), mTableColumn(tableColumn)
				{}
			TableAndTableColumn(const TableAndTableColumn& other) :
				mTable(other.mTable), mTableColumn(other.mTableColumn)
				{}

			// Properties
			const	CSQLiteTable&		mTable;
			const	CSQLiteTableColumn&	mTableColumn;
		};

	// TableColumnAndValue
	public:
		struct TableColumnAndValue {
			// Lifecycle methods
			TableColumnAndValue(const CSQLiteTableColumn& tableColumn, const SSQLiteValue& value) :
				mTableColumn(tableColumn), mValue(value)
				{}
			TableColumnAndValue(const TableColumnAndValue& other) :
				mTableColumn(other.mTableColumn), mValue(other.mValue)
				{}

			// Properites
			const	CSQLiteTableColumn&	mTableColumn;
			const	SSQLiteValue&		mValue;
		};

	// Procs
	public:
		typedef	void	(*LastInsertRowIDProc)(SInt64 lastRowID, void* userData);

	// Methods
	public:
											// Lifecycle methods
											CSQLiteTable(const CString& name, Options options,
													const TArray<CSQLiteTableColumn>& tableColumns,
													const TArray<CSQLiteTableColumn::Reference>& references,
													CSQLiteStatementPerformer& statementPerformer);
											CSQLiteTable(const CString& name, Options options,
													const TArray<CSQLiteTableColumn>& tableColumns,
													CSQLiteStatementPerformer& statementPerformer);
											~CSQLiteTable();

											// Instance methods
				const	CString&			getName() const;

				const	CSQLiteTableColumn&	getTableColumn(const CString& name) const;

						void				create(bool ifNotExists = true);
						void				rename(const CString& name);
						void				add(const CSQLiteTableColumn& tableColumn);
						void				add(const CSQLiteTrigger& trigger);
						void				drop();

						bool				hasRow(const CSQLiteWhere& where) const;
						UInt32				count(const OR<CSQLiteWhere>& where = OR<CSQLiteWhere>()) const;
						OV<SInt64>			rowID(const CSQLiteWhere& where) const;
						void				select(const OR<TArray<CSQLiteTableColumn> >& tableColumns,
													const OR<CSQLiteInnerJoin>& innerJoin,
													const OR<CSQLiteWhere>& where, const OR<CSQLiteOrderBy>& orderBy,
													CSQLiteResultsRow::Proc resultsRowProc, void* userData);
						void				select(const TArray<TableAndTableColumn>& tableAndTableColumns,
													const OR<CSQLiteInnerJoin>& innerJoin,
													const OR<CSQLiteWhere>& where, const OR<CSQLiteOrderBy>& orderBy,
													CSQLiteResultsRow::Proc resultsRowProc, void* userData);
						SInt64				insertRow(const TArray<TableColumnAndValue>& info);
						void				insertRow(const TArray<TableColumnAndValue>& info,
													LastInsertRowIDProc lastInsertRowIDProc, void* userData);
						SInt64				insertOrReplaceRow(const TArray<TableColumnAndValue>& info);
						void				insertOrReplaceRow(const TArray<TableColumnAndValue>& info,
													LastInsertRowIDProc lastInsertRowIDProc, void* userData);
						void				insertOrReplaceRows(const CSQLiteTableColumn& tableColumn,
													const TArray<SSQLiteValue>& values);
						void				update(const TArray<TableColumnAndValue>& info, const CSQLiteWhere& where);
						void				deleteRows(const CSQLiteTableColumn& tableColumn,
													const TArray<SSQLiteValue>& values);

	// Properties
	private:
		CSQLiteTableInternals*	mInternals;
};
