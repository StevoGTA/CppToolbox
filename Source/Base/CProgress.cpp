//----------------------------------------------------------------------------------------------------------------------
//	CProgress.h			©2021 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CProgress.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CProgress::Internals

class CProgress::Internals {
	public:
		Internals(const CProgress::UpdateInfo& updateInfo) : mUpdateInfo(updateInfo) {}

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
	mInternals = new Internals(updateInfo);

	// Update
	mInternals->mUpdateInfo.notify(*this);
}

//----------------------------------------------------------------------------------------------------------------------
CProgress::~CProgress()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
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
const OV<Float32>& CProgress::getValue() const
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

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CItemsProgress::Internals

class CItemsProgress::Internals {
	public:
				Internals(CItemsProgress& itemsProgress, const OV<UInt32>& initialTotalItemsCount) :
					mItemsProgress(itemsProgress),
							mTotalItemsCount(initialTotalItemsCount), mCompletedItemsCount(0)
					{
						// Set initial value
						updateValue();
					}

		void	updateValue()
					{
						mItemsProgress.setValue(
								mTotalItemsCount.hasValue() ?
										OV<Float32>((Float32) mCompletedItemsCount / (Float32) *mTotalItemsCount) :
										OV<Float32>());
					}

		CItemsProgress&	mItemsProgress;

		OV<UInt32>		mTotalItemsCount;
		UInt32			mCompletedItemsCount;
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
	mInternals = new Internals(*this, initialTotalItemsCount);
}

//----------------------------------------------------------------------------------------------------------------------
CItemsProgress::~CItemsProgress()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CItemsProgress::addTotalItemsCount(UInt32 itemsCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Update
	if (mInternals->mTotalItemsCount.hasValue())
		// Already have total items
		mInternals->mTotalItemsCount.setValue(*mInternals->mTotalItemsCount + itemsCount);
	else
		// First total items
		mInternals->mTotalItemsCount = OV<UInt32>(itemsCount);

	// Update value
	mInternals->updateValue();
}

//----------------------------------------------------------------------------------------------------------------------
OV<UInt32> CItemsProgress::getTotalItemsCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mTotalItemsCount;
}

//----------------------------------------------------------------------------------------------------------------------
void CItemsProgress::addCompletedItemsCount(UInt32 itemsCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Update
	mInternals->mCompletedItemsCount += itemsCount;

	// Check if have total items
	if (mInternals->mTotalItemsCount.hasValue())
		// Update value
		setValue(OV<Float32>((Float32) mInternals->mCompletedItemsCount / (Float32) *mInternals->mTotalItemsCount));
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 CItemsProgress::getCompletedItemsCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mCompletedItemsCount;
}

//----------------------------------------------------------------------------------------------------------------------
void CItemsProgress::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Reset
	mInternals->mTotalItemsCount = OV<UInt32>();
	mInternals->mCompletedItemsCount = 0;

	// Update value
	mInternals->updateValue();
}
