//----------------------------------------------------------------------------------------------------------------------
//	CSQLiteTable.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CSQLiteTable.h"

#include "CDictionary.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSQLiteTable::Internals

class CSQLiteTable::Internals : public TReferenceCountableAutoDelete<Internals> {
	// Types
	public:
		struct SInt64Results {
			// Lifecycle methods
			SInt64Results(const CSQLiteTableColumn& tableColumn) : mTableColumn(tableColumn) {}

			// Properties
			const	CSQLiteTableColumn&	mTableColumn;
					OV<SInt64>			mValue;
		};

	// Methods
	public:
								Internals(const CString& name, CSQLiteTable::Options options,
										const TArray<CSQLiteTableColumn>& tableColumns,
										const TArray<CSQLiteTableColumn::Reference>& references,
										CSQLiteStatementPerformer& statementPerformer) :
									TReferenceCountableAutoDelete(),
											mName(name), mOptions(options), mTableColumns(tableColumns),
											mStatementPerformer(statementPerformer)
									{
										// Iterate all table column references
										for (TIteratorD<CSQLiteTableColumn::Reference> iterator =
														references.getIterator();
												iterator.hasValue(); iterator.advance())
											// Add to map
											mTableColumnReferenceMap.set(iterator->mTableColumn.getName(), *iterator);
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
										if (tableColumn.getOptions() & CSQLiteTableColumn::kPrimaryKey)
											string += CString(OSSTR(" PRIMARY KEY"));
										if (tableColumn.getOptions() & CSQLiteTableColumn::kAutoIncrement)
											string += CString(OSSTR(" AUTOINCREMENT"));
										if (tableColumn.getOptions() & CSQLiteTableColumn::kNotNull)
											string += CString(OSSTR(" NOT NULL"));
										if (tableColumn.getOptions() & CSQLiteTableColumn::kUnique)
											string += CString(OSSTR(" UNIQUE"));
										if (tableColumn.getOptions() & CSQLiteTableColumn::kCheck)
											string += CString(OSSTR(" CHECK"));

										if (tableColumn.getDefaultValue().hasValue())
											// Default
											string +=
													CString(OSSTR(" DEFAULT (")) +
															tableColumn.getDefaultValue()->getString() +
															CString(OSSTR(")"));

										return string;
									}
		CString					getColumnNames(const CSQLiteTableColumn tableColumns[], UInt32 count)
									{
										// Setup
										TNArray<CString>	strings;
										for (UInt32 i = 0; i < count; i++) {
											// Add string
											if ((tableColumns[i] == CSQLiteTableColumn::mAll) ||
													(tableColumns[i] == CSQLiteTableColumn::mRowID))
												// Non-complex
												strings += tableColumns[i].getName();
											else
												// Escape
												strings +=
															CString(OSSTR("`")) + tableColumns[i].getName() +
																	CString(OSSTR("`"));
										}

										return CString(strings, CString(OSSTR(",")));
									}
		CString					getColumnNames(const CSQLiteTable::TableColumnAndValue tableColumnAndValues[],
										UInt32 count)
									{
										// Setup
										TNArray<CString>	strings;
										for (UInt32 i = 0; i < count; i++)
											// Add string
											strings +=
													CString(OSSTR("`")) +
															tableColumnAndValues[i].mTableColumn.getName() +
															CString(OSSTR("`"));

										return CString(strings, CString(OSSTR(",")));
									}
//		CString					getColumnNames(const TArray<CSQLiteTable::TableAndTableColumn>& tableAndTableColumns)
//									{
//										// Setup
//										TNArray<CString>	strings;
//										for (TIteratorD<CSQLiteTable::TableAndTableColumn> iterator =
//														tableAndTableColumns.getIterator();
//												iterator.hasValue(); iterator.advance())
//											// Add string
//											strings +=
//													CString(OSSTR("`")) + iterator->mTable.getName() +
//															CString(OSSTR("`.`")) + iterator->mTableColumn.getName() +
//															CString(OSSTR("`"));
//
//										return CString(strings, CString(OSSTR(",")));
//									}
		TArray<SSQLiteValue>	getValues(const CSQLiteTable::TableColumnAndValue tableColumnAndValues[], UInt32 count)
									{
										// Collect values
										TNArray<SSQLiteValue>	values;
										for (UInt32 i = 0; i < count; i++)
											// Add value
											values += tableColumnAndValues[i].mValue;

										return values;
									}
		void					select(const CString& columnNames, const OR<CSQLiteInnerJoin>& innerJoin,
										const OR<CSQLiteWhere>& where, const OR<CSQLiteOrderBy>& orderBy,
										CSQLiteResultsRow::Proc resultsRowProc, void* userData)
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
																		iterator->mString +
																		(orderBy.hasReference() ?
																				orderBy->getString() :
																				CString::mEmpty);


