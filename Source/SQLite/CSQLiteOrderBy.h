//----------------------------------------------------------------------------------------------------------------------
//	CSQLiteOrderBy.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CSQLiteTableColumn.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSQLiteOrderBy

class CSQLiteOrderBy {
	// Order
	enum Order {
		kAscending,
		kDescending,
	};

	// Classes
	private:
		class Internals;

	// Methods
	public:
							// Lifecycle methods
							CSQLiteOrderBy(const CSQLiteTable& table, CSQLiteTableColumn& tableColumn,
									Order order = kAscending);
							CSQLiteOrderBy(CSQLiteTableColumn& tableColumn, Order order = kAscending);
							~CSQLiteOrderBy();

							// Instance methods
		const	CString&	getString() const;

	// Properties
	private:
		Internals*	mInternals;
};
