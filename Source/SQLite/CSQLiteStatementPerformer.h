//----------------------------------------------------------------------------------------------------------------------
//	CSQLiteStatementPerformer.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CSQLiteResultsRow.h"
#include "CSQLiteTableColumn.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSQLiteStatementPerformer

class CSQLiteStatementPerformer {
	// TransactionResult
	public:
		enum TransactionResult {
			kTransactionResultCommit,
			kTransactionResultRollback,
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
		void		addToTransactionOrPerform(const CString& statement,
							const TArray<SSQLiteValue>& values = TNArray<SSQLiteValue>(),
							LastInsertRowIDProc lastInsertRowIDProc = nil, void* userData = nil);

		OV<SError>	perform(const CString& statement, const TArray<SSQLiteValue>& values,
							CSQLiteResultsRow::Proc resultsRowProc, void* userData);
		OV<SError>	perform(const CString& statement, CSQLiteResultsRow::Proc resultsRowProc, void* userData)
						{ return perform(statement, TNArray<SSQLiteValue>(), resultsRowProc, userData); }

		void		performAsTransaction(TransactionProc transactionProc, void* userData);

		SInt32		getVariableNumberLimit();

	// Properties
	private:
		Internals*	mInternals;
};
