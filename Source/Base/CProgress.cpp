//----------------------------------------------------------------------------------------------------------------------
//	CProgress.h			©2021 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CProgress.h"

#include "CUUID.h"
#include "TLockingDictionary.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CProgress::Internals

class CProgress::Internals {
	public:
		Internals(const CProgress::UpdateInfo& updateInfo) : mUpdateInfo(updateInfo), mLastValueNotifyTime(0.0) {}

		CProgress::UpdateInfo	mUpdateInfo;

		CString					mMessage;
		OV<Float32>				mValue;
		UniversalTime			mLastValueNotifyTime;
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
}

//----------------------------------------------------------------------------------------------------------------------
CProgress::CProgress(const UpdateInfo& updateInfo, Float32 initialValue)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new Internals(updateInfo);

	// Set initial value
	mInternals->mValue.setValue(initialValue);
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
void CProgress::setValue(Float32 value)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if changed
	bool	hasValue = mInternals->mValue.hasValue();
	if (!hasValue || (value != *mInternals->mValue)) {
		// Store
		mInternals->mValue.setValue(value);

		// Check if need to notify
		if (!hasValue || ((SUniversalTime::getCurrent() - mInternals->mLastValueNotifyTime) > 1.0 / 60.0)) {
			// Update
			mInternals->mUpdateInfo.notify(*this);
			mInternals->mLastValueNotifyTime = SUniversalTime::getCurrent();
		}
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CProgress::removeValue()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if changed
	if (mInternals->mValue.hasValue()) {
		// Store
		mInternals->mValue.removeValue();

		// Always notify
		mInternals->mUpdateInfo.notify(*this);
		mInternals->mLastValueNotifyTime = SUniversalTime::getCurrent();
	}
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
						// Check if have value
						if (mTotalItemsCount.hasValue())
							// Set value
							mItemsProgress.setValue((Float32) mCompletedItemsCount / (Float32) *mTotalItemsCount);
						else
							// Remove value
							mItemsProgress.removeValue();
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
CItemsProgress::CItemsProgress(const UpdateInfo& updateInfo, UInt32 initialTotalItemsCount) : CProgress(updateInfo, 0.0)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new Internals(*this, OV<UInt32>(initialTotalItemsCount));
}

//----------------------------------------------------------------------------------------------------------------------
CItemsProgress::CItemsProgress(const UpdateInfo& updateInfo) : CProgress(updateInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new Internals(*this, OV<UInt32>());
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
		setValue((Float32) mInternals->mCompletedItemsCount / (Float32) *mInternals->mTotalItemsCount);
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

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CTimeIntervalProgress::Internals

class CTimeIntervalProgress::Internals {
	public:
		Internals(UniversalTimeInterval totalTimeInterval) :
			mCurrentTimeInterval(0.0), mTotalTimeInterval(totalTimeInterval)
			{}

		UniversalTimeInterval	mCurrentTimeInterval;
		UniversalTimeInterval	mTotalTimeInterval;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CTimeIntervalProgress

// MARK Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CTimeIntervalProgress::CTimeIntervalProgress(const UpdateInfo& updateInfo, UniversalTimeInterval totalTimeInterval) :
		CProgress(updateInfo, 0.0)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(totalTimeInterval);
}

//----------------------------------------------------------------------------------------------------------------------
CTimeIntervalProgress::~CTimeIntervalProgress()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK Instance methods

//----------------------------------------------------------------------------------------------------------------------
UniversalTimeInterval CTimeIntervalProgress::getTotalTimeInterval() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mTotalTimeInterval;
}

//----------------------------------------------------------------------------------------------------------------------
UniversalTimeInterval CTimeIntervalProgress::getTimeInterval() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mCurrentTimeInterval;
}

//----------------------------------------------------------------------------------------------------------------------
void CTimeIntervalProgress::setTimeInterval(UniversalTimeInterval timeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mCurrentTimeInterval = timeInterval;

	// Update value
	setValue((Float32) (timeInterval / mInternals->mTotalTimeInterval));
}

