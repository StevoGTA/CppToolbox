//----------------------------------------------------------------------------------------------------------------------
//	CSQLiteTable.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CSQLiteTable.h"

#include "CDictionary.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CSQLiteTableColumn	sCountAllTableColumn(CString(OSSTR("COUNT(*)")), CSQLiteTableColumn::kInteger);

struct SInt64Results {
	// Lifecycle methods
	SInt64Results(const CSQLiteTableColumn& tableColumn) : mTableColumn(tableColumn) {}

	// Properties
	const	CSQLiteTableColumn&	mTableColumn;
			OV<SInt64>			mValue;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local procs declarations

static	CSQLiteTableColumn	sTableColumnFromTableColumnAndValue(CArray::ItemRef item);
static	SSQLiteValue		sValueFromTableColumnAndValue(CArray::ItemRef item);
static	void				sStoreLastInsertRowID(SInt64 lastRowID, void* userData);
static	void				sStoreSInt64Results(const CSQLiteResultsRow& resultsRow, void* userData);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSQLiteTableInternals

class CSQLiteTableInternals {
	public:
				CSQLiteTableInternals(const CString& name, CSQLiteTable::Options options,
						const TArray<CSQLiteTableColumn>& tableColumns,
						const TArray<CSQLiteTableColumn::Reference>& references,
						CSQLiteStatementPerformer& statementPerformer) :
					mName(name), mOptions(options), mTableColumns(tableColumns), mStatementPerformer(statementPerformer)
					{
						// Iterate all table columns
						for (TIteratorD<CSQLiteTableColumn> iterator = tableColumns.getIterator(); iterator.hasValue();
								iterator.advance())
							// Add to map
							mTableColumnsMap.set(iterator->getName(), *iterator);

						// Iterate all table column references
						for (TIteratorD<CSQLiteTableColumn::Reference> iterator = references.getIterator();
								iterator.hasValue(); iterator.advance())
							// Add to map
							mTableColumnReferenceMap.set(iterator->mTableColumn.getName(), *iterator);
					}

