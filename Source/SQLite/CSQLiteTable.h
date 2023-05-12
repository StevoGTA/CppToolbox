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
			TableColumnAndValue(const CSQLiteTableColumn& tableColumn, const CData& data) :
				mTableColumn(tableColumn), mValue(data)
				{}
			TableColumnAndValue(const CSQLiteTableColumn& tableColumn, Float64 float64) :
				mTableColumn(tableColumn), mValue(float64)
				{}
			TableColumnAndValue(const CSQLiteTableColumn& tableColumn, SInt64 sInt64) :
				mTableColumn(tableColumn), mValue(sInt64)
				{}
			TableColumnAndValue(const CSQLiteTableColumn& tableColumn, UInt32 uInt32) :
				mTableColumn(tableColumn), mValue(uInt32)
				{}
			TableColumnAndValue(const CSQLiteTableColumn& tableColumn, const CString& string) :
				mTableColumn(tableColumn), mValue(string)
				{}
			TableColumnAndValue(const TableColumnAndValue& other) :
				mTableColumn(other.mTableColumn), mValue(other.mValue)
				{}

			// Properites
			const	CSQLiteTableColumn&	mTableColumn;
					SSQLiteValue		mValue;
		};

	// Procs
	public:
		typedef	void	(*LastInsertRowIDProc)(SInt64 lastRowID, void* userData);

	// Classes
	private:
		class Internals;

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
									CSQLiteTable(const CSQLiteTable& other);
									~CSQLiteTable();

									// Instance methods
		const	CString&			getName() const;

				void				create(bool ifNotExists = true) const;
//				void				rename(const CString& name) const;
//				void				add(const CSQLiteTableColumn& tableColumn);
//				void				add(const CSQLiteTrigger& trigger);
//				void				drop(const CString& triggerName);
				void				drop() const;

//				bool				hasRow(const CSQLiteWhere& where) const;
				UInt32				count(const OR<CSQLiteInnerJoin>& innerJoin = OR<CSQLiteInnerJoin>(),
											const OR<CSQLiteWhere>& where = OR<CSQLiteWhere>()) const;
//				OV<SInt64>			rowID(const CSQLiteWhere& where) const;

				void				select(const CSQLiteTableColumn tableColumns[], UInt32 count,
											const OR<CSQLiteInnerJoin>& innerJoin, const OR<CSQLiteWhere>& where,
											const OR<CSQLiteOrderBy>& orderBy, CSQLiteResultsRow::Proc resultsRowProc,
											void* userData) const;
				void				select(const CSQLiteTableColumn tableColumns[], UInt32 count,
											const CSQLiteWhere& where, CSQLiteResultsRow::Proc resultsRowProc,
											void* userData) const
										{ select(tableColumns, count, OR<CSQLiteInnerJoin>(),
												OR<CSQLiteWhere>((CSQLiteWhere&) where), OR<CSQLiteOrderBy>(),
												resultsRowProc, userData); }
				void				select(const CSQLiteInnerJoin& innerJoin, const CSQLiteWhere& where,
											CSQLiteResultsRow::Proc resultsRowProc, void* userData) const
										{
											// Setup
											CSQLiteTableColumn	tableColumns[] = {mSelectAllTableColumn};

											// Select
											select(tableColumns, 1, OR<CSQLiteInnerJoin>((CSQLiteInnerJoin&) innerJoin),
													OR<CSQLiteWhere>((CSQLiteWhere&) where), OR<CSQLiteOrderBy>(),
													resultsRowProc, userData);
										}
				void				select(const CSQLiteInnerJoin& innerJoin, CSQLiteResultsRow::Proc resultsRowProc,
											void* userData) const
										{
											// Setup
											CSQLiteTableColumn	tableColumns[] = {mSelectAllTableColumn};

											// Select
											select(tableColumns, 1, OR<CSQLiteInnerJoin>((CSQLiteInnerJoin&) innerJoin),
													OR<CSQLiteWhere>(), OR<CSQLiteOrderBy>(), resultsRowProc, userData);
										}
				void				select(CSQLiteResultsRow::Proc resultsRowProc, void* userData) const
										{
											// Setup
											CSQLiteTableColumn	tableColumns[] = {mSelectAllTableColumn};

											// Select
											select(tableColumns, 1, OR<CSQLiteInnerJoin>(), OR<CSQLiteWhere>(),
													OR<CSQLiteOrderBy>(), resultsRowProc, userData);
										}

//				void				select(const TArray<TableAndTableColumn>& tableAndTableColumns,
//											const OR<CSQLiteInnerJoin>& innerJoin, const OR<CSQLiteWhere>& where,
//											const OR<CSQLiteOrderBy>& orderBy, CSQLiteResultsRow::Proc resultsRowProc,
//											void* userData) const;

				SInt64				insertRow(const TableColumnAndValue tableColumnAndValues[], UInt32 count) const;
				void				insertRow(const TableColumnAndValue tableColumnAndValues[], UInt32 count,
											LastInsertRowIDProc lastInsertRowIDProc, void* userData) const;

				SInt64				insertOrReplaceRow(const TableColumnAndValue tableColumnAndValues[], UInt32 count)
											const;
				void				insertOrReplaceRow(const TableColumnAndValue tableColumnAndValues[], UInt32 count,
											LastInsertRowIDProc lastInsertRowIDProc, void* userData) const;
				void				insertOrReplaceRows(const CSQLiteTableColumn& tableColumn,
											const TArray<SSQLiteValue>& values) const;

				void				update(const TableColumnAndValue tableColumnAndValues[], UInt32 count,
											const CSQLiteWhere& where) const;
				void				update(const TableColumnAndValue& tableColumnAndValue, const CSQLiteWhere& where)
											const
										{ update(&tableColumnAndValue, 1, where); }

				void				deleteRows(const CSQLiteTableColumn& tableColumn,
											const TArray<SSQLiteValue>& values) const;
				void				deleteRows(const CSQLiteTableColumn& tableColumn, const SSQLiteValue& value) const
										{ deleteRows(tableColumn, TNArray<SSQLiteValue>(value)); }

	// Properties
	public:
		static	const	CSQLiteTableColumn	mSelectAllTableColumn;

	private:
						Internals*			mInternals;
};
