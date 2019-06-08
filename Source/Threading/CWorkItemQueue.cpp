//----------------------------------------------------------------------------------------------------------------------
//	CWorkItemQueue.cpp			©2012 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CWorkItemQueue.h"

#include "CArray.h"
#include "CCoreServices.h"
#include "CLock.h"
#include "CThread.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SWorkItemInfo

struct SWorkItemInfo {
	// Lifecycle methods
	SWorkItemInfo(CWorkItemQueueInternals& owningWorkItemQueueInternals, CWorkItem& workItem,
			EWorkItemPriority priority) :
		mOwningWorkItemQueueInternals(owningWorkItemQueueInternals), mWorkItem(workItem), mPriority(priority),
				mIsPaused(false)
		{}

	// Properties
	CWorkItemQueueInternals&	mOwningWorkItemQueueInternals;
	CWorkItem&					mWorkItem;
	EWorkItemPriority			mPriority;
	bool						mIsPaused;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - SWorkItemThreadInfo

struct SWorkItemThreadInfo {
	// Lifecycle methods
	SWorkItemThreadInfo(SWorkItemInfo* initialWorkItemInfo, CThreadProc threadProc, const CString& name) :
		mWorkItemInfo(initialWorkItemInfo), mThread(threadProc, this, name)
		{}

	// Properties
	CSemaphore		mSemaphore;
	CThread			mThread;
	SWorkItemInfo*	mWorkItemInfo;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CWorkItemInternals

class CWorkItemInternals {
	public:
		CWorkItemInternals(bool disposeOnCompletion) :
			mDisposeOnCompletion(disposeOnCompletion), mIsActive(false), mIsCancelled(false)
			{}
		~CWorkItemInternals() {}

		bool	mDisposeOnCompletion;
		bool	mIsActive;
		bool	mIsCancelled;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CWorkItemQueueInternals

class CWorkItemQueueInternals {
	public:
		CWorkItemQueueInternals() {}
		~CWorkItemQueueInternals() {}

		TPtrArray<SWorkItemInfo*>	mWorkItemInfos;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CProcWorkItem

class CProcWorkItem : public CWorkItem {
	public:
				CProcWorkItem(CWorkItemProc proc, void* userData) : CWorkItem(true), mProc(proc), mUserData(userData) {}
				~CProcWorkItem() {}

		void	perform()
					{ mProc(mUserData, *this); }

		CWorkItemProc	mProc;
		void*			mUserData;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local data

static	TPtrArray<SWorkItemInfo*>	sWorkItemInfos;
static	CLock						sWorkItemInfosLock;

static	TPtrArray<SWorkItemThreadInfo*>	sActiveWorkitemThreadInfos;
static	TPtrArray<SWorkItemThreadInfo*>	sIdleWorkitemThreadInfos;
static	CLock							sWorkItemThreadInfosLock;
static	CArrayItemCount					sMaxWorkItemThreads = CCoreServices::getTotalProcessorCoresCount() - 1;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc declarations

static	ECompareResult	sWorkItemInfoCompareProc(SWorkItemInfo* const workItemInfo1, SWorkItemInfo* const workItemInfo2,
								void* userData);
static	void			sProcessWorkItems();
static	void			sThreadProc(const CThread& thread, void* userData);
static	void			sWorkItemInfoCleanup(SWorkItemInfo* workItemInfo);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CWorkItem

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CWorkItem::CWorkItem(bool disposeOnCompletion)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CWorkItemInternals(disposeOnCompletion);
}

//----------------------------------------------------------------------------------------------------------------------
CWorkItem::~CWorkItem()
//----------------------------------------------------------------------------------------------------------------------
{
	DisposeOf(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CWorkItem::cancel()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mIsCancelled = true;
}

//----------------------------------------------------------------------------------------------------------------------
bool CWorkItem::isCancelled() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mIsCancelled;
}

//----------------------------------------------------------------------------------------------------------------------
void CWorkItem::setActive(bool isActive)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mIsActive = isActive;
}

//----------------------------------------------------------------------------------------------------------------------
bool CWorkItem::isActive() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mIsActive;
}

//----------------------------------------------------------------------------------------------------------------------
void CWorkItem::finished() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if need dispose
	if (mInternals->mDisposeOnCompletion) {
		// Dispose
		CWorkItem*	THIS = (CWorkItem*) this;
		DisposeOf(THIS);
	}
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CWorkItemQueue

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CWorkItemQueue::CWorkItemQueue()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CWorkItemQueueInternals();
}

//----------------------------------------------------------------------------------------------------------------------
CWorkItemQueue::~CWorkItemQueue()
//----------------------------------------------------------------------------------------------------------------------
{
	DisposeOf(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CWorkItemQueue::add(CWorkItem& workItem, EWorkItemPriority priority)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SWorkItemInfo*	workItemInfo = new SWorkItemInfo(*mInternals, workItem, priority);

	// Add
	sWorkItemInfosLock.lock();
	mInternals->mWorkItemInfos += workItemInfo;
	sWorkItemInfos += workItemInfo;
	sWorkItemInfosLock.unlock();

	// Process work items
	sProcessWorkItems();
}

//----------------------------------------------------------------------------------------------------------------------
CWorkItem& CWorkItemQueue::add(CWorkItemProc proc, void* userData, EWorkItemPriority priority)
//----------------------------------------------------------------------------------------------------------------------
{
	// Create workItem
	CProcWorkItem*	procWorkItem = new CProcWorkItem(proc, userData);

	// Add
	add(*procWorkItem, priority);

	return *procWorkItem;
}

//----------------------------------------------------------------------------------------------------------------------
void CWorkItemQueue::pause()
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all workItem infos
	sWorkItemInfosLock.lock();
	for (CArrayItemIndex i = 0; i < mInternals->mWorkItemInfos.getCount(); i++)
		// Mark as paused
		mInternals->mWorkItemInfos[i]->mIsPaused = true;
	sWorkItemInfosLock.unlock();
}

//----------------------------------------------------------------------------------------------------------------------
void CWorkItemQueue::resume()
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all workItem infos
	sWorkItemInfosLock.lock();
	for (CArrayItemIndex i = 0; i < mInternals->mWorkItemInfos.getCount(); i++)
		// Mark as not paused
		mInternals->mWorkItemInfos[i]->mIsPaused = false;
	sWorkItemInfosLock.unlock();

