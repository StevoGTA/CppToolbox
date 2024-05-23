//----------------------------------------------------------------------------------------------------------------------
//	CSQLiteTable.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CSQLiteTable.h"

#include "CDictionary.h"
#include "CReferenceCountable.h"
#include "CUUID.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSQLiteTable::Internals

class CSQLiteTable::Internals : public TReferenceCountableAutoDelete<Internals> {
	private:
		typedef	CSQLiteStatementPerformer::TransactionResult	TransactionResult;

	public:
		struct SInt64Result {
			public:
											SInt64Result(const CSQLiteTableColumn& tableColumn) :
													mTableColumn(tableColumn)
													{}

						const	OV<SInt64>	getValue() const
												{ return mValue; }

				static			OV<SError>	process(const CSQLiteResultsRow& resultsRow, SInt64Result* int64Result)
												{
													// Store value
													int64Result->mValue =
															resultsRow.getInteger(int64Result->mTableColumn);

													return OV<SError>();
												}

			private:
				const	CSQLiteTableColumn&	mTableColumn;
						OV<SInt64>			mValue;
		};

		struct SInt64ResultByTableColumnName {
			public:
												SInt64ResultByTableColumnName(
														const TDictionary<CSQLiteTableColumn>&
																sumTableColumnsByTableColumnName) :
													mSumTableColumnsByTableColumnName(
															sumTableColumnsByTableColumnName)
													{}

						const	CDictionary&	getResultByTableColumnName() const
													{ return mResultByTableColumnName; }

				static			OV<SError>		process(const CSQLiteResultsRow& resultsRow,
														SInt64ResultByTableColumnName* int64ResultByTableColumnName)
													{
														// Iterate table columns
														const	TSet<CString>	sumTableColumnNames =
																						int64ResultByTableColumnName->
																								mSumTableColumnsByTableColumnName
																								.getKeys();
														for (TIteratorS<CString> iterator =
																		sumTableColumnNames.getIterator();
																iterator.hasValue(); iterator.advance())
															// Store value
															int64ResultByTableColumnName->mResultByTableColumnName
																	.set(*iterator,
																			*resultsRow.getInteger(
																					*int64ResultByTableColumnName->
																							mSumTableColumnsByTableColumnName[
																									*iterator]));

														return OV<SError>();
													}

			private:
				const	TDictionary<CSQLiteTableColumn>&	mSumTableColumnsByTableColumnName;
						CDictionary							mResultByTableColumnName;
		};

		struct MigrationInfo {
			public:
				typedef	TVResult<TArray<TableColumnAndValue> >	RowResult;

			public:
													MigrationInfo(const CString& statement, const CSQLiteTable& table,
															ResultsRowMigrationProc resultsRowMigrationProc,
															void* userData) :
														mStatement(statement), mTable(table),
																mResultsRowMigrationProc(resultsRowMigrationProc),
																mUserData(userData)
														{}

						const	CSQLiteTable&		getTable() const
														{ return mTable; }

						const	OV<SError>&			getError() const
														{ return mError; }

				static			TransactionResult	perform(MigrationInfo* migrationInfo)
														{
															// Iterate all existing rows
															migrationInfo->getTable().select(
																	(CSQLiteResultsRow::Proc) process, migrationInfo);

															return !migrationInfo->getError().hasValue() ?
																	CSQLiteStatementPerformer::
																			kTransactionResultCommit :
																	CSQLiteStatementPerformer::
																			kTransactionResultRollback;
														}

			private:
				static			void				process(const CSQLiteResultsRow& resultsRow,
															MigrationInfo* migrationInfo)
														{
															// Punt if error
															ReturnIfError(migrationInfo->getError());

															// Migrate this row
															RowResult	rowResult =
																				migrationInfo->mResultsRowMigrationProc(
																						resultsRow,
																						migrationInfo->mUserData);

															// Handle result
															if (rowResult.hasValue())
																// Insert
																migrationInfo->mTable.mInternals->mStatementPerformer
																		.addToTransactionOrPerform(
																				migrationInfo->mStatement,
																				migrationInfo->mTable.mInternals->
																						getValues(*rowResult));
															else
																// Error
																migrationInfo->mError.setValue(rowResult.getError());
														}

