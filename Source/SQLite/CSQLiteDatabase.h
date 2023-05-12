//----------------------------------------------------------------------------------------------------------------------
//	CSQLiteDatabase.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CFolder.h"
#include "CSQLiteTable.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSQLiteDatabase

class CSQLiteDatabase {
	// Options
	public:
		enum Options {
			kNone	 	= 0,
			kWALMode	= 1 << 0,
		};

	// TransactionResult
	public:
		enum TransactionResult {
			kCommit,
			kRollback,
		};

	// Procs
	public:
		typedef	TransactionResult	(*TransactionProc)(void* userData);

	// Classes
	private:
		class Internals;

	// Methods
	public:
								// Lifecycle methods
								CSQLiteDatabase(const CFile& file, Options options = kWALMode);
								CSQLiteDatabase(const CFolder& folder, const CString& name = CString(OSSTR("database")),
										Options options = kWALMode);
								~CSQLiteDatabase();

								// Instance methods
				CSQLiteTable	getTable(const CString& name, CSQLiteTable::Options options,
										const TArray<CSQLiteTableColumn>& tableColumns,
										const TArray<CSQLiteTableColumn::Reference>& references =
												TNArray<CSQLiteTableColumn::Reference>());
				void			performAsTransaction(TransactionProc transactionProc, void* userData);

								// Class methods
		static	bool			doesExist(const CFolder& folder, const CString& name = CString(OSSTR("database")));

	// Properties
	private:
		Internals*	mInternals;
};