												// Perform
												resultsRowProc(
														mStatementPerformer.perform(statement, iterator->mValues),
														userData);
											}
										} else {
											// No CSQLiteWhere
											CString	statement =
															CString(OSSTR("SELECT ")) + columnNames +
																	CString(OSSTR(" FROM `")) +
																	mName + CString(OSSTR("`")) +
																	(innerJoin.hasReference() ?
																			innerJoin->getString() : CString::mEmpty) +
																	(orderBy.hasReference() ?
																			orderBy->getString() : CString::mEmpty);

											// Perform
											resultsRowProc(mStatementPerformer.perform(statement), userData);
										}
									}

		static	void			storeLastInsertRowID(SInt64 lastRowID, SInt64* sInt64)
									{ *sInt64 = lastRowID; }
		static	void			storeSInt64Results(const CSQLiteResultsRow& resultsRow, SInt64Results* sInt64Results)
									{
										// Try to get row
										if (resultsRow.moveToNext())
											// Store value
											sInt64Results->mValue = resultsRow.getSInt64(sInt64Results->mTableColumn);
									}

				CString										mName;
				CSQLiteTable::Options						mOptions;
				TArray<CSQLiteTableColumn>					mTableColumns;
				TNDictionary<CSQLiteTableColumn::Reference>	mTableColumnReferenceMap;
				CSQLiteStatementPerformer&					mStatementPerformer;

		static	CSQLiteTableColumn							mCountAllTableColumn;
};

CSQLiteTableColumn	CSQLiteTable::Internals::mCountAllTableColumn(CString(OSSTR("COUNT(*)")),
							CSQLiteTableColumn::kKindInteger);

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
CSQLiteTable::CSQLiteTable(const CString& name, Options options, const TArray<CSQLiteTableColumn>& tableColumns,
		CSQLiteStatementPerformer& statementPerformer)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals =
			new Internals(name, options, tableColumns, TNArray<CSQLiteTableColumn::Reference>(), statementPerformer);
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
void CSQLiteTable::create(bool ifNotExists) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNArray<CString>	columnInfos;
	for (TIteratorD<CSQLiteTableColumn> iterator = mInternals->mTableColumns.getIterator(); iterator.hasValue();
			iterator.advance()) {
		// Start with create string
		CString	string = mInternals->getCreateString(*iterator);

		// Handle reference
		OR<CSQLiteTableColumn::Reference>	tableColumnReference =
													mInternals->mTableColumnReferenceMap[iterator->getName()];
		if (tableColumnReference.hasReference())
			// Add reference
			string +=
					CString(OSSTR(" REFERENCES ")) +
							tableColumnReference->mReferencedTable.getName() + CString(OSSTR("(")) +
							tableColumnReference->mReferencedTableColumn.getName() +
							CString(OSSTR(") ON UPDATE CASCADE"));

		// Add
		columnInfos += string;
	}

	// Create
	CString	statement =
					CString(OSSTR("CREATE TABLE")) + CString(ifNotExists ? OSSTR(" IF NOT EXISTS") : OSSTR("")) +
							CString(OSSTR(" `")) + mInternals->mName + CString(OSSTR("` (")) +
							CString(columnInfos) + CString(OSSTR(")")) +
							CString((mInternals->mOptions & kWithoutRowID) ? OSSTR(" WITHOUT ROWID") : OSSTR(""));
	mInternals->mStatementPerformer.addToTransactionOrPerform(statement);
}