			private:
				const	CString&				mStatement;
				const	CSQLiteTable&			mTable;
						ResultsRowMigrationProc	mResultsRowMigrationProc;
						void*					mUserData;

						OV<SError>				mError;
		};

								Internals(const CString& name, CSQLiteTable::Options options,
										const TArray<CSQLiteTableColumn>& tableColumns,
										const TArray<CSQLiteTableColumn::Reference>& references,
										CSQLiteStatementPerformer& statementPerformer) :
									TReferenceCountableAutoDelete(),
											mName(name), mOptions(options), mTableColumns(tableColumns),
											mStatementPerformer(statementPerformer)
									{
										// Iterate all table columns
										for (TIteratorD<CSQLiteTableColumn> iterator = tableColumns.getIterator();
												iterator.hasValue(); iterator.advance())
											// Add to map
											mTableColumnReferenceByName.set(iterator->getName(), *iterator);

										// Iterate all table column references
										for (TIteratorD<CSQLiteTableColumn::Reference> iterator =
														references.getIterator();
												iterator.hasValue(); iterator.advance())
											// Add to map
											mTableColumnReferenceMap.set(iterator->getTableColumn().getName(),
													*iterator);
									}

		CString					getCreateString(const CSQLiteTableColumn& tableColumn)
									{
										// Compose column string
										CString	string = tableColumn.getName() + CString::mSpace;

										switch (tableColumn.getKind()) {
											case CSQLiteTableColumn::kKindInteger:
												// Integer
												string += CString(OSSTR("INTEGER"));
												break;

											case CSQLiteTableColumn::kKindReal:
												// Real
												string += CString(OSSTR("REAL"));
												break;

											case CSQLiteTableColumn::kKindText:
											case CSQLiteTableColumn::kKindDateISO8601FractionalSecondsAutoSet:
											case CSQLiteTableColumn::kKindDateISO8601FractionalSecondsAutoUpdate:
												// Text
												string += CString(OSSTR("TEXT"));
												break;

											case CSQLiteTableColumn::kKindBlob:
												// Blob
												string += CString(OSSTR("BLOB"));
												break;
										}

										// Handle options
										if (tableColumn.getOptions() & CSQLiteTableColumn::kOptionsPrimaryKey)
											string += CString(OSSTR(" PRIMARY KEY"));
										if (tableColumn.getOptions() & CSQLiteTableColumn::kOptionsAutoIncrement)
											string += CString(OSSTR(" AUTOINCREMENT"));
										if (tableColumn.getOptions() & CSQLiteTableColumn::kOptionsNotNull)
											string += CString(OSSTR(" NOT NULL"));
										if (tableColumn.getOptions() & CSQLiteTableColumn::kOptionsUnique)
											string += CString(OSSTR(" UNIQUE"));
										if (tableColumn.getOptions() & CSQLiteTableColumn::kOptionsCheck)
											string += CString(OSSTR(" CHECK"));

										if (tableColumn.getDefaultValue().hasValue())
											// Default
											string +=
													CString(OSSTR(" DEFAULT (")) +
															tableColumn.getDefaultValue()->getString() +
															CString(OSSTR(")"));

										return string;
									}
		CString					getColumnNames(const TArray<CSQLiteTableColumn>& tableColumns)
									{
										// Setup
										TNArray<CString>	strings;
										for (TIteratorD<CSQLiteTableColumn> iterator = tableColumns.getIterator();
												iterator.hasValue(); iterator.advance()) {
											// Add string
											if ((*iterator == CSQLiteTableColumn::mAll) ||
													(*iterator == CSQLiteTableColumn::mRowID))
												// Non-complex
												strings += iterator->getName();
											else
												// Escape
												strings +=
															CString(OSSTR("`")) + iterator->getName() +
																	CString(OSSTR("`"));
										}

										return CString(strings, CString(OSSTR(",")));
									}
		CString					getColumnNames(const TArray<TableColumnAndValue>& tableColumnAndValues)
									{
										// Setup
										TNArray<CString>	strings;
										for (TIteratorD<TableColumnAndValue> iterator =
														tableColumnAndValues.getIterator();
												iterator.hasValue(); iterator.advance())
											// Add string
											strings +=
													CString(OSSTR("`")) + iterator->getTableColumn().getName() +
															CString(OSSTR("`"));

										return CString(strings, CString(OSSTR(",")));
									}
		CString					getColumnNames(const TArray<CSQLiteTable::TableAndTableColumn>& tableAndTableColumns)
									{
										// Setup
										TNArray<CString>	strings;
										for (TIteratorD<CSQLiteTable::TableAndTableColumn> iterator =
														tableAndTableColumns.getIterator();
												iterator.hasValue(); iterator.advance())
											// Add string
											strings +=
													CString(OSSTR("`")) + iterator->getTable().getName() +
															CString(OSSTR("`.`")) +
															iterator->getTableColumn().getName() +
															CString(OSSTR("`"));

										return CString(strings, CString(OSSTR(",")));
									}
		TArray<SSQLiteValue>	getValues(const TArray<TableColumnAndValue>& tableColumnAndValues)
									{
										// Collect values
										TNArray<SSQLiteValue>	values;
										for (TIteratorD<TableColumnAndValue> iterator =
														tableColumnAndValues.getIterator();
												iterator.hasValue(); iterator.advance())
											// Add value
											values += iterator->getValue();

										return values;
									}
		OV<SError>				select(const CString& columnNames, const OR<CSQLiteInnerJoin>& innerJoin,
										const OR<CSQLiteWhere>& where, const OR<CSQLiteOrderBy>& orderBy,
										const OR<CSQLiteLimit>& limit, CSQLiteResultsRow::Proc resultsRowProc,
										void* userData)
									{
										// Check if we have CSQLiteWhere
										if (where.hasReference()) {
											// Iterate all groups in CSQLiteWhere
											SInt32								variableNumberLimit =
																						mStatementPerformer
																								.getVariableNumberLimit();
											TArray<CSQLiteWhere::ValueGroup>	valueGroups =
																						where->getValueGroups(
																								variableNumberLimit);
											for (TIteratorD<CSQLiteWhere::ValueGroup> iterator =
															valueGroups.getIterator();
													iterator.hasValue(); iterator.advance()) {
												// Compose statement
												CString	statement =
																CString(OSSTR("SELECT ")) + columnNames +
																		CString(OSSTR(" FROM `")) +
																		mName + CString(OSSTR("`")) +
																		(innerJoin.hasReference() ?
																				innerJoin->getString() :
																				CString::mEmpty) +
																		iterator->getString() +
																		(orderBy.hasReference() ?
																				orderBy->getString() :
																				CString::mEmpty) +
																		(limit.hasReference() ?
																				limit->getString() :
																				CString::mEmpty);


												// Perform
												OV<SError>	error =
																	mStatementPerformer.perform(statement,
																			iterator->getValues(), resultsRowProc,
																			userData);
												ReturnErrorIfError(error);
											}

											return OV<SError>();
										} else {
											// No CSQLiteWhere
											CString	statement =
															CString(OSSTR("SELECT ")) + columnNames +
																	CString(OSSTR(" FROM `")) +
																	mName + CString(OSSTR("`")) +
																	(innerJoin.hasReference() ?
																			innerJoin->getString() :
																			CString::mEmpty) +
																	(orderBy.hasReference() ?
																			orderBy->getString() :
																			CString::mEmpty) +
																	(limit.hasReference() ?
																			limit->getString() : CString::mEmpty);

											return mStatementPerformer.perform(statement, resultsRowProc, userData);
										}
									}