		CString	getCreateString(const CSQLiteTableColumn& tableColumn)
					{
						// Compose column string
						CString	string = tableColumn.getName() + CString::mSpace;

						switch (tableColumn.getKind()) {
							case CSQLiteTableColumn::kInteger:
								// Integer
								string += CString(OSSTR("INTEGER"));
								break;

							case CSQLiteTableColumn::kReal:
								// Real
								string += CString(OSSTR("REAL"));
								break;

							case CSQLiteTableColumn::kText:
							case CSQLiteTableColumn::kDateISO8601FractionalSecondsAutoSet:
							case CSQLiteTableColumn::kDateISO8601FractionalSecondsAutoUpdate:
								// Text
								string += CString(OSSTR("TEXT"));
								break;

							case CSQLiteTableColumn::kBlob:
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

						if (tableColumn.getDefaultValue().hasInstance())
							// Default
							string +=
									CString(OSSTR(" DEFAULT (")) + tableColumn.getDefaultValue()->getString() +
											CString(OSSTR(")"));

						return string;
					}
		CString	getColumnNames(const TArray<CSQLiteTableColumn>& tableColumns)
					{
						// Setup
						TNArray<CString>	strings;
						for (TIteratorD<CSQLiteTableColumn> iterator = tableColumns.getIterator();
								iterator.hasValue(); iterator.advance())
							// Add string
							strings += CString(OSSTR("`")) + iterator->getName() + CString(OSSTR("`"));

						return CString(strings, CString(OSSTR(",")));
					}
		CString	getColumnNames(const TArray<CSQLiteTable::TableAndTableColumn>& tableAndTableColumns)
					{
						// Setup
						TNArray<CString>	strings;
						for (TIteratorD<CSQLiteTable::TableAndTableColumn> iterator =
										tableAndTableColumns.getIterator();
								iterator.hasValue(); iterator.advance())
							// Add string
							strings +=
									CString(OSSTR("`")) + iterator->mTable.getName() + CString(OSSTR("`.`")) +
											iterator->mTableColumn.getName() + CString(OSSTR("`"));

						return CString(strings, CString(OSSTR(",")));
					}
		void	select(const CString& columnNames, const OR<CSQLiteInnerJoin>& innerJoin, const OR<CSQLiteWhere>& where,
						const OR<CSQLiteOrderBy>& orderBy, CSQLiteResultsRow::Proc resultsRowProc, void* userData)
					{
						// Check if we have CSQLiteWhere
						if (where.hasReference()) {
							// Iterate all groups in CSQLiteWhere
							SInt32								variableNumberLimit =
																		mStatementPerformer.getVariableNumberLimit();
							TArray<CSQLiteWhere::ValueGroup>	valueGroups =
																		where->getValueGroups(variableNumberLimit);
							for (TIteratorD<CSQLiteWhere::ValueGroup> iterator = valueGroups.getIterator();
									iterator.hasValue(); iterator.advance()) {
								// Compose statement
								CString	statement =
												CString(OSSTR("SELECT ")) + columnNames + CString(OSSTR(" FROM `")) +
														mName + CString(OSSTR("`")) +
														(innerJoin.hasReference() ?
																innerJoin->getString() : CString::mEmpty) +
														iterator->mString +
														(orderBy.hasReference() ?
																orderBy->getString() : CString::mEmpty);


								// Perform
								resultsRowProc(mStatementPerformer.perform(statement, iterator->mValues), userData);
							}
						} else {
							// No CSQLiteWhere
							CString	statement =
											CString(OSSTR("SELECT ")) + columnNames + CString(OSSTR(" FROM `")) +
													mName + CString(OSSTR("`")) +
													(innerJoin.hasReference() ?
															innerJoin->getString() : CString::mEmpty) +
													(orderBy.hasReference() ?
															orderBy->getString() : CString::mEmpty);

							// Perform
							resultsRowProc(mStatementPerformer.perform(statement), userData);
						}
					}

		CString										mName;
		CSQLiteTable::Options						mOptions;
		TArray<CSQLiteTableColumn>					mTableColumns;
		TNDictionary<CSQLiteTableColumn>			mTableColumnsMap;
		TNDictionary<CSQLiteTableColumn::Reference>	mTableColumnReferenceMap;
		CSQLiteStatementPerformer&					mStatementPerformer;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSQLiteTable

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CSQLiteTable::CSQLiteTable(const CString& name, Options options, const TArray<CSQLiteTableColumn>& tableColumns,
		const TArray<CSQLiteTableColumn::Reference>& references, CSQLiteStatementPerformer& statementPerformer)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CSQLiteTableInternals(name, options, tableColumns, references, statementPerformer);
}

//----------------------------------------------------------------------------------------------------------------------
CSQLiteTable::CSQLiteTable(const CString& name, Options options, const TArray<CSQLiteTableColumn>& tableColumns,
		CSQLiteStatementPerformer& statementPerformer)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals =
			new CSQLiteTableInternals(name, options, tableColumns, TNArray<CSQLiteTableColumn::Reference>(),
					statementPerformer);
}

//----------------------------------------------------------------------------------------------------------------------
CSQLiteTable::~CSQLiteTable()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
const CString& CSQLiteTable::getName() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mName;
}

//----------------------------------------------------------------------------------------------------------------------
const CSQLiteTableColumn& CSQLiteTable::getTableColumn(const CString& name) const
//----------------------------------------------------------------------------------------------------------------------
{
	return *mInternals->mTableColumnsMap[name];
}

//----------------------------------------------------------------------------------------------------------------------
void CSQLiteTable::create(bool ifNotExists)
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

//----------------------------------------------------------------------------------------------------------------------
void CSQLiteTable::rename(const CString& name)
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
void CSQLiteTable::add(const CSQLiteTableColumn& tableColumn)
//----------------------------------------------------------------------------------------------------------------------
{
	// Perform
	mInternals->mStatementPerformer.addToTransactionOrPerform(
			CString(OSSTR("ALTER TABLE `")) + mInternals->mName + CString(OSSTR("` ADD COLUMN ")) +
					mInternals->getCreateString(tableColumn));

	// Update
	mInternals->mTableColumns += tableColumn;
	mInternals->mTableColumnsMap.set(tableColumn.getName() + CString(OSSTR("TableColumn")), tableColumn);
}

//----------------------------------------------------------------------------------------------------------------------
void CSQLiteTable::add(const CSQLiteTrigger& trigger)
//----------------------------------------------------------------------------------------------------------------------
{
	// Perform
	mInternals->mStatementPerformer.addToTransactionOrPerform(trigger.getString(*this));
}

//----------------------------------------------------------------------------------------------------------------------
void CSQLiteTable::drop()
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
	return count(OR<CSQLiteWhere>((CSQLiteWhere&) where)) > 0;
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CSQLiteTable::count(const OR<CSQLiteWhere>& where) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SInt64Results	sInt64Results(sCountAllTableColumn);

	// Perform
	mInternals->select(sCountAllTableColumn.getName(), OR<CSQLiteInnerJoin>(), where, OR<CSQLiteOrderBy>(),
			sStoreSInt64Results, &sInt64Results);

	return sInt64Results.mValue.hasValue() ? (UInt32) *sInt64Results.mValue : 0;
}

