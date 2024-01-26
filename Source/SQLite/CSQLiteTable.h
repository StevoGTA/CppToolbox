//----------------------------------------------------------------------------------------------------------------------
//	CSQLiteTable.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CSQLiteInnerJoin.h"
#include "CSQLiteLimit.h"
#include "CSQLiteOrderBy.h"
#include "CSQLiteStatementPerformer.h"
#include "CSQLiteTrigger.h"
#include "CSQLiteWhere.h"
#include "TResult.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSQLiteTable

class CSQLiteTable {
	// Options
	public:
		enum Options {
			kOptionsNone			= 0,
			kOptionsWithoutRowID	= 1 << 0,
		};

	// TableAndTableColumn
	public:
		struct TableAndTableColumn {
			// Methods
			public:
										// Lifecycle methods
										TableAndTableColumn(const CSQLiteTable& table,
												const CSQLiteTableColumn& tableColumn) :
											mTable(table), mTableColumn(tableColumn)
											{}
										TableAndTableColumn(const TableAndTableColumn& other) :
											mTable(other.mTable), mTableColumn(other.mTableColumn)
											{}

										// Instance methods
			const	CSQLiteTable&		getTable() const
											{ return mTable; }
			const	CSQLiteTableColumn&	getTableColumn() const
											{ return mTableColumn; }

			// Properties
			private:
				const	CSQLiteTable&		mTable;
				const	CSQLiteTableColumn&	mTableColumn;
		};

	// TableColumnAndValue
	public:
		struct TableColumnAndValue {
			// Methods
			public:
											// Lifecycle methods
											TableColumnAndValue(const CSQLiteTableColumn& tableColumn,
													const SSQLiteValue& value) :
												mTableColumn(tableColumn), mValue(value)
												{}
											TableColumnAndValue(const CSQLiteTableColumn& tableColumn,
													const CData& data) :
												mTableColumn(tableColumn), mValue(data)
												{}
											TableColumnAndValue(const CSQLiteTableColumn& tableColumn,
													Float64 float64) :
												mTableColumn(tableColumn), mValue(float64)
												{}
											TableColumnAndValue(const CSQLiteTableColumn& tableColumn,
													SInt64 sInt64) :
												mTableColumn(tableColumn), mValue(sInt64)
												{}
											TableColumnAndValue(const CSQLiteTableColumn& tableColumn,
													UInt32 uInt32) :
												mTableColumn(tableColumn), mValue(uInt32)
												{}
											TableColumnAndValue(const CSQLiteTableColumn& tableColumn,
													const CString& string) :
												mTableColumn(tableColumn), mValue(string)
												{}
											TableColumnAndValue(const TableColumnAndValue& other) :
												mTableColumn(other.mTableColumn), mValue(other.mValue)
												{}

											// Instance methods
				const	CSQLiteTableColumn&	getTableColumn() const
												{ return mTableColumn; }
				const	SSQLiteValue&		getValue() const
												{ return mValue; }
			// Properites
			private:
				const	CSQLiteTableColumn&	mTableColumn;
						SSQLiteValue		mValue;
		};

	// Procs
	public:
		typedef	void									(*LastInsertRowIDProc)(SInt64 lastRowID, void* userData);
		typedef	TVResult<TArray<TableColumnAndValue> >	(*ResultsRowMigrationProc)(const CSQLiteResultsRow& resultsRow,
																void* userData);

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
													CSQLiteTable(const CSQLiteTable& other);
													~CSQLiteTable();

													// Instance methods
				const	CString&					getName() const;

						SInt32						getVariableNumberLimit() const;

						void						add(const CSQLiteTableColumn& tableColumn);
						void						add(const CSQLiteTrigger& trigger);

						UInt32						count(
															const OR<CSQLiteInnerJoin>& innerJoin =
																	OR<CSQLiteInnerJoin>(),
															const OR<CSQLiteWhere>& where = OR<CSQLiteWhere>()) const;
						UInt32						count(const CSQLiteWhere& where) const
														{ return count(OR<CSQLiteInnerJoin>(),
																OR<CSQLiteWhere>((CSQLiteWhere&) where)); }

						void						create(bool ifNotExists = true) const;

						void						deleteRow(const CSQLiteWhere& where);
						void						deleteRows(const CSQLiteTableColumn& tableColumn,
															const TArray<SSQLiteValue>& values) const;
						void						deleteRows(const CSQLiteTableColumn& tableColumn,
															const SSQLiteValue& value) const
														{ deleteRows(tableColumn, TSArray<SSQLiteValue>(value)); }

						void						drop(const CString& triggerName);
						void						drop() const;

						bool						hasRow(const CSQLiteWhere& where) const;

						SInt64						insertRow(const TArray<TableColumnAndValue>& tableColumnAndValues)
															const;
						void						insertRow(const TArray<TableColumnAndValue>& tableColumnAndValues,
															LastInsertRowIDProc lastInsertRowIDProc, void* userData)
															const;

						SInt64						insertOrReplaceRow(
															const TArray<TableColumnAndValue>& tableColumnAndValues)
															const;
						void						insertOrReplaceRow(
															const TArray<TableColumnAndValue>& tableColumnAndValues,
															LastInsertRowIDProc lastInsertRowIDProc, void* userData)
															const;
						void						insertOrReplaceRows(const CSQLiteTableColumn& tableColumn,
															const TArray<SSQLiteValue>& values) const;

						OV<SError>					migrate(ResultsRowMigrationProc resultsRowMigrationProc,
															void* userData = nil);

						void						rename(const CString& name) const;

						OV<SInt64>					rowID(const CSQLiteWhere& where) const;