		static	void			storeLastInsertRowID(SInt64 lastRowID, SInt64* sInt64)
									{ *sInt64 = lastRowID; }
		static	OV<SError>		addTableNameFromResultsRow(const CSQLiteResultsRow& resultsRow,
										TNArray<CString>* tableNames)
									{
										// Add table name
										(*tableNames) += *resultsRow.getText(mNameTableColumn);

										return OV<SError>();
									}

				CString										mName;
				CSQLiteTable::Options						mOptions;
				TNArray<CSQLiteTableColumn>					mTableColumns;
				TNDictionary<CSQLiteTableColumn::Reference>	mTableColumnReferenceMap;
				CSQLiteStatementPerformer&					mStatementPerformer;

				TNDictionary<CSQLiteTableColumn>			mTableColumnReferenceByName;

		static	CSQLiteTableColumn							mCountAllTableColumn;
		static	CSQLiteTableColumn							mNameTableColumn;
};

CSQLiteTableColumn	CSQLiteTable::Internals::mCountAllTableColumn(CString(OSSTR("COUNT(*)")),
							CSQLiteTableColumn::kKindInteger);
CSQLiteTableColumn	CSQLiteTable::Internals::mNameTableColumn(CString(OSSTR("name")), CSQLiteTableColumn::kKindText);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSQLiteTable

