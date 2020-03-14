//----------------------------------------------------------------------------------------------------------------------
//	CURL.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CURL.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CURLInternals

class CURLInternals : public TReferenceCountable<CURLInternals> {
	public:
		CURLInternals(const CString& string) : TReferenceCountable(), mString(string) {}

		CString	mString;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CURL

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CURL::CURL(const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CURLInternals(string);
}

//----------------------------------------------------------------------------------------------------------------------
CURL::CURL(const CURL& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CURL::~CURL()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CString CURL::getString() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mString;
}