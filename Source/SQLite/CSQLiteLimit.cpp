//----------------------------------------------------------------------------------------------------------------------
//	CSQLiteLimit.cpp			©2023	Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CSQLiteLimit.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSQLiteLimit::Internals

class CSQLiteLimit::Internals {
	public:
		Internals(const CString& string) : mString(string) {}

	CString	mString;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSQLiteLimit

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CSQLiteLimit::CSQLiteLimit(UInt32 limit, const OV<UInt32>& offset)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals =
			new Internals(
					CString(OSSTR(" LIMIT ")) + CString(limit) +
							(offset.hasValue() ? CString(OSSTR(" OFFSET ")) + CString(*offset) : CString::mEmpty));
}

//----------------------------------------------------------------------------------------------------------------------
CSQLiteLimit::~CSQLiteLimit()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
const CString& CSQLiteLimit::getString() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mString;
}