// MARK: Properties

const	CSQLiteTableColumn	CSQLiteTable::mSelectAllTableColumn(CString(OSSTR("*")), CSQLiteTableColumn::kKindInteger);

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CSQLiteTable::CSQLiteTable(const CString& name, Options options, const TArray<CSQLiteTableColumn>& tableColumns,
		const TArray<CSQLiteTableColumn::Reference>& references, CSQLiteStatementPerformer& statementPerformer)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(name, options, tableColumns, references, statementPerformer);
}

//----------------------------------------------------------------------------------------------------------------------
CSQLiteTable::CSQLiteTable(const CString& name, CSQLiteStatementPerformer& statementPerformer)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals =
			new Internals(name, kOptionsNone, TNArray<CSQLiteTableColumn>(), TNArray<CSQLiteTableColumn::Reference>(),
					statementPerformer);
}

//----------------------------------------------------------------------------------------------------------------------
CSQLiteTable::CSQLiteTable(const CSQLiteTable& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CSQLiteTable::~CSQLiteTable()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
const CString& CSQLiteTable::getName() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mName;
}

//----------------------------------------------------------------------------------------------------------------------
SInt32 CSQLiteTable::getVariableNumberLimit() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mStatementPerformer.getVariableNumberLimit();
}

//----------------------------------------------------------------------------------------------------------------------
void CSQLiteTable::add(const CSQLiteTableColumn& tableColumn)
//----------------------------------------------------------------------------------------------------------------------
{
	// Perform
	mInternals->mStatementPerformer.addToTransactionOrPerform(
			CString(OSSTR("ALTER TABLE `")) + mInternals->mName + CString(OSSTR("` ADD COLUMN ")) +
					mInternals->getCreateString(tableColumn));

	// Update
	mInternals->mTableColumns += tableColumn;
}

//----------------------------------------------------------------------------------------------------------------------
void CSQLiteTable::add(const CSQLiteTrigger& trigger)
//----------------------------------------------------------------------------------------------------------------------
{
	// Perform
	mInternals->mStatementPerformer.addToTransactionOrPerform(trigger.getString(*this));
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CSQLiteTable::count(const OR<CSQLiteInnerJoin>& innerJoin, const OR<CSQLiteWhere>& where) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	Internals::SInt64Result	sInt64Result(Internals::mCountAllTableColumn);

	// Perform
	mInternals->select(Internals::mCountAllTableColumn.getName(), innerJoin, where, OR<CSQLiteOrderBy>(),
			OR<CSQLiteLimit>(), (CSQLiteResultsRow::Proc) Internals::SInt64Result::process, &sInt64Result);

	return sInt64Result.getValue().hasValue() ? (UInt32) *sInt64Result.getValue() : 0;
}