//----------------------------------------------------------------------------------------------------------------------
OV<SInt64> CSQLiteTable::rowID(const CSQLiteWhere& where) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SInt64Results	sInt64Results(sCountAllTableColumn);

	// Perform
	mInternals->select(CSQLiteTableColumn::mRowID.getName(), OR<CSQLiteInnerJoin>(),
			OR<CSQLiteWhere>((CSQLiteWhere&) where), OR<CSQLiteOrderBy>(), sStoreSInt64Results, &sInt64Results);

	return sInt64Results.mValue;
}

void sStoreSInt64Results(const CSQLiteResultsRow& resultsRow, void* userData)
{
	// Setup
	SInt64Results&	sInt64Results = *((SInt64Results*) userData);

	// Try to get row
	if (resultsRow.moveToNext())
		// Store value
		sInt64Results.mValue = resultsRow.getSInt64(sInt64Results.mTableColumn);
}

//----------------------------------------------------------------------------------------------------------------------
void CSQLiteTable::select(const OR<TArray<CSQLiteTableColumn> >& tableColumns,
		const OR<CSQLiteInnerJoin>& innerJoin, const OR<CSQLiteWhere>& where, const OR<CSQLiteOrderBy>& orderBy,
		CSQLiteResultsRow::Proc resultsRowProc, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Perform
	mInternals->select(tableColumns.hasReference() ? mInternals->getColumnNames(*tableColumns) : CString(OSSTR("*")),
			innerJoin, where, orderBy, resultsRowProc, userData);
}

//----------------------------------------------------------------------------------------------------------------------
void CSQLiteTable::select(const TArray<TableAndTableColumn>& tableAndTableColumns,
		const OR<CSQLiteInnerJoin>& innerJoin, const OR<CSQLiteWhere>& where, const OR<CSQLiteOrderBy>& orderBy,
		CSQLiteResultsRow::Proc resultsRowProc, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Perform
	mInternals->select(mInternals->getColumnNames(tableAndTableColumns), innerJoin, where, orderBy, resultsRowProc,
			userData);
}

//----------------------------------------------------------------------------------------------------------------------
SInt64 CSQLiteTable::insertRow(const TArray<TableColumnAndValue>& info)
//----------------------------------------------------------------------------------------------------------------------
{
	// Perform
	SInt64	lastInsertRowID = 0;
	insertRow(info, sStoreLastInsertRowID, &lastInsertRowID);

	return lastInsertRowID;
}

//----------------------------------------------------------------------------------------------------------------------
void CSQLiteTable::insertRow(const TArray<TableColumnAndValue>& info, LastInsertRowIDProc lastInsertRowIDProc,
		void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNArray<CSQLiteTableColumn>	tableColumns(info, sTableColumnFromTableColumnAndValue);
	CString						statement =
										CString(OSSTR("INSERT INTO `")) + mInternals->mName + CString(OSSTR("` (")) +
												mInternals->getColumnNames(tableColumns) +
												CString(OSSTR(") VALUES (")) +
												CString(TNArray<CString>(CString(OSSTR("?")), info.getCount()),
														CString(OSSTR(","))) +
												CString(OSSTR(")"));
	TNArray<SSQLiteValue>		values(info, sValueFromTableColumnAndValue);

	// Perform
	mInternals->mStatementPerformer.addToTransactionOrPerform(statement, values, lastInsertRowIDProc, userData);
}

//----------------------------------------------------------------------------------------------------------------------
SInt64 CSQLiteTable::insertOrReplaceRow(const TArray<TableColumnAndValue>& info)
//----------------------------------------------------------------------------------------------------------------------
{
	// Perform
	SInt64	lastInsertRowID = 0;
	insertOrReplaceRow(info, sStoreLastInsertRowID, &lastInsertRowID);

	return lastInsertRowID;
}