						OV<SError>					select(const TArray<CSQLiteTableColumn>& tableColumns,
															const OR<CSQLiteInnerJoin>& innerJoin,
															const OR<CSQLiteWhere>& where,
															const OR<CSQLiteOrderBy>& orderBy,
															const OR<CSQLiteLimit>& limit,
															CSQLiteResultsRow::Proc resultsRowProc, void* userData)
															const;
						OV<SError>					select(const TArray<CSQLiteTableColumn>& tableColumns,
															const CSQLiteInnerJoin& innerJoin,
															const CSQLiteWhere& where,
															const CSQLiteOrderBy& orderBy,
															const CSQLiteLimit& limit,
															CSQLiteResultsRow::Proc resultsRowProc, void* userData)
															const
														{ return select(tableColumns,
																OR<CSQLiteInnerJoin>((CSQLiteInnerJoin&) innerJoin),
																OR<CSQLiteWhere>((CSQLiteWhere&) where),
																OR<CSQLiteOrderBy>((CSQLiteOrderBy&) orderBy),
																OR<CSQLiteLimit>((CSQLiteLimit&) limit), resultsRowProc,
																userData); }
						OV<SError>					select(const TArray<CSQLiteTableColumn>& tableColumns,
															const CSQLiteWhere& where,
															CSQLiteResultsRow::Proc resultsRowProc, void* userData)
															const
														{ return select(tableColumns, OR<CSQLiteInnerJoin>(),
																OR<CSQLiteWhere>((CSQLiteWhere&) where),
																OR<CSQLiteOrderBy>(), OR<CSQLiteLimit>(),
																resultsRowProc, userData); }
						OV<SError>					select(const CSQLiteInnerJoin& innerJoin, const CSQLiteWhere& where,
															CSQLiteResultsRow::Proc resultsRowProc, void* userData)
															const
														{ return select(
																TSArray<CSQLiteTableColumn>(mSelectAllTableColumn),
																OR<CSQLiteInnerJoin>((CSQLiteInnerJoin&) innerJoin),
																OR<CSQLiteWhere>((CSQLiteWhere&) where),
																OR<CSQLiteOrderBy>(), OR<CSQLiteLimit>(),
																resultsRowProc, userData); }
						OV<SError>					select(const CSQLiteInnerJoin& innerJoin,
															CSQLiteResultsRow::Proc resultsRowProc, void* userData)
															const
														{ return select(
																TSArray<CSQLiteTableColumn>(mSelectAllTableColumn),
																OR<CSQLiteInnerJoin>((CSQLiteInnerJoin&) innerJoin),
																OR<CSQLiteWhere>(), OR<CSQLiteOrderBy>(),
																OR<CSQLiteLimit>(), resultsRowProc, userData); }
						OV<SError>					select(const CSQLiteWhere& where,
															CSQLiteResultsRow::Proc resultsRowProc, void* userData) const
														{ return select(TSArray<CSQLiteTableColumn>(mSelectAllTableColumn),
																OR<CSQLiteInnerJoin>(),
																OR<CSQLiteWhere>((CSQLiteWhere&) where),
																OR<CSQLiteOrderBy>(), OR<CSQLiteLimit>(),
																resultsRowProc, userData); }
						OV<SError>					select(CSQLiteResultsRow::Proc resultsRowProc, void* userData)
															const
														{ return select(
																TSArray<CSQLiteTableColumn>(mSelectAllTableColumn),
																OR<CSQLiteInnerJoin>(), OR<CSQLiteWhere>(),
																OR<CSQLiteOrderBy>(), OR<CSQLiteLimit>(),
																resultsRowProc, userData); }
						OV<SError>					select(const TArray<TableAndTableColumn>& tableAndTableColumns,
															const OR<CSQLiteInnerJoin>& innerJoin,
															const OR<CSQLiteWhere>& where,
															const OR<CSQLiteOrderBy>& orderBy,
															const OR<CSQLiteLimit>& limit,
															CSQLiteResultsRow::Proc resultsRowProc, void* userData)
															const;

						TVResult<SInt64>			sum(const CSQLiteTableColumn& tableColumn,
															const OR<CSQLiteInnerJoin>& innerJoin,
															const OR<CSQLiteWhere>& where) const;
						TVResult<CDictionary>		sum(const TArray<CSQLiteTableColumn>& tableColumns,
															const OR<CSQLiteInnerJoin>& innerJoin,
															const OR<CSQLiteWhere>& where) const;
						TVResult<CDictionary>		sum(const TArray<CSQLiteTableColumn>& tableColumns,
															const CSQLiteInnerJoin& innerJoin,
															const CSQLiteWhere& where) const
														{ return sum(tableColumns,
																OR<CSQLiteInnerJoin>((CSQLiteInnerJoin&) innerJoin),
																OR<CSQLiteWhere>((CSQLiteWhere&) where)); }

				const	CSQLiteTableColumn&			getTableColumn(const CString& name) const;
						TArray<CSQLiteTableColumn>	getTableColumns(const TArray<CString>& names) const;

						void						update(const TArray<TableColumnAndValue>& tableColumnAndValues,
															const CSQLiteWhere& where) const;
						void						update(const TableColumnAndValue& tableColumnAndValue,
															const CSQLiteWhere& where) const
														{ update(TSArray<TableColumnAndValue>(tableColumnAndValue),
																where); }

		static			TArray<CSQLiteTable>		getAll(CSQLiteStatementPerformer& statementPerformer);

	private:
													// Lifecycle methods
													CSQLiteTable(const CString& name,
															CSQLiteStatementPerformer& statementPerformer);
	// Properties
	public:
		static	const	CSQLiteTableColumn	mSelectAllTableColumn;

	private:
						Internals*			mInternals;
};
