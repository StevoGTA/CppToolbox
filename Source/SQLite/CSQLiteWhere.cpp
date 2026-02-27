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
// MARK: - CSQLiteWhere::Internals

class CSQLiteWhere::Internals {
	public:
				Internals(const CString& string, const TArray<SSQLiteValue>& values) :
					mString(string), mValues(values)
					{}
				Internals(const CString& string) : mString(string) {}

		void	append(const CString& comparison, const SSQLiteValue& value)
					{
						// Check value type
						if (value.getType() == SSQLiteValue::kTypeNull) {
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
						} else if (value.getType() != SSQLiteValue::kTypeLastInsertRowID) {
							// Actual value
							mString += CString::mSpace + comparison + CString(OSSTR(" ?"));
							mValues += TNArray<SSQLiteValue>(value);
						} else
							// Unsupported operation
							AssertFail();
					}

		CString							mString;
		TNArray<TArray<SSQLiteValue> >	mValues;
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
			new Internals(
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
			new Internals(
					CString(OSSTR(" WHERE `")) + table.getName() + CString(OSSTR("`.`")) + tableColumn.getName() +
							CString(OSSTR("`")));
	mInternals->append(CString(OSSTR("=")), value);
}

//----------------------------------------------------------------------------------------------------------------------
CSQLiteWhere::CSQLiteWhere(const CSQLiteTableColumn& tableColumn, const CString& comparison, const SSQLiteValue& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new Internals(CString(OSSTR(" WHERE `")) + tableColumn.getName() + CString(OSSTR("`")));
	mInternals->append(comparison, value);
}

//----------------------------------------------------------------------------------------------------------------------
CSQLiteWhere::CSQLiteWhere(const CSQLiteTableColumn& tableColumn, const SSQLiteValue& value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new Internals(CString(OSSTR(" WHERE `")) + tableColumn.getName() + CString(OSSTR("`")));
	mInternals->append(CString(OSSTR("=")), value);
}

//----------------------------------------------------------------------------------------------------------------------
CSQLiteWhere::CSQLiteWhere(const CSQLiteTable& table, const CSQLiteTableColumn& tableColumn,
		const TArray<SSQLiteValue>& values)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals =
			new Internals(
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
			new Internals(
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
TArray<CSQLiteWhere::ValueGroup> CSQLiteWhere::getValueGroups(UInt32 groupSize) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	TNArray<ValueGroup>	valueGroups;

	// Check if need to group
	if (!mInternals->mString.contains(sVariablePlaceholder)) {
		// Single group
		TNArray<SSQLiteValue>	values;
		for (TArray<TArray<SSQLiteValue> >::Iterator iterator = mInternals->mValues.getIterator(); iterator; iterator++)
			// Append
			values += *iterator;
		valueGroups += ValueGroup(mInternals->mString, values);
	} else {
		// Group
		TNArray<SSQLiteValue>	preValueGroupValues;
		TNArray<SSQLiteValue>	valueGroup;
		TNArray<SSQLiteValue>	postValueGroupValues;
		for (TArray<TArray<SSQLiteValue> >::Iterator iterator = mInternals->mValues.getIterator(); iterator;
				iterator++) {
			// Check count
			if (iterator->getCount() == 1) {
				// Single value
				if (valueGroup.isEmpty())
					// Pre
					preValueGroupValues += *iterator;
				else
					// Post
					postValueGroupValues += *iterator;
			} else
				// Value group
				valueGroup = *iterator;
		}

		// Check if need to group
		TNArray<SSQLiteValue>	allValues = preValueGroupValues + valueGroup + postValueGroupValues;
		if (allValues.getCount() <= groupSize) {
			// Can perform as a single group
			CString	string =
							mInternals->mString.replacingSubStrings(sVariablePlaceholder,
									CString(
											TSArray<CString>(CString(OSSTR("?")),
													(UInt32) std::max<CArray::ItemCount>(allValues.getCount(), 1)),
											CString(OSSTR(","))));
			valueGroups += ValueGroup(string, allValues);
		} else {
			// Must perform in groups
			TArray<TArray<SSQLiteValue> >	valuesChunks = TNArray<SSQLiteValue>::asChunksFrom(valueGroup, groupSize);
			for (TArray<TArray<SSQLiteValue> >::Iterator iterator = valuesChunks.getIterator(); iterator; iterator++) {
				// Setup
				CString					string =
												mInternals->mString.replacingSubStrings(sVariablePlaceholder,
														CString(TSArray<CString>(CString(OSSTR("?")),
																iterator->getCount()),
														CString(OSSTR(","))));
				TNArray<SSQLiteValue>	values = preValueGroupValues + *iterator + postValueGroupValues;

				// Add ValueGroup
				valueGroups += ValueGroup(string, values);
			}
		}
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
			CString(OSSTR(" AND `")) + table.getName() + CString(OSSTR("`.`")) + tableColumn.getName() +
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
			CString(OSSTR(" AND `")) + tableColumn.getName() + CString(OSSTR("`")) + CString(OSSTR(" IN (")) +
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
