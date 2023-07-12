//----------------------------------------------------------------------------------------------------------------------
//	CSQLiteTableColumn.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CSQLiteTableColumn.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSQLiteTableColumn::Internals

class CSQLiteTableColumn::Internals : public TReferenceCountable<Internals> {
	public:
		Internals(const CString& name, CSQLiteTableColumn::Kind kind, CSQLiteTableColumn::Options options,
				OV<SSQLiteValue> defaultValue = OV<SSQLiteValue>()) :
			mName(name), mKind(kind), mOptions(options), mDefaultValue(defaultValue)
			{}

		CString						mName;
		CSQLiteTableColumn::Kind	mKind;
		CSQLiteTableColumn::Options	mOptions;
		OV<SSQLiteValue>			mDefaultValue;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSQLiteTableColumn

// MARK: Properties

const	CSQLiteTableColumn	CSQLiteTableColumn::mRowID(CString(OSSTR("rowid")), kKindInteger);
const	CSQLiteTableColumn	CSQLiteTableColumn::mAll(CString(OSSTR("*")), kKindInteger);

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CSQLiteTableColumn::CSQLiteTableColumn(const CString& name, Kind kind, Options options, OV<SSQLiteValue> defaultValue)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(name, kind, options, defaultValue);
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
OV<SSQLiteValue> CSQLiteTableColumn::getDefaultValue() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mDefaultValue;
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CSQLiteTableColumn CSQLiteTableColumn::dateISO8601FractionalSecondsAutoSet(const CString& name)
//----------------------------------------------------------------------------------------------------------------------
{
	return CSQLiteTableColumn(name, kKindDateISO8601FractionalSecondsAutoSet, kNotNull,
			OV<SSQLiteValue>(CString(OSSTR("strftime('%Y-%m-%dT%H:%M:%f', 'now', 'localtime')"))));
}

//----------------------------------------------------------------------------------------------------------------------
CSQLiteTableColumn CSQLiteTableColumn::dateISO8601FractionalSecondsAutoUpdate(const CString& name)
//----------------------------------------------------------------------------------------------------------------------
{
	return CSQLiteTableColumn(name, kKindDateISO8601FractionalSecondsAutoUpdate, kNotNull,
			OV<SSQLiteValue>(CString(OSSTR("strftime('%Y-%m-%dT%H:%M:%f', 'now', 'localtime')"))));
}