//----------------------------------------------------------------------------------------------------------------------
void CSQLiteTable::create(bool ifNotExists) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNArray<CString>	tableColumnInfos;
	for (TIteratorD<CSQLiteTableColumn> iterator = mInternals->mTableColumns.getIterator(); iterator.hasValue();
			iterator.advance()) {
		// Start with create string
		CString	tableColumnInfo = mInternals->getCreateString(*iterator);

		// Add references if applicable
		OR<CSQLiteTableColumn::Reference>	tableColumnReference =
													mInternals->mTableColumnReferenceMap[iterator->getName()];
		if (tableColumnReference.hasReference())
			// Add reference
			tableColumnInfo +=
					CString(OSSTR(" REFERENCES ")) +
							tableColumnReference->getReferencedTable().getName() + CString(OSSTR("(")) +
							tableColumnReference->getReferencedTableColumn().getName() +
							CString(OSSTR(") ON UPDATE CASCADE"));

		// Add
		tableColumnInfos += tableColumnInfo;
	}

	// Create
	CString	statement =
					CString(OSSTR("CREATE TABLE")) + CString(ifNotExists ? OSSTR(" IF NOT EXISTS") : OSSTR("")) +
							CString(OSSTR(" `")) + mInternals->mName + CString(OSSTR("` (")) +
							CString(tableColumnInfos) + CString(OSSTR(")")) +
							CString((mInternals->mOptions & kOptionsWithoutRowID) ?
									OSSTR(" WITHOUT ROWID") : OSSTR(""));
	mInternals->mStatementPerformer.addToTransactionOrPerform(statement);
}

//----------------------------------------------------------------------------------------------------------------------
void CSQLiteTable::deleteRow(const CSQLiteWhere& where)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CString	statement = CString(OSSTR("DELETE FROM `")) + mInternals->mName + CString(OSSTR("`"));

	// Perform
	TArray<CSQLiteWhere::ValueGroup>	valueGroups =
												where.getValueGroups(
														mInternals->mStatementPerformer.getVariableNumberLimit());
	for (TIteratorD<CSQLiteWhere::ValueGroup> iterator = valueGroups.getIterator(); iterator.hasValue();
			iterator.advance())
		// Add to transaction or perform
		mInternals->mStatementPerformer.addToTransactionOrPerform(statement + iterator->getString(),
				iterator->getValues());
}

//----------------------------------------------------------------------------------------------------------------------
void CSQLiteTable::deleteRows(const CSQLiteTableColumn& tableColumn, const TArray<SSQLiteValue>& values) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Perform in chunks of SQLITE_LIMIT_VARIABLE_NUMBER
	TArray<TArray<SSQLiteValue> >	valuesChunks =
											TNArray<SSQLiteValue>::asChunksFrom(values,
													mInternals->mStatementPerformer.getVariableNumberLimit());

	// Iterate values chunks
	for (TIteratorD<TArray<SSQLiteValue> > iterator = valuesChunks.getIterator(); iterator.hasValue();
			iterator.advance()) {
		// Setup
		CString	statement =
					CString(OSSTR("DELETE FROM `")) + mInternals->mName + CString(OSSTR("` WHERE `")) +
							tableColumn.getName() + CString(OSSTR("` IN (")) +
							CString(TSArray<CString>(CString(OSSTR("?")), iterator->getCount()), CString(OSSTR(","))) +
							CString(OSSTR(")"));

		// Perform
		mInternals->mStatementPerformer.addToTransactionOrPerform(statement, *iterator);
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CSQLiteTable::drop(const CString& triggerName)
//----------------------------------------------------------------------------------------------------------------------
{
	// Perform
	mInternals->mStatementPerformer.addToTransactionOrPerform(CString(OSSTR("DROP TRIGGER ")) + triggerName);
}

//----------------------------------------------------------------------------------------------------------------------
void CSQLiteTable::drop() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Perform
	mInternals->mStatementPerformer.addToTransactionOrPerform(
			CString(OSSTR("DROP TABLE `")) + mInternals->mName + CString(OSSTR("`")));
}

//----------------------------------------------------------------------------------------------------------------------
bool CSQLiteTable::hasRow(const CSQLiteWhere& where) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Return count
	return count(OR<CSQLiteInnerJoin>(), OR<CSQLiteWhere>((CSQLiteWhere&) where)) > 0;
}

