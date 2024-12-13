//----------------------------------------------------------------------------------------------------------------------
//	CSQLiteStatementPerformer.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CSQLiteStatementPerformer.h"

#include "CDictionary.h"
#include "ConcurrencyPrimitives.h"
#include "CLogServices.h"
#include "CSQLiteTableColumn.h"
#include "CThread.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Types

typedef	CSQLiteStatementPerformer::LastInsertRowIDProc	LastInsertRowIDProc;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: CSQLiteStatement

class CSQLiteStatement {
	// Methods
	public:
					CSQLiteStatement(const CString& statement, const TArray<SSQLiteValue>& values,
							LastInsertRowIDProc lastInsertRowIDProc, void* userData) :
						mString(statement), mValues(values), mLastInsertRowIDProc(lastInsertRowIDProc),
								mSQLiteResultsRowProc(nil), mUserData(userData)
						{}
					CSQLiteStatement(const CString& statement, const TArray<SSQLiteValue>& values,
							CSQLiteResultsRow::Proc resultsRowProc, void* userData) :
						mString(statement), mValues(values), mLastInsertRowIDProc(nil),
								mSQLiteResultsRowProc(resultsRowProc), mUserData(userData)
						{}
					CSQLiteStatement(const CString& statement) :
						mString(statement), mValues(TNArray<SSQLiteValue>()), mLastInsertRowIDProc(nil),
								mSQLiteResultsRowProc(nil), mUserData(nil)
						{}
					CSQLiteStatement(const CSQLiteStatement& other) :
						mString(other.mString), mValues(other.mValues),
								mLastInsertRowIDProc(other.mLastInsertRowIDProc),
								mSQLiteResultsRowProc(other.mSQLiteResultsRowProc),
								mUserData(other.mUserData)
						{}

		OV<SError>	perform(sqlite3* database)
						{
							// Prepare
							sqlite3_stmt*	statement;
							if (sqlite3_prepare_v2(database, *mString.getUTF8String(), -1, &statement, nil) !=
									SQLITE_OK) {
								// Error
								CLogServices::logError(
										CString(OSSTR("SQLiteStatement could not prepare query with \"")) + mString +
										CString(OSSTR("\", with error \"")) + CString(sqlite3_errmsg(database)) +
										CString(OSSTR("\"")));
								AssertFail();
							}

							// Bind values
							for (CArray::ItemIndex i = 0; i < mValues.getCount(); i++) {
								// Setup
								const	SSQLiteValue&	value = mValues[i];

								// Check value type
								switch (value.getType()) {
									case SSQLiteValue::kTypeData:
										// Data
										sqlite3_bind_blob(statement, i + 1, value.getData().getBytePtr(),
												(int) value.getData().getByteCount(), SQLITE_STATIC);
										break;

									case SSQLiteValue::kTypeFloat64:
										// Float64
										sqlite3_bind_double(statement, i + 1, value.getFloat64());
										break;

									case SSQLiteValue::kTypeSInt64:
										// Int64
										sqlite3_bind_int64(statement, i + 1, value.getSInt64());
										break;

									case SSQLiteValue::kTypeString:
										// String
										sqlite3_bind_text(statement, i + 1, *value.getString().getUTF8String(), -1,
												SQLITE_TRANSIENT);
										break;

									case SSQLiteValue::kTypeLastInsertRowID:
										// Last insert row ID
										sqlite3_bind_int64(statement, i + 1,
												sqlite3_last_insert_rowid(database));
										break;

									case SSQLiteValue::kTypeNull:
										// Null
										sqlite3_bind_null(statement, i + 1);
										break;
								}
							}

							// Perform
							if (mSQLiteResultsRowProc != nil) {
								// Perform as query
								CSQLiteResultsRow	sqliteResultsRow(statement);
								while (sqlite3_step(statement) == SQLITE_ROW) {
									// Call proc
									OV<SError>	error = mSQLiteResultsRowProc(sqliteResultsRow, mUserData);
									ReturnErrorIfError(error);
								}

								return OV<SError>();
							} else {
								// Perform as step
								if (sqlite3_step(statement) != SQLITE_DONE) {
									// Error
									CLogServices::logError(
											CString(OSSTR("SQLiteStatement could not prepare query with \""))
											+ mString + CString(OSSTR("\", with error \"")) +
											CString(sqlite3_errmsg(database)) + CString(OSSTR("\"")));
									AssertFail();
								}

								// Check for last insert row ID proc
								if (mLastInsertRowIDProc != nil)
									// Call proc
									mLastInsertRowIDProc(sqlite3_last_insert_rowid(database), mUserData);

								return OV<SError>();
							}
						}

