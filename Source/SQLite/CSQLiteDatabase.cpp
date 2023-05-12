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
		Internals(sqlite3* database, CSQLiteDatabase::Options options) :
			mDatabase(database), mStatementPerformer(database)
			{
				// Finish setup
				if (options & CSQLiteDatabase::kWALMode)
					// Activate WAL mode
					sqlite3_exec(mDatabase, "PRAGMA journal_mode = WAL;", nil, nil, nil);
			}
		~Internals()
			{
				sqlite3_close(mDatabase);
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
	int			result = sqlite3_open(*file.getFilesystemPath().getString().getCString(), &database);
	if (result != SQLITE_OK) {
		// Error
		CLogServices::logError(
				CString(OSSTR("CSQLiteDatabase failed to open with ")) + CString(result) +
						CString(OSSTR(" (\"")) + CString(sqlite3_errmsg(database)) + CString(OSSTR("\")")));
		AssertFail();
	}

	// Setup internals
	mInternals = new Internals(database, options);
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
	int			result = sqlite3_open(*file.getFilesystemPath().getString().getCString(), &database);
	if (result != SQLITE_OK) {
		// Error
		CLogServices::logError(
				CString(OSSTR("CSQLiteDatabase failed to open with ")) + CString(result) +
						CString(OSSTR(" (\"")) + CString(sqlite3_errmsg(database)) + CString(OSSTR("\")")));
		AssertFail();
	}

	// Setup internals
	mInternals = new Internals(database, options);
}

//----------------------------------------------------------------------------------------------------------------------
CSQLiteDatabase::~CSQLiteDatabase()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

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
	// Perform as transaction
	mInternals->mStatementPerformer.performAsTransaction((CSQLiteStatementPerformer::TransactionProc) transactionProc,
			userData);
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
