//----------------------------------------------------------------------------------------------------------------------
//	CSQLiteWhere.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CSQLiteTableColumn.h"
#include "SSQLiteValue.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSQLiteWhere

class CSQLiteWhereInternals;
class CSQLiteWhere {
	// ValueGroup
	public:
		struct ValueGroup {
			// Lifecycle methods
			ValueGroup(const CString& string, const TArray<SSQLiteValue>& values) : mString(string), mValues(values) {}
			ValueGroup(const ValueGroup& other) : mString(other.mString), mValues(other.mValues) {}

			// Properties
			CString					mString;
			TArray<SSQLiteValue>	mValues;
		};

	// Methods
	public:
										// Lifecycle methods
										CSQLiteWhere(const CSQLiteTable& table, const CSQLiteTableColumn& tableColumn,
												const CString& comparison, const SSQLiteValue& value);
										CSQLiteWhere(const CSQLiteTable& table, const CSQLiteTableColumn& tableColumn,
												const SSQLiteValue& value);
										CSQLiteWhere(const CSQLiteTableColumn& tableColumn, const CString& comparison,
												const SSQLiteValue& value);
										CSQLiteWhere(const CSQLiteTableColumn& tableColumn, const SSQLiteValue& value);
										CSQLiteWhere(const CSQLiteTable& table, const CSQLiteTableColumn& tableColumn,
												const TArray<SSQLiteValue>& values);
										CSQLiteWhere(const CSQLiteTableColumn& tableColumn,
												const TArray<SSQLiteValue>& values);
										~CSQLiteWhere();

										// Instance methods
		const	CString&				getString() const;
		const	TArray<SSQLiteValue>&	getValues() const;

				TArray<ValueGroup>		getValueGroups(UInt32 groupSize) const;

				CSQLiteWhere&			addAnd(const CSQLiteTable& table, const CSQLiteTableColumn& tableColumn,
												const CString& comparison, const SSQLiteValue& value);
				CSQLiteWhere&			addAnd(const CSQLiteTable& table, const CSQLiteTableColumn& tableColumn,
												const SSQLiteValue& value)
											{ return addAnd(table, tableColumn, CString(OSSTR("=")), value); }
				CSQLiteWhere&			addAnd(const CSQLiteTableColumn& tableColumn, const CString& comparison,
												const SSQLiteValue& value);
				CSQLiteWhere&			addAnd(const CSQLiteTableColumn& tableColumn, const SSQLiteValue& value)
											{ return addAnd(tableColumn, CString(OSSTR("=")), value); }
				CSQLiteWhere&			addAnd(const CSQLiteTable& table, const CSQLiteTableColumn& tableColumn,
												const TArray<SSQLiteValue>& values);
				CSQLiteWhere&			addAnd(const CSQLiteTableColumn& tableColumn,
												const TArray<SSQLiteValue>& values);
				CSQLiteWhere&			addOr(const CSQLiteTable& table, const CSQLiteTableColumn& tableColumn,
												const CString& comparison, const SSQLiteValue& value);
				CSQLiteWhere&			addOr(const CSQLiteTable& table, const CSQLiteTableColumn& tableColumn,
												const SSQLiteValue& value)
											{ return addOr(table, tableColumn, CString(OSSTR("=")), value);}
				CSQLiteWhere&			addOr(const CSQLiteTableColumn& tableColumn, const CString& comparison,
												const SSQLiteValue& value);
				CSQLiteWhere&			addOr(const CSQLiteTableColumn& tableColumn, const SSQLiteValue& value)
											{ return addOr(tableColumn, CString(OSSTR("=")), value); }

	// Properties
	private:
		CSQLiteWhereInternals*	mInternals;
};
