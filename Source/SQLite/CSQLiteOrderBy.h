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
		kOrderAscending,
		kOrderDescending,
	};

	// Classes
	private:
		class Internals;

	// Methods
	public:
							// Lifecycle methods
							CSQLiteOrderBy(const CSQLiteTable& table, const CSQLiteTableColumn& tableColumn,
									Order order = kOrderAscending);
							CSQLiteOrderBy(const CSQLiteTableColumn& tableColumn, Order order = kOrderAscending);
							~CSQLiteOrderBy();

							// Instance methods
		const	CString&	getString() const;

	// Properties
	private:
		Internals*	mInternals;
};
