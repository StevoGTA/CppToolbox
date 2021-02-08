//----------------------------------------------------------------------------------------------------------------------
//	CSQLiteWhere.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CSQLiteWhere.h"

#include "CSQLiteTable.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CString	sVariablePlaceholder(OSSTR("##VARIABLEPLACEHOLDER##"));

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSQLiteWhereInternals

class CSQLiteWhereInternals {
	public:
				CSQLiteWhereInternals(const CString& string, const TArray<SSQLiteValue>& values) :
					mString(string), mValues(values)
					{}
				CSQLiteWhereInternals(const CString& string) : mString(string) {}

		void	append(const CString& comparison, const SSQLiteValue& value)
					{
						// Check value type
						if (value.mType == SSQLiteValue::kNull) {
							// Value is NULL
							if (comparison == CString(OSSTR("=")))
								// IS NULL
								mString += " IS NULL";
							else if (comparison == CString(OSSTR("!=")))
								// IS NOT NULL
								mString += " IS NOT NULL";
							else
								// Unsupported NULL comparison
								AssertFail();
						} else if (value.mType != SSQLiteValue::kLastInsertRowID) {
							// Actual value
							mString += CString::mSpace + comparison + CString(OSSTR(" ?"));
							mValues += value;
						} else
							// Unsupported operation
							AssertFail();
					}

		CString					mString;
		TNArray<SSQLiteValue>	mValues;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSQLiteWhere

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CSQLiteWhere::CSQLiteWhere(const CSQLiteTable& table, const CSQLiteTableColumn& tableColumn, const CString& comparison,
		const SSQLiteValue& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals =
			new CSQLiteWhereInternals(
					CString(OSSTR(" WHERE `")) + table.getName() + CString(OSSTR("`.`")) + tableColumn.getName() +
							CString(OSSTR("`")));
	mInternals->append(comparison, value);
}

//----------------------------------------------------------------------------------------------------------------------
CSQLiteWhere::CSQLiteWhere(const CSQLiteTable& table, const CSQLiteTableColumn& tableColumn, const SSQLiteValue& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals =
			new CSQLiteWhereInternals(
					CString(OSSTR(" WHERE `")) + table.getName() + CString(OSSTR("`.`")) + tableColumn.getName() +
							CString(OSSTR("`")));
	mInternals->append(CString(OSSTR("=")), value);
}

//----------------------------------------------------------------------------------------------------------------------
CSQLiteWhere::CSQLiteWhere(const CSQLiteTableColumn& tableColumn, const CString& comparison, const SSQLiteValue& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new CSQLiteWhereInternals(CString(OSSTR(" WHERE `")) + tableColumn.getName() + CString(OSSTR("`")));
	mInternals->append(comparison, value);
}

//----------------------------------------------------------------------------------------------------------------------
CSQLiteWhere::CSQLiteWhere(const CSQLiteTableColumn& tableColumn, const SSQLiteValue& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new CSQLiteWhereInternals(CString(OSSTR(" WHERE `")) + tableColumn.getName() + CString(OSSTR("`")));
	mInternals->append(CString(OSSTR("=")), value);
}

//----------------------------------------------------------------------------------------------------------------------
CSQLiteWhere::CSQLiteWhere(const CSQLiteTable& table, const CSQLiteTableColumn& tableColumn,
		const TArray<SSQLiteValue>& values)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals =
			new CSQLiteWhereInternals(
					CString(OSSTR(" WHERE `")) + table.getName() + CString(OSSTR("`.`")) + tableColumn.getName() +
							CString(OSSTR("` IN (")) + sVariablePlaceholder + CString(OSSTR(")")),
					values);
}

//----------------------------------------------------------------------------------------------------------------------
CSQLiteWhere::CSQLiteWhere(const CSQLiteTableColumn& tableColumn, const TArray<SSQLiteValue>& values)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals =
			new CSQLiteWhereInternals(
					CString(OSSTR(" WHERE `")) + tableColumn.getName() + CString(OSSTR("` IN (")) +
							sVariablePlaceholder + CString(OSSTR(")")),
					values);
}

