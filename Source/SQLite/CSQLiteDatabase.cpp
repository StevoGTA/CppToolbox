//----------------------------------------------------------------------------------------------------------------------
//	CSQLiteDatabase.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CSQLiteDatabase.h"

#include "CFile.h"
#include "CLogServices.h"
#include "CSQLiteStatementPerformer.h"

#include <sqlite3.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSQLiteDatabase::Internals

class CSQLiteDatabase::Internals {
	public:
		struct TransactionInfo {
			public:
									TransactionInfo(TransactionProc transactionProc, void* userData) :
										mTransactionProc(transactionProc), mUserData(userData)
										{}

				TransactionResult	perform()
										{ return mTransactionProc(mUserData); }

			private:
				TransactionProc	mTransactionProc;
				void*			mUserData;
		};

																Internals(sqlite3* database,
																		CSQLiteDatabase::Options options) :
																	mDatabase(database), mStatementPerformer(database)
																	{}
																~Internals()
																	{
																		sqlite3_close(mDatabase);
																	}

		static	CSQLiteStatementPerformer::TransactionResult	performAsTransaction(TransactionProc transactionProc,
																		TransactionInfo* transactionInfo)
																	{
																		// Perform
																		TransactionResult	transactionResult =
																									transactionInfo->
																											perform();

																		// Handle result
																		switch (transactionResult) {
																			case kTransactionResultCommit:
																					return CSQLiteStatementPerformer::
																							kTransactionResultCommit;

																			case kTransactionResultRollback:
																					return CSQLiteStatementPerformer::
																							kTransactionResultRollback;
																		}
																	}

		sqlite3*					mDatabase;
		CSQLiteStatementPerformer	mStatementPerformer;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSQLiteDatabase

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CSQLiteDatabase::CSQLiteDatabase(const CFile& file, Options options)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup database
	sqlite3*	database;
	int			result =
						sqlite3_open_v2(*file.getFilesystemPath().getString().getUTF8String(), &database,
								SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, nil);
	if (result != SQLITE_OK) {
		// Error
		CLogServices::logError(
				CString(OSSTR("CSQLiteDatabase failed to open with ")) + CString(result) +
						CString(OSSTR(" (\"")) + CString(sqlite3_errstr(result)) + CString(OSSTR("\")")));
		AssertFail();
	}

	// Setup internals
	mInternals = new Internals(database, options);

	// Check options
	if (options & kOptionsWALMode)
		// Activate WAL mode
		sqlite3_exec(database, "PRAGMA journal_mode = WAL;", nil, nil, nil);
}

//----------------------------------------------------------------------------------------------------------------------
CSQLiteDatabase::CSQLiteDatabase(const CFolder& folder, const CString& name, Options options)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFilesystemPath	folderPath = folder.getFilesystemPath();
	CFile			file(folderPath.appendingComponent(name).appendingExtension(CString(OSSTR("sqlite3"))));
	if (!file.doesExist())
		file = CFile(folderPath.appendingComponent(name).appendingExtension(CString(OSSTR("sqlite"))));

	// Setup database
	sqlite3*	database;
	int			result =
						sqlite3_open_v2(*file.getFilesystemPath().getString().getUTF8String(), &database,
								SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_FULLMUTEX, nil);
	if (result != SQLITE_OK) {
		// Error
		CLogServices::logError(
				CString(OSSTR("CSQLiteDatabase failed to open with ")) + CString(result) +
						CString(OSSTR(" (\"")) + CString(sqlite3_errstr(result)) + CString(OSSTR("\")")));
		AssertFail();
	}

	// Setup internals
	mInternals = new Internals(database, options);

	// Check options
	if (options & kOptionsWALMode)
		// Activate WAL mode
		sqlite3_exec(database, "PRAGMA journal_mode = WAL;", nil, nil, nil);
}

//----------------------------------------------------------------------------------------------------------------------
CSQLiteDatabase::~CSQLiteDatabase()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
TArray<CSQLiteTable> CSQLiteDatabase::getAllTables() const
//----------------------------------------------------------------------------------------------------------------------
{
	return CSQLiteTable::getAll(mInternals->mStatementPerformer);
}

//----------------------------------------------------------------------------------------------------------------------
CSQLiteTable CSQLiteDatabase::getTable(const CString& name, CSQLiteTable::Options options,
		const TArray<CSQLiteTableColumn>& tableColumns, const TArray<CSQLiteTableColumn::Reference>& references)
//----------------------------------------------------------------------------------------------------------------------
{
	// Return table
	return CSQLiteTable(name, options, tableColumns, references, mInternals->mStatementPerformer);
}

//----------------------------------------------------------------------------------------------------------------------
void CSQLiteDatabase::performAsTransaction(TransactionProc transactionProc, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	Internals::TransactionInfo	transactionInfo(transactionProc, userData);

	// Perform as transaction
	mInternals->mStatementPerformer.performAsTransaction(
			(CSQLiteStatementPerformer::TransactionProc) Internals::performAsTransaction, &transactionInfo);
}

// Class methods

//----------------------------------------------------------------------------------------------------------------------
bool CSQLiteDatabase::doesExist(const CFolder& folder, const CString& name)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CFilesystemPath	folderPath = folder.getFilesystemPath();

	return CFile(folderPath.appendingComponent(name).appendingExtension(CString(OSSTR("sqlite")))).doesExist() ||
			CFile(folderPath.appendingComponent(name).appendingExtension(CString(OSSTR("sqlite3")))).doesExist();
}
