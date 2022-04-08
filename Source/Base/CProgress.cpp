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
	// Prepare for write
	mInternals = mInternals->prepareForWrite();

	// Store
	mInternals->mMessage = message;

	// Update
	mInternals->mUpdateInfo.notify(*this);
}

//----------------------------------------------------------------------------------------------------------------------
const OV<Float32>& CProgress::getValue() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mValue;
}

//----------------------------------------------------------------------------------------------------------------------
void CProgress::setValue(OV<Float32> value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals = mInternals->prepareForWrite();

	// Store
	mInternals->mValue = value;

	// Update
	mInternals->mUpdateInfo.notify(*this);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CItemsProgressInternal

class CItemsProgressInternals : public TCopyOnWriteReferenceCountable<CItemsProgressInternals> {
	public:
		CItemsProgressInternals(const OV<UInt32>& initialTotalItemsCount) :
			mTotalItemsCount(initialTotalItemsCount), mCompletedItemsCount(0)
			{}

		OV<UInt32>	mTotalItemsCount;
		UInt32		mCompletedItemsCount;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CItemsProgress

// MARK Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CItemsProgress::CItemsProgress(const UpdateInfo& updateInfo, const OV<UInt32>& initialTotalItemsCount) :
		CProgress(updateInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CItemsProgressInternals(initialTotalItemsCount);
}

//----------------------------------------------------------------------------------------------------------------------
CItemsProgress::CItemsProgress(const CItemsProgress& other) : CProgress(other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CItemsProgress::~CItemsProgress()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CItemsProgress::addTotalItemsCount(UInt32 itemsCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals = mInternals->prepareForWrite();

	// Update
	if (mInternals->mTotalItemsCount.hasValue())
		// Already have total items
		mInternals->mTotalItemsCount.setValue(*mInternals->mTotalItemsCount + itemsCount);
	else
		// First total items
		mInternals->mTotalItemsCount = OV<UInt32>(itemsCount);

	// Update value
	setValue(OV<Float32>((Float32) mInternals->mCompletedItemsCount / (Float32) *mInternals->mTotalItemsCount));
}

//----------------------------------------------------------------------------------------------------------------------
void CItemsProgress::addCompletedItemsCount(UInt32 itemsCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals = mInternals->prepareForWrite();

	// Update
	mInternals->mCompletedItemsCount += itemsCount;

	// Check if have total items (no idea why not!)
	if (mInternals->mTotalItemsCount.hasValue())
		// Update value
		setValue(OV<Float32>((Float32) mInternals->mCompletedItemsCount / (Float32) *mInternals->mTotalItemsCount));
}