		CString					mString;
		TArray<SSQLiteValue>	mValues;
		LastInsertRowIDProc		mLastInsertRowIDProc;
		CSQLiteResultsRow::Proc	mSQLiteResultsRowProc;
		void*					mUserData;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSQLiteStatementPerformer::Internals

class CSQLiteStatementPerformer::Internals {
	public:
		Internals(sqlite3* database) : mDatabase(database) {}

		sqlite3*									mDatabase;
		TNDictionary<TNArray<CSQLiteStatement> >	mTransactionsMap;
		CReadPreferringLock							mTransacftionsMapLock;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSQLiteStatementPerformer

// MARK: Lifecycle methods
//----------------------------------------------------------------------------------------------------------------------
CSQLiteStatementPerformer::CSQLiteStatementPerformer(sqlite3* database)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(database);
}

//----------------------------------------------------------------------------------------------------------------------
CSQLiteStatementPerformer::~CSQLiteStatementPerformer()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CSQLiteStatementPerformer::addToTransactionOrPerform(const CString& statement, const TArray<SSQLiteValue>& values,
		LastInsertRowIDProc lastInsertRowIDProc, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CSQLiteStatement	sqliteStatement(statement, values, lastInsertRowIDProc, userData);
	CString				currentThreadRef = CThread::getCurrentRefAsString();

	// Check for transaction
	mInternals->mTransacftionsMapLock.lockForReading();
	OR<TNArray<CSQLiteStatement> >	sqliteStatements = mInternals->mTransactionsMap[currentThreadRef];
	mInternals->mTransacftionsMapLock.unlockForReading();

	if (sqliteStatements.hasReference()) {
		// In transaction
		*sqliteStatements += sqliteStatement;

		mInternals->mTransacftionsMapLock.lockForWriting();
		mInternals->mTransactionsMap.set(currentThreadRef, *sqliteStatements);
		mInternals->mTransacftionsMapLock.unlockForWriting();
	} else
		// Perform
		sqliteStatement.perform(mInternals->mDatabase);
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CSQLiteStatementPerformer::perform(const CString& statement, const TArray<SSQLiteValue>& values,
		CSQLiteResultsRow::Proc resultsRowProc, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Perform
	OV<SError>	error = CSQLiteStatement(statement, values, resultsRowProc, userData).perform(mInternals->mDatabase);

	return error;
}

//----------------------------------------------------------------------------------------------------------------------
void CSQLiteStatementPerformer::performAsTransaction(TransactionProc transactionProc, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CString	currentThreadRef = CThread::getCurrentRefAsString();

	// Internals check
	mInternals->mTransacftionsMapLock.lockForReading();
	bool	inTransaction = mInternals->mTransactionsMap[currentThreadRef].hasReference();
	mInternals->mTransacftionsMapLock.unlockForReading();
	if (inTransaction) {
		// Error
		CLogServices::logError(
				CString(OSSTR("SQLiteStatementPerformer performAsTransaction() called while already in transaction")));
		AssertFail();
	}

	// Start transaction
	mInternals->mTransacftionsMapLock.lockForWriting();
	mInternals->mTransactionsMap.set(currentThreadRef,
			TNArray<CSQLiteStatement>(CSQLiteStatement(CString(OSSTR("BEGIN TRANSACTION")))));
	mInternals->mTransacftionsMapLock.unlockForWriting();

	// Call proc and check result
	if (transactionProc(userData) == kTransactionResultCommit) {
		// End transaction
		mInternals->mTransacftionsMapLock.lockForWriting();
		TNArray<CSQLiteStatement>	sqliteStatements = *mInternals->mTransactionsMap[currentThreadRef];
		mInternals->mTransactionsMap.remove(currentThreadRef);
		mInternals->mTransacftionsMapLock.unlockForWriting();

		// Check for empty transaction
		if (sqliteStatements.getCount() == 1)
			// No statements
			return;

		// Add COMMIT
		sqliteStatements += CSQLiteStatement(CString(OSSTR("COMMIT")));

		// Perform
		for (TIteratorD<CSQLiteStatement> iterator = sqliteStatements.getIterator(); iterator.hasValue();
				iterator.advance())
			// Perform
			iterator->perform(mInternals->mDatabase);
	} else {
		// No longer in transaction
		mInternals->mTransacftionsMapLock.lockForWriting();
		mInternals->mTransactionsMap.remove(currentThreadRef);
		mInternals->mTransacftionsMapLock.unlockForWriting();
	}
}

//----------------------------------------------------------------------------------------------------------------------
SInt32 CSQLiteStatementPerformer::getVariableNumberLimit()
//----------------------------------------------------------------------------------------------------------------------
{
	return sqlite3_limit(mInternals->mDatabase, SQLITE_LIMIT_VARIABLE_NUMBER, -1);
}
