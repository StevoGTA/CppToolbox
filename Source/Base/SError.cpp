//----------------------------------------------------------------------------------------------------------------------
//	SError.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "SError.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SError

// MARK: Properties

const	SError	SError::mCancelled(CString(OSSTR("SError")), 1, CString(OSSTR("Cancelled")));
const	SError	SError::mEndOfData(CString(OSSTR("SError")), 2, CString(OSSTR("End of Data")));
const	SError	SError::mUnimplemented(CString(OSSTR("SError")), 3, CString(OSSTR("Unimplemented")));

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
SError::SError(const CDictionary& storageInfo) :
		mDomain(storageInfo.getString(CString(OSSTR("domain")))), mCode(storageInfo.getSInt32(CString(OSSTR("code")))),
		mLocalizationInfo(storageInfo.getDictionary(CString(OSSTR("localizationInfo")))),
		mInternalDescription(storageInfo.getOVString(CString(OSSTR("internalDescription"))))
//----------------------------------------------------------------------------------------------------------------------
{
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CDictionary SError::getStorageInfo() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Compose storage info
	CDictionary	storageInfo;
	storageInfo.set(CString(OSSTR("domain")), mDomain);
	storageInfo.set(CString(OSSTR("code")), mCode);
	storageInfo.set(CString(OSSTR("localizationInfo")), mLocalizationInfo);
	storageInfo.set(CString(OSSTR("internalDescription")), mInternalDescription);

	return storageInfo;
}