////----------------------------------------------------------------------------------------------------------------------
//void CSQLiteTable::rename(const CString& name)
////----------------------------------------------------------------------------------------------------------------------
//{
//	// Perform
//	mInternals->mStatementPerformer.addToTransactionOrPerform(
//			CString(OSSTR("ALTER TABLE `")) + mInternals->mName + CString(OSSTR("` RENAME TO `")) + name +
//					CString(OSSTR("`")));
//
//	// Update
//	mInternals->mName = name;
//}

////----------------------------------------------------------------------------------------------------------------------
//void CSQLiteTable::add(const CSQLiteTableColumn& tableColumn)
////----------------------------------------------------------------------------------------------------------------------
//{
//	// Perform
//	mInternals->mStatementPerformer.addToTransactionOrPerform(
//			CString(OSSTR("ALTER TABLE `")) + mInternals->mName + CString(OSSTR("` ADD COLUMN ")) +
//					mInternals->getCreateString(tableColumn));
//
//	// Update
//	mInternals->mTableColumns += tableColumn;
//}

////----------------------------------------------------------------------------------------------------------------------
//void CSQLiteTable::add(const CSQLiteTrigger& trigger)
////----------------------------------------------------------------------------------------------------------------------
//{
//	// Perform
//	mInternals->mStatementPerformer.addToTransactionOrPerform(trigger.getString(*this));
//}

////----------------------------------------------------------------------------------------------------------------------
//void CSQLiteTable::drop(const CString& triggerName)
////----------------------------------------------------------------------------------------------------------------------
//{
//	// Perform
//	mInternals->mStatementPerformer.addToTransactionOrPerform(CString(OSSTR("DROP TRIGGER ")) + triggerName);
//}

//----------------------------------------------------------------------------------------------------------------------
void CSQLiteTable::drop() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Perform
	mInternals->mStatementPerformer.addToTransactionOrPerform(
			CString(OSSTR("DROP TABLE `")) + mInternals->mName + CString(OSSTR("`")));
}

////----------------------------------------------------------------------------------------------------------------------
//bool CSQLiteTable::hasRow(const CSQLiteWhere& where) const
////----------------------------------------------------------------------------------------------------------------------
//{
//	// Return count
//	return count(OR<CSQLiteWhere>((CSQLiteWhere&) where)) > 0;
//}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CSQLiteTable::count(const OR<CSQLiteInnerJoin>& innerJoin, const OR<CSQLiteWhere>& where) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	Internals::SInt64Results	sInt64Results(Internals::mCountAllTableColumn);

	// Perform
	mInternals->select(Internals::mCountAllTableColumn.getName(), innerJoin, where, OR<CSQLiteOrderBy>(),
			(CSQLiteResultsRow::Proc) Internals::storeSInt64Results, &sInt64Results);

	return sInt64Results.mValue.hasValue() ? (UInt32) *sInt64Results.mValue : 0;
}

////----------------------------------------------------------------------------------------------------------------------
//OV<SInt64> CSQLiteTable::rowID(const CSQLiteWhere& where) const
////----------------------------------------------------------------------------------------------------------------------
//{
//	// Setup
//	SInt64Results	sInt64Results(sCountAllTableColumn);
//
//	// Perform
//	mInternals->select(CSQLiteTableColumn::mRowID.getName(), OR<CSQLiteInnerJoin>(),
//			OR<CSQLiteWhere>((CSQLiteWhere&) where), OR<CSQLiteOrderBy>(), sStoreSInt64Results, &sInt64Results);
//
//	return sInt64Results.mValue;
//}

//----------------------------------------------------------------------------------------------------------------------
void CSQLiteTable::select(const CSQLiteTableColumn tableColumns[], UInt32 count, const OR<CSQLiteInnerJoin>& innerJoin,
		const OR<CSQLiteWhere>& where, const OR<CSQLiteOrderBy>& orderBy, CSQLiteResultsRow::Proc resultsRowProc,
		void* userData) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Perform
	mInternals->select(mInternals->getColumnNames(tableColumns, count), innerJoin, where, orderBy, resultsRowProc,
			userData);
}