//----------------------------------------------------------------------------------------------------------------------
SInt64 CSQLiteTable::insertRow(const TArray<TableColumnAndValue>& tableColumnAndValues) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Perform
	SInt64	lastInsertRowID = 0;
	insertRow(tableColumnAndValues, (LastInsertRowIDProc) Internals::storeLastInsertRowID, &lastInsertRowID);

	return lastInsertRowID;
}

//----------------------------------------------------------------------------------------------------------------------
void CSQLiteTable::insertRow(const TArray<TableColumnAndValue>& tableColumnAndValues,
		LastInsertRowIDProc lastInsertRowIDProc, void* userData) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CString					statement =
									CString(OSSTR("INSERT INTO `")) + mInternals->mName + CString(OSSTR("` (")) +
											mInternals->getColumnNames(tableColumnAndValues) +
											CString(OSSTR(") VALUES (")) +
											CString(
													TNArray<CString>(CString(OSSTR("?")),
															tableColumnAndValues.getCount()),
													CString(OSSTR(","))) +
											CString(OSSTR(")"));
	TArray<SSQLiteValue>	values = mInternals->getValues(tableColumnAndValues);

	// Perform
	mInternals->mStatementPerformer.addToTransactionOrPerform(statement, values, lastInsertRowIDProc, userData);
}

//----------------------------------------------------------------------------------------------------------------------
SInt64 CSQLiteTable::insertOrReplaceRow(const TArray<TableColumnAndValue>& tableColumnAndValues) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Perform
	SInt64	lastInsertRowID = 0;
	insertOrReplaceRow(tableColumnAndValues, (LastInsertRowIDProc) Internals::storeLastInsertRowID, &lastInsertRowID);

	return lastInsertRowID;
}

//----------------------------------------------------------------------------------------------------------------------
void CSQLiteTable::insertOrReplaceRow(const TArray<TableColumnAndValue>& tableColumnAndValues,
		LastInsertRowIDProc lastInsertRowIDProc, void* userData) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CString					statement =
									CString(OSSTR("INSERT OR REPLACE INTO `")) + mInternals->mName +
											CString(OSSTR("` (")) +
											mInternals->getColumnNames(tableColumnAndValues) +
											CString(OSSTR(") VALUES (")) +
											CString(
													TSArray<CString>(
															CString(OSSTR("?")), tableColumnAndValues.getCount()),
													CString(OSSTR(","))) +
											CString(OSSTR(")"));
	TArray<SSQLiteValue>	values = mInternals->getValues(tableColumnAndValues);

	// Perform
	mInternals->mStatementPerformer.addToTransactionOrPerform(statement, values, lastInsertRowIDProc, userData);
}

//----------------------------------------------------------------------------------------------------------------------
void CSQLiteTable::insertOrReplaceRows(const CSQLiteTableColumn& tableColumn, const TArray<SSQLiteValue>& values) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Perform in chunks of SQLITE_LIMIT_VARIABLE_NUMBER
	TArray<TArray<SSQLiteValue> >	valuesChunks =
											TNArray<SSQLiteValue>::asChunksFrom(values,
													mInternals->mStatementPerformer.getVariableNumberLimit());

	// Iterate values chunks
	for (TIteratorD<TArray<SSQLiteValue> > iterator = valuesChunks.getIterator(); iterator.hasValue();
			iterator.advance()) {
		// Setup
		CString	statement =
					CString(OSSTR("INSERT OR REPLACE INTO `")) + mInternals->mName + CString(OSSTR("` (`")) +
							tableColumn.getName() + CString(OSSTR("`) VALUES ")) +
							CString(TSArray<CString>(CString(OSSTR("(?)")), iterator->getCount()), CString(OSSTR(",")));

		// Perform
		mInternals->mStatementPerformer.addToTransactionOrPerform(statement, *iterator);
	}
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CSQLiteTable::migrate(ResultsRowMigrationProc resultsRowMigrationProc, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CString			tempTableName = CUUID().getBase64String();
	CSQLiteTable	tempTable(tempTableName, mInternals->mOptions, mInternals->mTableColumns,
							TNArray<CSQLiteTableColumn::Reference>(), mInternals->mStatementPerformer);

	// Create new table
	tempTable.create();

	// Migrate content
	Internals::MigrationInfo	migrationInfo(
										CString(OSSTR("INSERT INTO `")) + tempTableName + CString(OSSTR("` (`")) +
												mInternals->getColumnNames(mInternals->mTableColumns) +
												CString(OSSTR(") VALUES (")) +
												CString(
														TSArray<CString>(CString(OSSTR("(?)")),
																mInternals->mTableColumns.getCount()),
														CString(OSSTR(","))),
										*this, resultsRowMigrationProc, userData);
	mInternals->mStatementPerformer.performAsTransaction(
			(CSQLiteStatementPerformer::TransactionProc) Internals::MigrationInfo::perform, &migrationInfo);
	ReturnErrorIfError(migrationInfo.getError());

	// Drop current table
	drop();

	// Rename temp to current
	tempTable.rename(mInternals->mName);

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
void CSQLiteTable::rename(const CString& name) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Perform
	mInternals->mStatementPerformer.addToTransactionOrPerform(
			CString(OSSTR("ALTER TABLE `")) + mInternals->mName + CString(OSSTR("` RENAME TO `")) + name +
					CString(OSSTR("`")));

	// Update
	mInternals->mName = name;
}

