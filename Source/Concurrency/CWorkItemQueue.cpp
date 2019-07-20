//----------------------------------------------------------------------------------------------------------------------
//	CWorkItemQueue.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CWorkItemQueue.h"

#include "CArray.h"
#include "CCoreServices.h"
#include "CLock.h"
#include "CThread.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CWorkItemInternals

class CWorkItemInternals {
	public:
		CWorkItemInternals(bool disposeWhenFinishedOrCancelled) :
			mDisposeWhenFinishedOrCancelled(disposeWhenFinishedOrCancelled), mState(kWorkItemStateWaiting)
			{}
		~CWorkItemInternals() {}

		bool			mDisposeWhenFinishedOrCancelled;
		EWorkItemState	mState;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CWorkItem

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CWorkItem::CWorkItem(bool disposeWhenFinishedOrCancelled)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CWorkItemInternals(disposeWhenFinishedOrCancelled);
}

//----------------------------------------------------------------------------------------------------------------------
CWorkItem::~CWorkItem()
//----------------------------------------------------------------------------------------------------------------------
{
	DisposeOf(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
EWorkItemState CWorkItem::getState() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mState;
}

//----------------------------------------------------------------------------------------------------------------------
void CWorkItem::cancel()
//----------------------------------------------------------------------------------------------------------------------
{
	// Can only cancel if waiting
	if (mInternals->mState == kWorkItemStateWaiting)
		// Transition to cancelled
		mInternals->mState = kWorkItemStateCancelled;
}

//----------------------------------------------------------------------------------------------------------------------
void CWorkItem::finished() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if need dispose
	if (mInternals->mDisposeWhenFinishedOrCancelled) {
		// Dispose
		CWorkItem*	THIS = (CWorkItem*) this;
		DisposeOf(THIS);
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CWorkItem::cancelled() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if need dispose
	if (mInternals->mDisposeWhenFinishedOrCancelled) {
		// Dispose
		CWorkItem*	THIS = (CWorkItem*) this;
		DisposeOf(THIS);
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CWorkItem::transitionToActive()
//----------------------------------------------------------------------------------------------------------------------
{
	// Update state
	mInternals->mState = kWorkItemStateActive;
}

//----------------------------------------------------------------------------------------------------------------------
void CWorkItem::transitionToFinished()
//----------------------------------------------------------------------------------------------------------------------
{
	// Update state
	mInternals->mState = kWorkItemStateFinished;

	// Call method
	finished();
}

//----------------------------------------------------------------------------------------------------------------------
void CWorkItem::transitionToCancelled()
//----------------------------------------------------------------------------------------------------------------------
{
	// Update state
	mInternals->mState = kWorkItemStateCancelled;

	// Call method
	cancelled();
}

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
// MARK: - SWorkItemInfo

struct SWorkItemInfo {
	// Lifecycle methods
	SWorkItemInfo(CWorkItemQueueInternals& owningWorkItemQueueInternals, CWorkItem& workItem,
			EWorkItemPriority priority) :
		mOwningWorkItemQueueInternals(owningWorkItemQueueInternals), mWorkItem(workItem), mPriority(priority),
				mIsPaused(false), mIndex(SWorkItemInfo::mNextIndex++)
		{}

	// Properties
			CWorkItemQueueInternals&	mOwningWorkItemQueueInternals;
			CWorkItem&					mWorkItem;
			EWorkItemPriority			mPriority;
			bool						mIsPaused;
			UInt32						mIndex;

	static	UInt32						mNextIndex;
};

UInt32	SWorkItemInfo::mNextIndex = 0;

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
// MARK: - CWorkItemQueueInternals

class CWorkItemQueueInternals {
	public:
								CWorkItemQueueInternals(CWorkItemQueueInternals* targetWorkItemQueueInternals,
										UInt32 maximumConcurrentWorkItems) :
									mTargetWorkItemQueueInternals(targetWorkItemQueueInternals),
											mMaximumConcurrentWorkItems(maximumConcurrentWorkItems)
									{
										// Check if have target
										if (mTargetWorkItemQueueInternals != nil)
											// Add as child
											mTargetWorkItemQueueInternals->addChildWorkItemQueueInternals(*this);
									}
								~CWorkItemQueueInternals()
									{
										// Check if have target
										if (mTargetWorkItemQueueInternals != nil)
											// Remove as child
											mTargetWorkItemQueueInternals->removeChildWorkItemQueueInternals(*this);
									}

				void			add(CWorkItem& workItem, EWorkItemPriority priority)
									{
										// Setup
										SWorkItemInfo*	workItemInfo = new SWorkItemInfo(*this, workItem, priority);

										// Add
										mWorkItemInfosLock.lock();
										mIdleWorkItemInfos += workItemInfo;
										mWorkItemInfosLock.unlock();
									}

				void			pause()
									{
										// Iterate all workItem infos
										mWorkItemInfosLock.lock();
										for (CArrayItemIndex i = 0; i < mIdleWorkItemInfos.getCount(); i++)
											// Mark as paused
											mIdleWorkItemInfos[i]->mIsPaused = true;
										mWorkItemInfosLock.unlock();
									}
				void			resume()
									{
										// Iterate all workItem infos
										mWorkItemInfosLock.lock();
										for (CArrayItemIndex i = 0; i < mIdleWorkItemInfos.getCount(); i++)
											// Mark as not paused
											mIdleWorkItemInfos[i]->mIsPaused = false;
										mWorkItemInfosLock.unlock();
									}

		static	void			processWorkItems()
									{
										// Check if can do another workItem
										mWorkItemThreadInfosLock.lock();
										if (mActiveWorkItemThreadInfos.getCount() <
												mMainWorkItemQueueInternals->mMaximumConcurrentWorkItems) {
											// Get next work item info
											SWorkItemInfo*	workItemInfo =
																	mMainWorkItemQueueInternals->getNextWorkItemInfo();

											// Did we get a work item info
											if (workItemInfo != nil) {
												// Perform this workItem
												workItemInfo->mWorkItem.transitionToActive();
												workItemInfo->mOwningWorkItemQueueInternals
														.noteTransitioningToActive(workItemInfo);

												// Find thread
												if (mIdleWorkItemThreadInfos.getCount() > 0) {
													// Resume an idle thread
													SWorkItemThreadInfo*	workItemThreadInfo =
																					mIdleWorkItemThreadInfos[0];
													mIdleWorkItemThreadInfos.removeAtIndex(0);

													workItemThreadInfo->mWorkItemInfo = workItemInfo;
													workItemThreadInfo->mSemaphore.signal();

													mActiveWorkItemThreadInfos += workItemThreadInfo;
												} else
													// Create new active thread
													mActiveWorkItemThreadInfos +=
															new SWorkItemThreadInfo(workItemInfo, threadProc,
																	CString("CWorkItemQueue Thread #") +
																			CString(mActiveWorkItemThreadInfos
																					.getCount() + 1));
											}
										}
										mWorkItemThreadInfosLock.unlock();
									}
	private:
				void			addChildWorkItemQueueInternals(CWorkItemQueueInternals& workItemQueueInternals)
									{
										// Add
										mChildWorkItemQueueInternalsLock.lock();
										mChildWorkItemQueueInternals += &workItemQueueInternals;
										mChildWorkItemQueueInternalsLock.unlock();
									}
				void			removeChildWorkItemQueueInternals(CWorkItemQueueInternals& workItemQueueInternals)
									{
										// Remove
										mChildWorkItemQueueInternalsLock.lock();
										mChildWorkItemQueueInternals -= &workItemQueueInternals;
										mChildWorkItemQueueInternalsLock.unlock();
									}

				UInt32			getActiveWorkItemInfosCountDeep()
									{
										// Setup
										UInt32	count = 0;

										// Add our active work items
										mWorkItemInfosLock.lock();
										count += mActiveWorkItemInfos.getCount();
										mWorkItemInfosLock.unlock();

										// Add active work items from all child work item queues
										mChildWorkItemQueueInternalsLock.lock();
										for (CArrayItemIndex i = 0; i < mChildWorkItemQueueInternals.getCount(); i++)
											// Add to count
											count += mChildWorkItemQueueInternals[i]->getActiveWorkItemInfosCountDeep();
										mChildWorkItemQueueInternalsLock.unlock();

										return count;
									}
				SWorkItemInfo*	getNextWorkItemInfo()
									{
										// Check if have headroom
										if (getActiveWorkItemInfosCountDeep() >= mMaximumConcurrentWorkItems)
											// No more headroom
											return nil;

										// Setup
										SWorkItemInfo*	workItemInfo = nil;

										// Get our next work item info
										mWorkItemInfosLock.lock();

										// Sort
										mIdleWorkItemInfos.sort(workItemInfoCompareProc);

										// Keep going until we find a work item that is ready
										while ((workItemInfo == nil) && (mIdleWorkItemInfos.getCount() > 0)) {
											// Get first work item
											SWorkItemInfo*	testWorkItemInfo = mIdleWorkItemInfos.getFirst();
											if (testWorkItemInfo->mWorkItem.isCancelled()) {
												// Cancelled
												testWorkItemInfo->mWorkItem.transitionToCancelled();

												mIdleWorkItemThreadInfos.removeAtIndex(0);
												DisposeOf(testWorkItemInfo);
											} else if (!testWorkItemInfo->mIsPaused)
												// Start with this work item info
												workItemInfo = testWorkItemInfo;
										}
										mWorkItemInfosLock.unlock();

										// Check any child work item queue
										mChildWorkItemQueueInternalsLock.lock();
										for (CArrayItemIndex i = 0; i < mChildWorkItemQueueInternals.getCount(); i++) {
											// Get next work item info for this work item queue
											SWorkItemInfo*	childWorkItemInfo =
																	mChildWorkItemQueueInternals[i]->
																			getNextWorkItemInfo();
											if ((workItemInfo == nil) && (childWorkItemInfo != nil))
												// Start with this child work item info
												workItemInfo = childWorkItemInfo;
											else if ((workItemInfo != nil) && (childWorkItemInfo != nil) &&
													(childWorkItemInfo->mIndex < workItemInfo->mIndex))
												// Prefer this child work item info
												workItemInfo = childWorkItemInfo;
										}
										mChildWorkItemQueueInternalsLock.unlock();

										return workItemInfo;
									}
				void			noteTransitioningToActive(SWorkItemInfo* workItemInfo)
									{
										// Remove from active
										mWorkItemInfosLock.lock();
										mIdleWorkItemInfos -= workItemInfo;
										mActiveWorkItemInfos += workItemInfo;
										mWorkItemInfosLock.unlock();
									}
				void			noteFinished(SWorkItemInfo* workItemInfo)
									{
										// Remove from active
										mWorkItemInfosLock.lock();
										mActiveWorkItemInfos -= workItemInfo;
										mWorkItemInfosLock.unlock();

										// Cleanup
										DisposeOf(workItemInfo);
									}

		static	ECompareResult	workItemInfoCompareProc(SWorkItemInfo* const workItemInfo1,
										SWorkItemInfo* const workItemInfo2, void* userData)
									{
										// Sort in this order:
										//	Active
										//	Cancelled
										//  Not Paused
										//	Priority
										//	Index
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
										else if (!workItemInfo1->mIsPaused)
											// Work item 1 is not paused
											return kCompareResultBefore;
										else if (!workItemInfo2->mIsPaused)
											// Wori item 2 is not paused
											return kCompareResultAfter;
										else if (workItemInfo1->mPriority < workItemInfo2->mPriority)
											// Work item 1 has a higher priority
											return kCompareResultBefore;
										else if (workItemInfo2->mPriority < workItemInfo1->mPriority)
											// Work item 2 has a higher priority
											return kCompareResultAfter;
										else if (workItemInfo1->mIndex < workItemInfo2->mIndex)
											// Work item 1 has an earlier index
											return kCompareResultBefore;
										else
											// Work item 1 has a later index
											return kCompareResultAfter;
									}
		static	void			threadProc(const CThread& thread, void* userData)
									{
										// Setup
										SWorkItemThreadInfo*	workItemThreadInfo = (SWorkItemThreadInfo*) userData;

										// Run forever
										while (true) {
											// Run workItem
											SWorkItemInfo*	workItemInfo = workItemThreadInfo->mWorkItemInfo;
											workItemInfo->mWorkItem.perform();

											// Note finished
											workItemInfo->mWorkItem.transitionToFinished();

											workItemThreadInfo->mWorkItemInfo = nil;
											workItemInfo->mOwningWorkItemQueueInternals.noteFinished(workItemInfo);

											// Move to idle
											mWorkItemThreadInfosLock.lock();
											mActiveWorkItemThreadInfos -= workItemThreadInfo;
											mIdleWorkItemThreadInfos += workItemThreadInfo;
											mWorkItemThreadInfosLock.unlock();

											// Process work items
											processWorkItems();

											// Check if waiting on workItem info
											if (workItemThreadInfo->mWorkItemInfo == nil)
												// Wait
												workItemThreadInfo->mSemaphore.waitFor();
										}
									}

	public:
		static	CWorkItemQueueInternals*			mMainWorkItemQueueInternals;

	private:
				CWorkItemQueueInternals*			mTargetWorkItemQueueInternals;
				UInt32								mMaximumConcurrentWorkItems;

				TPtrArray<CWorkItemQueueInternals*>	mChildWorkItemQueueInternals;
				CLock								mChildWorkItemQueueInternalsLock;

				TPtrArray<SWorkItemInfo*>			mActiveWorkItemInfos;
				TPtrArray<SWorkItemInfo*>			mIdleWorkItemInfos;
				CLock								mWorkItemInfosLock;

		static	TPtrArray<SWorkItemThreadInfo*>		mActiveWorkItemThreadInfos;
		static	TPtrArray<SWorkItemThreadInfo*>		mIdleWorkItemThreadInfos;
		static	CLock								mWorkItemThreadInfosLock;
};

CWorkItemQueueInternals*			CWorkItemQueueInternals::mMainWorkItemQueueInternals = nil;
TPtrArray<SWorkItemThreadInfo*>		CWorkItemQueueInternals::mActiveWorkItemThreadInfos;
TPtrArray<SWorkItemThreadInfo*>		CWorkItemQueueInternals::mIdleWorkItemThreadInfos;
CLock								CWorkItemQueueInternals::mWorkItemThreadInfosLock;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CWorkItemQueue

static	CWorkItemQueue*	sMainWorkItemQueue = nil;
CWorkItemQueue&	CWorkItemQueue::mMain = *sMainWorkItemQueue;

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CWorkItemQueue::CWorkItemQueue(UInt32 maximumConcurrentWorkItems)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	bool	sHaveMainWorkItemQueue = false;
	static	bool	sCreatingMainWorkItemQueue = false;

	// Check if need to create main work item queue
	if (!sHaveMainWorkItemQueue && !sCreatingMainWorkItemQueue) {
		// Must create main work item queue first
		sCreatingMainWorkItemQueue = true;
		sMainWorkItemQueue = new CWorkItemQueue(CCoreServices::getTotalProcessorCoresCount() - 1);
		sCreatingMainWorkItemQueue = false;
		sHaveMainWorkItemQueue = true;
	}

	// Check if creating main work item queue
	if (sCreatingMainWorkItemQueue) {
		// In the process of creating main work item queue
		mInternals = new CWorkItemQueueInternals(nil, maximumConcurrentWorkItems);
		CWorkItemQueueInternals::mMainWorkItemQueueInternals = mInternals;
	} else
		// Other Work Item Queue
		mInternals =
				new CWorkItemQueueInternals(CWorkItemQueueInternals::mMainWorkItemQueueInternals,
						maximumConcurrentWorkItems);
}

//----------------------------------------------------------------------------------------------------------------------
CWorkItemQueue::CWorkItemQueue(CWorkItemQueue& targetWorkItemQueue, UInt32 maximumConcurrentWorkItems)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CWorkItemQueueInternals(targetWorkItemQueue.mInternals, maximumConcurrentWorkItems);
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
	// Add
	mInternals->add(workItem, priority);

	// Process work items
	CWorkItemQueueInternals::processWorkItems();
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
	// Pause
	mInternals->pause();
}

//----------------------------------------------------------------------------------------------------------------------
void CWorkItemQueue::resume()
//----------------------------------------------------------------------------------------------------------------------
{
	// Resume
	mInternals->resume();

	// Process work items
	CWorkItemQueueInternals::processWorkItems();
}
