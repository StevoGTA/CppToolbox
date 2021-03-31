//----------------------------------------------------------------------------------------------------------------------
//	CSQLiteTrigger.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CSQLiteTrigger.h"

#include "CSQLiteTable.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSQLiteTriggerInternals

class CSQLiteTriggerInternals {
	public:
		CSQLiteTriggerInternals(const CSQLiteTableColumn& updateTableColumn,
				const CSQLiteTableColumn& comparisonTableColumn) :
			mUpdateTableColumn(updateTableColumn), mComparisonTableColumn(comparisonTableColumn)
			{}

		const	CSQLiteTableColumn&	mUpdateTableColumn;
		const	CSQLiteTableColumn&	mComparisonTableColumn;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSQLiteTrigger

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CSQLiteTrigger::CSQLiteTrigger(const CSQLiteTableColumn& updateTableColumn,
		const CSQLiteTableColumn& comparisonTableColumn)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CSQLiteTriggerInternals(updateTableColumn, comparisonTableColumn);
}

//----------------------------------------------------------------------------------------------------------------------
CSQLiteTrigger::~CSQLiteTrigger()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CString CSQLiteTrigger::getString(const CSQLiteTable& table) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Return string
	return CString(OSSTR("CREATE TRIGGER `")) + table.getName() + CString(OSSTR("-")) +
					mInternals->mUpdateTableColumn.getName() + CString(OSSTR("Trigger`")) +
			CString(OSSTR(" AFTER UPDATE ON `")) + table.getName() + CString(OSSTR("`")) +
			CString(OSSTR(" FOR EACH ROW")) +
			CString(OSSTR(" BEGIN UPDATE `")) + table.getName() + CString(OSSTR("`")) +
			CString(OSSTR(" SET `")) + mInternals->mUpdateTableColumn.getName() + CString(OSSTR("`=")) +
					mInternals->mUpdateTableColumn.getDefaultValue()->getString() +
			CString(OSSTR(" WHERE `")) + mInternals->mComparisonTableColumn.getName() + CString(OSSTR("`=NEW.`")) +
					mInternals->mComparisonTableColumn.getName() + CString(OSSTR("`;")) +
			CString(OSSTR(" END"));
}