//----------------------------------------------------------------------------------------------------------------------
void CTimeIntervalProgress::complete()
//----------------------------------------------------------------------------------------------------------------------
{
	// Complete
	mInternals->mCurrentTimeInterval = mInternals->mTotalTimeInterval;

	// Update value
	setValue(1.0);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAggregateableTimeIntervalProgress

class CAggregateableTimeIntervalProgress : public CTimeIntervalProgress {
	public:
		CAggregateableTimeIntervalProgress(const UpdateInfo& updateInfo, UniversalTimeInterval totalTimeInterval) :
			CTimeIntervalProgress(updateInfo, totalTimeInterval), mID(CUUID().getBase64String())
			{}

		CString	mID;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAggregateTimeIntervalProgress::Internals

class CAggregateTimeIntervalProgress::Internals {
	public:
		struct Info {
			Info(const UpdateInfo& updateInfo, UniversalTimeInterval totalTimeInterval) :
				mProgress(updateInfo, totalTimeInterval), mPreviousTimeInterval(0.0)
				{}

			CAggregateableTimeIntervalProgress	mProgress;
			Float32								mPreviousTimeInterval;
		};

						Internals(CAggregateTimeIntervalProgress& progress) :
							mProgress(progress), mTotalTimeInterval(0.0)
							{}

		static	void	progressUpdated(const CAggregateableTimeIntervalProgress& progress, Internals* internals)
							{
								// Look up by id
								internals->mInfoByIDLock.lockForReading();
								Info&	info = **internals->mInfoByID[progress.mID];
								internals->mInfoByIDLock.unlockForReading();

								// Update value
								Float32	value =
												(internals->mProgress.getValue().hasValue() ?
																*internals->mProgress.getValue() : 0.0F) +
														((Float32) progress.getTimeInterval() -
																		info.mPreviousTimeInterval) /
																internals->mTotalTimeInterval;

								// Update info
								info.mPreviousTimeInterval = (Float32) progress.getTimeInterval();

								// Update value
								internals->mProgress.setValue(value);
							}

		CAggregateTimeIntervalProgress&	mProgress;
		TNDictionary<I<Info> >			mInfoByID;
		CReadPreferringLock				mInfoByIDLock;
		Float32							mTotalTimeInterval;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CAggregateTimeIntervalProgress

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CAggregateTimeIntervalProgress::CAggregateTimeIntervalProgress(const UpdateInfo& updateInfo) : CProgress(updateInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new Internals(*this);
}

//----------------------------------------------------------------------------------------------------------------------
CAggregateTimeIntervalProgress::~CAggregateTimeIntervalProgress()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CTimeIntervalProgress& CAggregateTimeIntervalProgress::addTimeIntervalProgress(UniversalTimeInterval totalTimeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	I<Internals::Info>	info(
								new Internals::Info(
										CProgress::UpdateInfo((CProgress::UpdateInfo::Proc) Internals::progressUpdated,
												mInternals),
										totalTimeInterval));

	// Update
	mInternals->mInfoByIDLock.lockForWriting();

	// Store
	mInternals->mInfoByID.set(info->mProgress.mID, info);

	// Recalculate value if needed
	if (getValue().hasValue() && (*getValue() > 0.0)) {
		// Recalculate metrics
		Float32	currentTimeInterval = 0.0;
		mInternals->mTotalTimeInterval = 0.0;

		// Iterate existing items
		for (TIteratorS<CDictionary::Item> iterator = mInternals->mInfoByID.getIterator(); iterator.hasValue();
				iterator.advance()) {
			// Setup
			const	Internals::Info&	_info = **((I<Internals::Info>*) iterator->mValue.getOpaque());

			// Update metrics
			currentTimeInterval += _info.mPreviousTimeInterval;
			mInternals->mTotalTimeInterval += (Float32) _info.mProgress.getTotalTimeInterval();
		}

		// Update value
		setValue(currentTimeInterval / mInternals->mTotalTimeInterval);
	} else
		// Update total time interval
		mInternals->mTotalTimeInterval += (Float32) totalTimeInterval;

	// Done
	mInternals->mInfoByIDLock.unlockForWriting();

	return info->mProgress;
}

//----------------------------------------------------------------------------------------------------------------------
void CAggregateTimeIntervalProgress::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove all
	mInternals->mInfoByID.removeAll();

	// Remove value
	removeValue();
}
