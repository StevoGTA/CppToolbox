//----------------------------------------------------------------------------------------------------------------------
//	CSQLiteStatementPerformer.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CSQLiteResultsRow.h"
#include "CSQLiteTableColumn.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSQLiteStatementPerformer

class CSQLiteStatementPerformer {
	// Transaction result
	public:
		enum TransactionResult {
			kCommit,
			kRollback,
		};

	// Procs
	public:
		typedef	void				(*LastInsertRowIDProc)(SInt64 lastRowID, void* userData);
		typedef	TransactionResult	(*TransactionProc)(void* userData);

	// Classes
	private:
		class Internals;

	// Methods
	public:
							// Lifecycle methods
							CSQLiteStatementPerformer(sqlite3* database);
							~CSQLiteStatementPerformer();

							// Instance methods
		void				addToTransactionOrPerform(const CString& statement,
									const TArray<SSQLiteValue>& values = TNArray<SSQLiteValue>(),
									LastInsertRowIDProc lastInsertRowIDProc = nil, void* userData = nil);

		CSQLiteResultsRow	perform(const CString& statement, const TArray<SSQLiteValue>& values);
		CSQLiteResultsRow	perform(const CString& statement)
								{ return perform(statement, TNArray<SSQLiteValue>()); }

		void				performAsTransaction(TransactionProc transactionProc, void* userData);

		SInt32				getVariableNumberLimit();

	// Properties
	private:
		Internals*	mInternals;
};
