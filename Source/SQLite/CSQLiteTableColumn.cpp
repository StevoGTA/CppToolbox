//----------------------------------------------------------------------------------------------------------------------
//	CSQLiteTableColumn.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CSQLiteTableColumn.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSQLiteTableColumnInternals

class CSQLiteTableColumnInternals : public TReferenceCountable<CSQLiteTableColumnInternals> {
	public:
		CSQLiteTableColumnInternals(const CString& name, CSQLiteTableColumn::Kind kind,
				CSQLiteTableColumn::Options options, OI<SSQLiteValue> defaultValue = nil) :
			mName(name), mKind(kind), mOptions(options), mDefaultValue(defaultValue)
			{}

		CString						mName;
		CSQLiteTableColumn::Kind	mKind;
		CSQLiteTableColumn::Options	mOptions;
		OI<SSQLiteValue>			mDefaultValue;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSQLiteTableColumn

// MARK: Properties

const	CSQLiteTableColumn	CSQLiteTableColumn::mRowID(CString(OSSTR("rowid")), kInteger);
const	CSQLiteTableColumn	CSQLiteTableColumn::mAll(CString(OSSTR("*")), kInteger);

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CSQLiteTableColumn::CSQLiteTableColumn(const CString& name, Kind kind, Options options, OI<SSQLiteValue> defaultValue)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CSQLiteTableColumnInternals(name, kind, options, defaultValue);
}

//----------------------------------------------------------------------------------------------------------------------
CSQLiteTableColumn::CSQLiteTableColumn(const CSQLiteTableColumn& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CSQLiteTableColumn::~CSQLiteTableColumn()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
const CString& CSQLiteTableColumn::getName() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mName;
}

//----------------------------------------------------------------------------------------------------------------------
CSQLiteTableColumn::Kind CSQLiteTableColumn::getKind() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mKind;
}

//----------------------------------------------------------------------------------------------------------------------
CSQLiteTableColumn::Options CSQLiteTableColumn::getOptions() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mOptions;
}

//----------------------------------------------------------------------------------------------------------------------
OI<SSQLiteValue> CSQLiteTableColumn::getDefaultValue() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mDefaultValue;
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CSQLiteTableColumn CSQLiteTableColumn::dateISO8601FractionalSecondsAutoSet(const CString& name)
//----------------------------------------------------------------------------------------------------------------------
{
	return CSQLiteTableColumn(name, kDateISO8601FractionalSecondsAutoSet, kNotNull,
			OI<SSQLiteValue>(CString(OSSTR("strftime('%Y-%m-%dT%H:%M:%f', 'now', 'localtime')"))));
}

//----------------------------------------------------------------------------------------------------------------------
CSQLiteTableColumn CSQLiteTableColumn::dateISO8601FractionalSecondsAutoUpdate(const CString& name)
//----------------------------------------------------------------------------------------------------------------------
{
	return CSQLiteTableColumn(name, kDateISO8601FractionalSecondsAutoUpdate, kNotNull,
			OI<SSQLiteValue>(CString(OSSTR("strftime('%Y-%m-%dT%H:%M:%f', 'now', 'localtime')"))));
}