	// Process work items
	sProcessWorkItems();
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: Local proc definitions

//----------------------------------------------------------------------------------------------------------------------
ECompareResult sWorkItemInfoCompareProc(SWorkItemInfo* const workItemInfo1, SWorkItemInfo* const workItemInfo2,
		void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Sort in this order:
	//	Active
	//	Cancelled
	//	Priority
	if (workItemInfo1->mWorkItem.isActive())
		// Work item 1 is acive
		return kCompareResultBefore;
	else if (workItemInfo2->mWorkItem.isActive())
		// Work item 2 is active
		return kCompareResultAfter;
	else if (workItemInfo1->mWorkItem.isCancelled())
		// Work item 1 is cancelled
		return kCompareResultBefore;
	else if (workItemInfo2->mWorkItem.isCancelled())
		// Work item 2 is cancelled
		return kCompareResultAfter;
	else if (workItemInfo1->mPriority < workItemInfo2->mPriority)
		// Work item 1 has a higher priority
		return kCompareResultBefore;
	else if (workItemInfo2->mPriority < workItemInfo1->mPriority)
		// Work item 2 has a higher priority
		return kCompareResultAfter;
	else
		// They be equivalent
		return kCompareResultEquivalent;
}

//----------------------------------------------------------------------------------------------------------------------
void sProcessWorkItems()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if can do another workItem
	sWorkItemThreadInfosLock.lock();
	if (sActiveWorkitemThreadInfos.getCount() < sMaxWorkItemThreads) {
		// Lock
		sWorkItemInfosLock.lock();

		// Sort
		sWorkItemInfos.sort(sWorkItemInfoCompareProc);

		// Iterate until we find an workItem to perform
		for (CArrayItemIndex i = 0; i < sWorkItemInfos.getCount();) {
			// Check next workItem
			SWorkItemInfo*	workItemInfo = sWorkItemInfos[i];

			// Check if active
			if (workItemInfo->mWorkItem.isActive()) {
				// Skip
				i++;
				continue;
			}

			if (workItemInfo->mWorkItem.isCancelled())
				// Cancelled
				sWorkItemInfoCleanup(workItemInfo);
			else if (workItemInfo->mIsPaused)
				// Skip
				i++;
			else {
				// Perform this workItem
				workItemInfo->mWorkItem.setActive(true);
				if (sIdleWorkitemThreadInfos.getCount() > 0) {
					// Resume an idle thread
					SWorkItemThreadInfo*	workItemThreadInfo = sIdleWorkitemThreadInfos[0];
					sIdleWorkitemThreadInfos.removeAtIndex(0);

					workItemThreadInfo->mWorkItemInfo = workItemInfo;
					workItemThreadInfo->mSemaphore.signal();

					sActiveWorkitemThreadInfos += workItemThreadInfo;
				} else
					// Create new active thread
					sActiveWorkitemThreadInfos +=
							new SWorkItemThreadInfo(workItemInfo, sThreadProc,
									CString("CWorkItemQueue Thread #") +
											CString(sActiveWorkitemThreadInfos.getCount() + 1));

				break;
			}
		}

		// Unlock
		sWorkItemInfosLock.unlock();
	}
	sWorkItemThreadInfosLock.unlock();
}

//----------------------------------------------------------------------------------------------------------------------
void sThreadProc(const CThread& thread, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SWorkItemThreadInfo*	workItemThreadInfo = (SWorkItemThreadInfo*) userData;

	// Run forever
	while (true) {
		// Run workItem
		SWorkItemInfo*	workItemInfo = workItemThreadInfo->mWorkItemInfo;
		workItemInfo->mWorkItem.perform();
		workItemInfo->mWorkItem.setActive(false);
		workItemInfo->mWorkItem.finished();

		// Update
		sWorkItemInfoCleanup(workItemInfo);
		workItemThreadInfo->mWorkItemInfo = nil;

		sWorkItemThreadInfosLock.lock();
		sActiveWorkitemThreadInfos -= workItemThreadInfo;
		sIdleWorkitemThreadInfos += workItemThreadInfo;
		sWorkItemThreadInfosLock.unlock();

		// Next
		sProcessWorkItems();

		// Check if waiting on workItem info
		if (workItemThreadInfo->mWorkItemInfo == nil)
			// Wait
			workItemThreadInfo->mSemaphore.waitFor();
	}
}

//----------------------------------------------------------------------------------------------------------------------
void sWorkItemInfoCleanup(SWorkItemInfo* workItemInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove from arrays
	sWorkItemInfosLock.lock();
	sWorkItemInfos -= workItemInfo;
	workItemInfo->mOwningWorkItemQueueInternals.mWorkItemInfos -= workItemInfo;
	sWorkItemInfosLock.unlock();
}