//----------------------------------------------------------------------------------------------------------------------
CSQLiteWhere::~CSQLiteWhere()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
const CString& CSQLiteWhere::getString() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mString;
}

//----------------------------------------------------------------------------------------------------------------------
const TArray<SSQLiteValue>& CSQLiteWhere::getValues() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mValues;
}

//----------------------------------------------------------------------------------------------------------------------
TArray<CSQLiteWhere::ValueGroup> CSQLiteWhere::getValueGroups(UInt32 groupSize) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TArray<TArray<SSQLiteValue> >	valuesChunks = SSQLiteValue::chunked(mInternals->mValues, groupSize);

	// Iterate values chunks
	TNArray<ValueGroup>	valueGroups;
	for (TIteratorD<TArray<SSQLiteValue> > iterator = valuesChunks.getIterator(); iterator.hasValue();
			iterator.advance()) {
		// Transmogrify string
		CString	string =
						mInternals->mString.replacingSubStrings(sVariablePlaceholder,
								CString(TNArray<CString>(CString(OSSTR("?")), iterator->getCount()),
										CString(OSSTR(","))));

		// Add value group
		valueGroups += ValueGroup(string, *iterator);
	}

	return valueGroups;
}

//----------------------------------------------------------------------------------------------------------------------
CSQLiteWhere& CSQLiteWhere::addAnd(const CSQLiteTable& table, const CSQLiteTableColumn& tableColumn,
		const CString& comparison, const SSQLiteValue& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Append
	mInternals->mString +=
			CString(OSSTR(" AND `")) + table.getName() + CString(OSSTR("`.`")) + tableColumn.getName() +
					CString(OSSTR("`"));
	mInternals->append(comparison, value);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CSQLiteWhere& CSQLiteWhere::addAnd(const CSQLiteTableColumn& tableColumn, const CString& comparison,
		const SSQLiteValue& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Append
	mInternals->mString += CString(OSSTR(" AND `")) + tableColumn.getName() + CString(OSSTR("`"));
	mInternals->append(comparison, value);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CSQLiteWhere& CSQLiteWhere::addAnd(const CSQLiteTable& table, const CSQLiteTableColumn& tableColumn,
		const TArray<SSQLiteValue>& values)
//----------------------------------------------------------------------------------------------------------------------
{
	// Append
	mInternals->mString +=
			CString(OSSTR(" WHERE `")) + table.getName() + CString(OSSTR("`.`")) + tableColumn.getName() +
					CString(OSSTR("`")) + CString(OSSTR(" IN (")) + sVariablePlaceholder + CString(OSSTR(")"));
	mInternals->mValues = values;

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CSQLiteWhere& CSQLiteWhere::addAnd(const CSQLiteTableColumn& tableColumn, const TArray<SSQLiteValue>& values)
//----------------------------------------------------------------------------------------------------------------------
{
	// Append
	mInternals->mString +=
			CString(OSSTR(" WHERE `")) + tableColumn.getName() + CString(OSSTR("`")) + CString(OSSTR(" IN (")) +
					sVariablePlaceholder + CString(OSSTR(")"));
	mInternals->mValues = values;

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CSQLiteWhere& CSQLiteWhere::addOr(const CSQLiteTable& table, const CSQLiteTableColumn& tableColumn,
		const CString& comparison, const SSQLiteValue& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Append
	mInternals->mString +=
			CString(OSSTR(" OR `")) + table.getName() + CString(OSSTR("`.`")) + tableColumn.getName() +
					CString(OSSTR("`"));
	mInternals->append(comparison, value);

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
CSQLiteWhere& CSQLiteWhere::addOr(const CSQLiteTableColumn& tableColumn, const CString& comparison,
		const SSQLiteValue& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Append
	mInternals->mString += CString(OSSTR(" OR `")) + tableColumn.getName() + CString(OSSTR("`"));
	mInternals->append(comparison, value);

	return *this;
}