//----------------------------------------------------------------------------------------------------------------------
void CSQLiteTable::insertOrReplaceRow(const TArray<TableColumnAndValue>& info, LastInsertRowIDProc lastInsertRowIDProc,
		void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNArray<CSQLiteTableColumn>	tableColumns(info, sTableColumnFromTableColumnAndValue);
	CString						statement =
										CString(OSSTR("INSERT OR REPLACE INTO `")) + mInternals->mName +
												CString(OSSTR("` (")) + mInternals->getColumnNames(tableColumns) +
												CString(OSSTR(") VALUES (")) +
												CString(TNArray<CString>(CString(OSSTR("?")), info.getCount()),
														CString(OSSTR(","))) +
												CString(OSSTR(")"));
	TNArray<SSQLiteValue>		values(info, sValueFromTableColumnAndValue);

	// Perform
	mInternals->mStatementPerformer.addToTransactionOrPerform(statement, values, lastInsertRowIDProc, userData);
}

//----------------------------------------------------------------------------------------------------------------------
void CSQLiteTable::insertOrReplaceRows(const CSQLiteTableColumn& tableColumn, const TArray<SSQLiteValue>& values)
//----------------------------------------------------------------------------------------------------------------------
{
	// Perform in chunks of SQLITE_LIMIT_VARIABLE_NUMBER
	TArray<TArray<SSQLiteValue> >	valuesChunks =
											SSQLiteValue::chunked(values,
													mInternals->mStatementPerformer.getVariableNumberLimit());

	// Iterate values chunks
	for (TIteratorD<TArray<SSQLiteValue> > iterator = valuesChunks.getIterator(); iterator.hasValue();
			iterator.advance()) {
		// Setup
		CString	statement =
					CString(OSSTR("INSERT OR REPLACE INTO `")) + mInternals->mName + CString(OSSTR("` (`")) +
							tableColumn.getName() + CString(OSSTR("`) VALUES ")) +
							CString(TNArray<CString>(CString(OSSTR("(?)")), iterator->getCount()), CString(OSSTR(",")));

		// Perform
		mInternals->mStatementPerformer.addToTransactionOrPerform(statement, *iterator);
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CSQLiteTable::update(const TArray<TableColumnAndValue>& info, const CSQLiteWhere& where)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNArray<CString>	strings;
	for (TIteratorD<TableColumnAndValue> iterator = info.getIterator(); iterator.hasValue(); iterator.advance())
		// Add string
		strings += CString(OSSTR("`")) + iterator->mTableColumn.getName() + CString(OSSTR("` = ?"));

	CString	statement =
					CString(OSSTR("UPDATE `")) + mInternals->mName + CString(OSSTR("` SET ")) +
							CString(strings) + where.getString();

	TNArray<SSQLiteValue>	values(info, sValueFromTableColumnAndValue);
	values += where.getValues();

	// Perform
	mInternals->mStatementPerformer.addToTransactionOrPerform(statement, values);
}

//----------------------------------------------------------------------------------------------------------------------
void CSQLiteTable::deleteRows(const CSQLiteTableColumn& tableColumn, const TArray<SSQLiteValue>& values)
//----------------------------------------------------------------------------------------------------------------------
{
	// Perform in chunks of SQLITE_LIMIT_VARIABLE_NUMBER
	TArray<TArray<SSQLiteValue> >	valuesChunks =
											SSQLiteValue::chunked(values,
													mInternals->mStatementPerformer.getVariableNumberLimit());

	// Iterate values chunks
	for (TIteratorD<TArray<SSQLiteValue> > iterator = valuesChunks.getIterator(); iterator.hasValue();
			iterator.advance()) {
		// Setup
		CString	statement =
					CString(OSSTR("DELETE FROM `")) + mInternals->mName + CString(OSSTR("` WHERE `")) +
							tableColumn.getName() + CString(OSSTR("` IN (")) +
							CString(TNArray<CString>(CString(OSSTR("?")), iterator->getCount()), CString(OSSTR(","))) +
							CString(OSSTR(")"));

		// Perform
		mInternals->mStatementPerformer.addToTransactionOrPerform(statement, *iterator);
	}
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions

//----------------------------------------------------------------------------------------------------------------------
CSQLiteTableColumn sTableColumnFromTableColumnAndValue(CArray::ItemRef item)
//----------------------------------------------------------------------------------------------------------------------
{
	return ((CSQLiteTable::TableColumnAndValue&) item).mTableColumn;
}

//----------------------------------------------------------------------------------------------------------------------
SSQLiteValue sValueFromTableColumnAndValue(CArray::ItemRef item)
//----------------------------------------------------------------------------------------------------------------------
{
	return ((CSQLiteTable::TableColumnAndValue&) item).mValue;
}

//----------------------------------------------------------------------------------------------------------------------
void sStoreLastInsertRowID(SInt64 lastRowID, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	*((SInt64*) userData) = lastRowID;
}