//----------------------------------------------------------------------------------------------------------------------
OV<SInt64> CSQLiteTable::rowID(const CSQLiteWhere& where) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	Internals::SInt64Result	sInt64Result(mSelectAllTableColumn);

	// Perform
	mInternals->select(CSQLiteTableColumn::mRowID.getName(), OR<CSQLiteInnerJoin>(),
			OR<CSQLiteWhere>((CSQLiteWhere&) where), OR<CSQLiteOrderBy>(), OR<CSQLiteLimit>(),
			(CSQLiteResultsRow::Proc) Internals::SInt64Result::process, &sInt64Result);

	return sInt64Result.getValue();
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CSQLiteTable::select(const TArray<CSQLiteTableColumn>& tableColumns, const OR<CSQLiteInnerJoin>& innerJoin,
		const OR<CSQLiteWhere>& where, const OR<CSQLiteOrderBy>& orderBy, const OR<CSQLiteLimit>& limit,
		CSQLiteResultsRow::Proc resultsRowProc, void* userData) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Perform
	return mInternals->select(mInternals->getColumnNames(tableColumns), innerJoin, where, orderBy, limit,
			resultsRowProc, userData);
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CSQLiteTable::select(const TArray<TableAndTableColumn>& tableAndTableColumns,
		const OR<CSQLiteInnerJoin>& innerJoin, const OR<CSQLiteWhere>& where, const OR<CSQLiteOrderBy>& orderBy,
		const OR<CSQLiteLimit>& limit, CSQLiteResultsRow::Proc resultsRowProc, void* userData) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Perform
	return mInternals->select(mInternals->getColumnNames(tableAndTableColumns), innerJoin, where, orderBy, limit,
			resultsRowProc, userData);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<SInt64> CSQLiteTable::sum(const CSQLiteTableColumn& tableColumn, const OR<CSQLiteInnerJoin>& innerJoin,
		const OR<CSQLiteWhere>& where) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CSQLiteTableColumn		sumTableColumn = CSQLiteTableColumn::sum(tableColumn);
	Internals::SInt64Result	sInt64Result(sumTableColumn);

	// Perform
	OV<SError>	error =
						mInternals->select(sumTableColumn.getName(), innerJoin, where, OR<CSQLiteOrderBy>(),
								OR<CSQLiteLimit>(), (CSQLiteResultsRow::Proc) Internals::SInt64Result::process,
								&sInt64Result);
	ReturnValueIfError(error, TVResult<SInt64>(*error));

	return TVResult<SInt64>(sInt64Result.getValue().hasValue() ? *sInt64Result.getValue() : 0);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CDictionary> CSQLiteTable::sum(const TArray<CSQLiteTableColumn>& tableColumns,
		const OR<CSQLiteInnerJoin>& innerJoin, const OR<CSQLiteWhere>& where) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNDictionary<CSQLiteTableColumn>	tableColumnNameBySumTableColumnName;
	TNArray<CString>					sumTableColumnNames;
	for (TIteratorD<CSQLiteTableColumn> iterator = tableColumns.getIterator(); iterator.hasValue();
			iterator.advance()) {
		// Setup
		CSQLiteTableColumn	sumTableColumn = CSQLiteTableColumn::sum(*iterator);

		// Add
		tableColumnNameBySumTableColumnName.set(iterator->getName(), sumTableColumn);
		sumTableColumnNames += sumTableColumn.getName();
	}

	Internals::SInt64ResultByTableColumnName	sInt64ResultByTableColumnName(tableColumnNameBySumTableColumnName);

	// Perform
	OV<SError>	error =
						mInternals->select(sumTableColumnNames, innerJoin, where, OR<CSQLiteOrderBy>(),
								OR<CSQLiteLimit>(),
								(CSQLiteResultsRow::Proc) Internals::SInt64ResultByTableColumnName::process,
								&sInt64ResultByTableColumnName);
	ReturnValueIfError(error, TVResult<CDictionary>(*error));

	return TVResult<CDictionary>(sInt64ResultByTableColumnName.getResultByTableColumnName());
}