////----------------------------------------------------------------------------------------------------------------------
//void CSQLiteTable::select(const TArray<TableAndTableColumn>& tableAndTableColumns,
//		const OR<CSQLiteInnerJoin>& innerJoin, const OR<CSQLiteWhere>& where, const OR<CSQLiteOrderBy>& orderBy,
//		CSQLiteResultsRow::Proc resultsRowProc, void* userData)
////----------------------------------------------------------------------------------------------------------------------
//{
//	// Perform
//	mInternals->select(mInternals->getColumnNames(tableAndTableColumns), innerJoin, where, orderBy, resultsRowProc,
//			userData);
//}

//----------------------------------------------------------------------------------------------------------------------
SInt64 CSQLiteTable::insertRow(const TableColumnAndValue tableColumnAndValues[], UInt32 count) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Perform
	SInt64	lastInsertRowID = 0;
	insertRow(tableColumnAndValues, count, (LastInsertRowIDProc) Internals::storeLastInsertRowID, &lastInsertRowID);

	return lastInsertRowID;
}

//----------------------------------------------------------------------------------------------------------------------
void CSQLiteTable::insertRow(const TableColumnAndValue tableColumnAndValues[], UInt32 count,
		LastInsertRowIDProc lastInsertRowIDProc, void* userData) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CString					statement =
									CString(OSSTR("INSERT INTO `")) + mInternals->mName + CString(OSSTR("` (")) +
											mInternals->getColumnNames(tableColumnAndValues, count) +
											CString(OSSTR(") VALUES (")) +
											CString(TNArray<CString>(CString(OSSTR("?")), count), CString(OSSTR(","))) +
											CString(OSSTR(")"));
	TArray<SSQLiteValue>	values = mInternals->getValues(tableColumnAndValues, count);

	// Perform
	mInternals->mStatementPerformer.addToTransactionOrPerform(statement, values, lastInsertRowIDProc, userData);
}

//----------------------------------------------------------------------------------------------------------------------
SInt64 CSQLiteTable::insertOrReplaceRow(const TableColumnAndValue tableColumnAndValues[], UInt32 count) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Perform
	SInt64	lastInsertRowID = 0;
	insertOrReplaceRow(tableColumnAndValues, count, (LastInsertRowIDProc) Internals::storeLastInsertRowID,
			&lastInsertRowID);

	return lastInsertRowID;
}

//----------------------------------------------------------------------------------------------------------------------
void CSQLiteTable::insertOrReplaceRow(const TableColumnAndValue tableColumnAndValues[], UInt32 count,
		LastInsertRowIDProc lastInsertRowIDProc, void* userData) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CString					statement =
									CString(OSSTR("INSERT OR REPLACE INTO `")) + mInternals->mName +
											CString(OSSTR("` (")) +
											mInternals->getColumnNames(tableColumnAndValues, count) +
											CString(OSSTR(") VALUES (")) +
											CString(TSArray<CString>(CString(OSSTR("?")), count),
													CString(OSSTR(","))) +
											CString(OSSTR(")"));
	TArray<SSQLiteValue>	values = mInternals->getValues(tableColumnAndValues, count);

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
void CSQLiteTable::update(const TableColumnAndValue tableColumnAndValues[], UInt32 count, const CSQLiteWhere& where)
		const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNArray<CString>	tableColumnStrings;
	for (UInt32 i = 0; i < count; i++)
		// Add string
		tableColumnStrings +=
				CString(OSSTR("`")) + tableColumnAndValues[i].mTableColumn.getName() + CString(OSSTR("` = ?"));

	// Iterate all groups in CSQLiteWhere
	SInt32								groupSize = mInternals->mStatementPerformer.getVariableNumberLimit() - count;
	TArray<CSQLiteWhere::ValueGroup>	valueGroups = where.getValueGroups(groupSize);
	for (TIteratorD<CSQLiteWhere::ValueGroup> iterator = valueGroups.getIterator(); iterator.hasValue();
			iterator.advance()) {
		// Compose statement
		CString					statement =
										CString(OSSTR("UPDATE `")) + mInternals->mName + CString(OSSTR("` SET ")) +
												CString(tableColumnStrings) + iterator->mString;
		TArray<SSQLiteValue>	values = mInternals->getValues(tableColumnAndValues, count) + iterator->mValues;

		// Perform
		mInternals->mStatementPerformer.addToTransactionOrPerform(statement, values);
	}
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
