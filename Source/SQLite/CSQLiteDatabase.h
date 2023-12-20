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
			kOptionsNone	= 0,
			kOptionsWALMode	= 1 << 0,
		};

	// TransactionResult
	public:
		enum TransactionResult {
			kTransactionResultCommit,
			kTransactionResultRollback,
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
										CSQLiteDatabase(const CFile& file, Options options = kOptionsWALMode);
										CSQLiteDatabase(const CFolder& folder,
												const CString& name = CString(OSSTR("database")),
												Options options = kOptionsWALMode);
										~CSQLiteDatabase();

										// Instance methods
				TArray<CSQLiteTable>	getAllTables() const;
				CSQLiteTable			getTable(const CString& name, CSQLiteTable::Options options,
												const TArray<CSQLiteTableColumn>& tableColumns,
												const TArray<CSQLiteTableColumn::Reference>& references =
														TNArray<CSQLiteTableColumn::Reference>());
				CSQLiteTable			getTable(const CString& name, const TArray<CSQLiteTableColumn>& tableColumns,
												const TArray<CSQLiteTableColumn::Reference>& references =
														TNArray<CSQLiteTableColumn::Reference>())
											{ return getTable(name, CSQLiteTable::kOptionsNone, tableColumns,
													references); }
				void					performAsTransaction(TransactionProc transactionProc, void* userData);

										// Class methods
		static	bool					doesExist(const CFolder& folder,
												const CString& name = CString(OSSTR("database")));

	// Properties
	private:
		Internals*	mInternals;
};