//----------------------------------------------------------------------------------------------------------------------
const CSQLiteTableColumn& CSQLiteTable::getTableColumn(const CString& name) const
//----------------------------------------------------------------------------------------------------------------------
{
	return *mInternals->mTableColumnReferenceByName[name];
}

//----------------------------------------------------------------------------------------------------------------------
TArray<CSQLiteTableColumn> CSQLiteTable::getTableColumns(const TArray<CString>& names) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNArray<CSQLiteTableColumn>	tableColumns;
	for (TIteratorD<CString> iterator = names.getIterator(); iterator.hasValue(); iterator.advance())
		// Add table column
		tableColumns += *mInternals->mTableColumnReferenceByName[*iterator];

	return tableColumns;
}

//----------------------------------------------------------------------------------------------------------------------
void CSQLiteTable::update(const TArray<TableColumnAndValue>& tableColumnAndValues, const CSQLiteWhere& where) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNArray<CString>	tableColumnStrings;
	for (UInt32 i = 0; i < tableColumnAndValues.getCount(); i++)
		// Add string
		tableColumnStrings +=
				CString(OSSTR("`")) + tableColumnAndValues[i].getTableColumn().getName() + CString(OSSTR("` = ?"));

	// Iterate all groups in CSQLiteWhere
	SInt32								groupSize =
												mInternals->mStatementPerformer.getVariableNumberLimit() -
														tableColumnAndValues.getCount();
	TArray<CSQLiteWhere::ValueGroup>	valueGroups = where.getValueGroups(groupSize);
	for (TIteratorD<CSQLiteWhere::ValueGroup> iterator = valueGroups.getIterator(); iterator.hasValue();
			iterator.advance()) {
		// Compose statement
		CString					statement =
										CString(OSSTR("UPDATE `")) + mInternals->mName + CString(OSSTR("` SET ")) +
												CString(tableColumnStrings) + iterator->getString();
		TArray<SSQLiteValue>	values = mInternals->getValues(tableColumnAndValues) + iterator->getValues();

		// Perform
		mInternals->mStatementPerformer.addToTransactionOrPerform(statement, values);
	}
}

//----------------------------------------------------------------------------------------------------------------------
TArray<CSQLiteTable> CSQLiteTable::getAll(CSQLiteStatementPerformer& statementPerformer)
//----------------------------------------------------------------------------------------------------------------------
{
	// Collect table names
	TNArray<CString>	tableNames;
	statementPerformer.perform(
			CString(OSSTR("SELECT name FROM sqlite_master WHERE type='table' AND name NOT LIKE 'sqlite_%'")),
			(CSQLiteResultsRow::Proc) Internals::addTableNameFromResultsRow, &tableNames);

	// Create Tables
	TNArray<CSQLiteTable>	tables;
	for (TIteratorD<CString> iterator = tableNames.getIterator(); iterator.hasValue(); iterator.advance())
		// Add Table
		tables += CSQLiteTable(*iterator, statementPerformer);

	return tables;
}
