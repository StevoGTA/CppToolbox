//----------------------------------------------------------------------------------------------------------------------
//	CProgress.h			©2021 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CProgress.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CProgressInternals

class CProgressInternals : public TCopyOnWriteReferenceCountable<CProgressInternals> {
	public:
		CProgressInternals(const CProgress::UpdateInfo& updateInfo) : mUpdateInfo(updateInfo) {}

		CProgress::UpdateInfo	mUpdateInfo;

		CString					mMessage;
		OV<Float32>				mValue;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CProgress

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CProgress::CProgress(const UpdateInfo& updateInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new CProgressInternals(updateInfo);

	// Update
	mInternals->mUpdateInfo.notify(*this);
}

//----------------------------------------------------------------------------------------------------------------------
CProgress::CProgress(const CProgress& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CProgress::~CProgress()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
const CString& CProgress::getMessage() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mMessage;
}

//----------------------------------------------------------------------------------------------------------------------
void CProgress::setMessage(const CString& message)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mMessage = message;

	// Update
	mInternals->mUpdateInfo.notify(*this);
}

//----------------------------------------------------------------------------------------------------------------------
OV<Float32> CProgress::getValue() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mValue;
}

//----------------------------------------------------------------------------------------------------------------------
void CProgress::setValue(OV<Float32> value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mValue = value;

	// Update
	mInternals->mUpdateInfo.notify(*this);
}
