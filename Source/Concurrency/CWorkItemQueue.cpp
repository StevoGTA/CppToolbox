//----------------------------------------------------------------------------------------------------------------------
//	CWorkItemQueue.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CWorkItemQueue.h"

#include "CArray.h"
#include "CCoreServices.h"
#include "ConcurrencyPrimitives.h"
#include "CThread.h"
#include "TLockingArray.h"
#include "TLockingValue.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CWorkItemInfo

class CWorkItemInfo : public CEquatable {
	public:
				// Lifecycle methods
				CWorkItemInfo(CWorkItemQueueInternals& owningWorkItemQueueInternals, const I<CWorkItem>& workItem,
						CWorkItem::Priority priority) :
					mOwningWorkItemQueueInternals(owningWorkItemQueueInternals), mWorkItem(workItem),
							mPriority(priority), mIndex(CWorkItemInfo::mNextIndex++)
					{}

				// CEquatable methods
		bool	operator==(const CEquatable& other) const
					{ return this == &other; }

				// Instance methods
		void	transitionTo(CWorkItem::State state)
					{ mWorkItem->transitionTo(state); }
		void	perform()
					{ mWorkItem->perform(); }

		// Properties
				CWorkItemQueueInternals&	mOwningWorkItemQueueInternals;
				I<CWorkItem>				mWorkItem;
				CWorkItem::Priority			mPriority;
				UInt32						mIndex;

		static	UInt32						mNextIndex;
};

UInt32	CWorkItemInfo::mNextIndex = 0;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - SWorkItemThreadInfo

struct SWorkItemThreadInfo {
	// Lifecycle methods
	SWorkItemThreadInfo(CWorkItemInfo& initialWorkItemInfo, CThread::ThreadProc threadPerform, const CString& name) :
		mThread(threadPerform, this, name, CThread::kOptionsNone), mWorkItemInfo(&initialWorkItemInfo)
		{
			// Start
			mThread.start();
		}

	// Properties
	CSemaphore		mSemaphore;
	CThread			mThread;
	CWorkItemInfo*	mWorkItemInfo;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CWorkItemQueueInternals

class CWorkItemQueueInternals {
	public:
										CWorkItemQueueInternals(UInt32 maximumConcurrentWorkItems,
												OR<CWorkItemQueueInternals> targetWorkItemQueueInternals =
														OR<CWorkItemQueueInternals>()) :
											mIsPaused(false),
													mTargetWorkItemQueueInternals(targetWorkItemQueueInternals),
													mMaximumConcurrentWorkItems(maximumConcurrentWorkItems)
											{
												if (mTargetWorkItemQueueInternals.hasReference())
													// Add as child
													mTargetWorkItemQueueInternals->addChild(*this);
											}
										~CWorkItemQueueInternals()
											{
												// Check if have target
												if (mTargetWorkItemQueueInternals.hasReference())
													// Remove as child
													mTargetWorkItemQueueInternals->removeChild(*this);
											}

				void					addChild(CWorkItemQueueInternals& childWorkItemQueueInternals)
											{ mChildWorkItemQueueInternals +=
													R<CWorkItemQueueInternals>(childWorkItemQueueInternals); }
				void					removeChild(CWorkItemQueueInternals& childWorkItemQueueInternals)
											{ mChildWorkItemQueueInternals -=
													R<CWorkItemQueueInternals>(childWorkItemQueueInternals); }

				void					add(const I<CWorkItem>& workItem, CWorkItem::Priority priority)
											{
												// Add
												mWorkItemInfosLock.lock();
												mIdleWorkItemInfos +=
														I<CWorkItemInfo>(new CWorkItemInfo(*this, workItem, priority));
												mWorkItemInfosLock.unlock();

												mTotalWorkItemsCount.add(1);
											}
				void					cancel(TArray<I<CWorkItemInfo> >::IsMatchProc isMatchProc, void* userData)
											{
												// Update info
												mWorkItemInfosLock.lock();

												// Process active work item infos
												for (TIteratorD<I<CWorkItemInfo> > iterator =
																mActiveWorkItemInfos.getIterator();
														iterator.hasValue(); iterator.advance()) {
													// Check for match
													if (isMatchProc(*iterator, userData))
														// Transition to cancelled.  We do not remove from the array as
														//	by definition, this work item is in progress.
														(*iterator)->transitionTo(CWorkItem::kStateCancelled);
												}

												// Process idle work item infos
												for (CArray::ItemIndex i = mIdleWorkItemInfos.getCount(); i > 0; i--) {
													// Get item
													I<CWorkItemInfo>&	workItemInfo = mIdleWorkItemInfos[i - 1];

													// Check for match
													if (isMatchProc(workItemInfo, userData)) {
														// Transition to cancelled
														workItemInfo->transitionTo(CWorkItem::kStateCancelled);

														// Remove
														mIdleWorkItemInfos.removeAtIndex(i - 1);
													}
												}

												// All done
												mWorkItemInfosLock.unlock();
											}

				void					pause()
											{ mIsPaused = true; }
				void					resume()
											{ mIsPaused = false; }
				void					wait()
											{ mTotalWorkItemsCount.wait(); }

				UInt32					getActiveWorkItemInfosCountDeep()
											{
												// Setup
												UInt32	count = 0;

												// Add our active work items
												mWorkItemInfosLock.lock();
												count += mActiveWorkItemInfos.getCount();
												mWorkItemInfosLock.unlock();

												// Add active work items from all child work item queues
												mChildWorkItemQueueInternals.apply(
														(TNLockingArray<R<CWorkItemQueueInternals> >::ApplyProc)
																getActiveWorkItemInfosCountDeep,
														&count);

												return count;
											}

				OR<I<CWorkItemInfo> >	getNextWorkItemInfo()
											{
												// Check if paused
												if (mIsPaused)
													// Paused
													return OR<I<CWorkItemInfo> >();

												// Check if have headroom
												if (getActiveWorkItemInfosCountDeep() >= mMaximumConcurrentWorkItems)
													// No more headroom
													return OR<I<CWorkItemInfo> >();

												// Get our next work item info
												mWorkItemInfosLock.lock();
												mIdleWorkItemInfos.sort(workItemInfoCompare);
												OR<I<CWorkItemInfo> >	workItemInfo =
																				!mIdleWorkItemInfos.isEmpty() ?
																						OR<I<CWorkItemInfo> >(
																								mIdleWorkItemInfos[0]) :
																						OR<I<CWorkItemInfo> >();
												mWorkItemInfosLock.unlock();

												// Check any child work item queues
												mChildWorkItemQueueInternals.apply(
														(TNLockingArray<R<CWorkItemQueueInternals> >::ApplyProc)
																getNextWorkItemInfo,
														&workItemInfo);

												return workItemInfo;
											}

		static	void					processWorkItems()
											{
												// Check if can do another workItem
												mWorkItemThreadInfosLock.lock();

												// Get next work item info
												OR<I<CWorkItemInfo> >	workItemInfo =
																				mMainWorkItemQueueInternals->
																						getNextWorkItemInfo();
												while (
														(mActiveWorkItemThreadInfos.getCount() <
																mMainWorkItemQueueInternals->
																		mMaximumConcurrentWorkItems) &&
														workItemInfo.hasReference()) {
													// Move from idle to active
													(*workItemInfo)->mOwningWorkItemQueueInternals.mWorkItemInfosLock
															.lock();
													(*workItemInfo)->mOwningWorkItemQueueInternals.mIdleWorkItemInfos
															.move(
																	*workItemInfo,
																	(*workItemInfo)->mOwningWorkItemQueueInternals
																			.mActiveWorkItemInfos);
													(*workItemInfo)->transitionTo(CWorkItem::kStateActive);
													(*workItemInfo)->mOwningWorkItemQueueInternals.mWorkItemInfosLock
															.unlock();

													// Find thread
													if (mIdleWorkItemThreadInfos.getCount() > 0) {
														// Resume an idle thread
														I<SWorkItemThreadInfo>&	workItemThreadInfo =
																						mIdleWorkItemThreadInfos
																								.getFirst();
														workItemThreadInfo->mWorkItemInfo = &**workItemInfo;
														workItemThreadInfo->mSemaphore.signal();

														mIdleWorkItemThreadInfos.move(workItemThreadInfo,
																mActiveWorkItemThreadInfos);
													} else {
														// Create new active thread
														UInt32	threadNumber =
																		mActiveWorkItemThreadInfos.getCount() + 1;
														CString	threadName =
																		CString(OSSTR("CWorkItemQueue #")) +
																				CString(threadNumber);
														mActiveWorkItemThreadInfos +=
																I<SWorkItemThreadInfo>(
																		new SWorkItemThreadInfo(**workItemInfo,
																				threadPerform, threadName));
													}

													// Get next work item info
													workItemInfo = mMainWorkItemQueueInternals->getNextWorkItemInfo();
												}
												mWorkItemThreadInfosLock.unlock();
											}

		static	bool					workItemInfoMatches(const I<CWorkItemInfo>& workItemInfo, CWorkItem* workItem)
											{ return workItemInfo->mWorkItem->getID() == workItem->getID(); }
		static	bool					workItemInfoHasID(const I<CWorkItemInfo>& workItemInfo,
												TSet<CString>* workItemIDs)
											{ return workItemIDs->contains(workItemInfo->mWorkItem->getID()); }
		static	bool					workItemInfoHasReference(const I<CWorkItemInfo>& workItemInfo,
												TSet<CString>* workitemReferences)
											{ return workItemInfo->mWorkItem->getReference().hasValue() &&
													workitemReferences->contains(
															*workItemInfo->mWorkItem->getReference()); }
		static	bool					workItemInfoAlwaysMatches(const I<CWorkItemInfo>& workItemInfo, void* userData)
											{ return true; }

	private:
		static	void					getActiveWorkItemInfosCountDeep(
												R<CWorkItemQueueInternals>& workItemQueueInternals, void* userData)
											{ *((UInt32*) userData) +=
													workItemQueueInternals->getActiveWorkItemInfosCountDeep(); }
		static	void					getNextWorkItemInfo(
												R<CWorkItemQueueInternals>& workItemQueueInternals, void* userData)
											{
												// Setup
												OR<I<CWorkItemInfo> >*	workItemInfo =
																				(OR<I<CWorkItemInfo> >*) userData;

												// Update work item info
												OR<I<CWorkItemInfo> >	childWorkItemInfo =
																				workItemQueueInternals
																						->getNextWorkItemInfo();
												if (childWorkItemInfo.hasReference() &&
														(!workItemInfo->hasReference() ||
																((*childWorkItemInfo)->mIndex <
																		(**workItemInfo)->mIndex)))
													// Start with this child work item info
													*workItemInfo = childWorkItemInfo;
											}

		static	bool					workItemInfoCompare(const I<CWorkItemInfo>& workItemInfo1,
												const I<CWorkItemInfo>& workItemInfo2, void* userData)
											{
												// Sort in this order:
												//	Priority
												//	Index
												if (workItemInfo1->mPriority < workItemInfo2->mPriority)
													// Work item 1 has a higher priority
													return true;
												else if (workItemInfo2->mPriority < workItemInfo1->mPriority)
													// Work item 2 has a higher priority
													return false;
												else
													// Use index
													return workItemInfo1->mIndex < workItemInfo2->mIndex;
											}
		static	void					threadPerform(CThread& thread, void* userData)
											{
												// Setup
												SWorkItemThreadInfo&	workItemThreadInfo =
																				*((SWorkItemThreadInfo*) userData);

												// Run forever
												while (true) {
													// Setup
													CWorkItemInfo&				workItemInfo =
																						*workItemThreadInfo
																								.mWorkItemInfo;
													CWorkItemQueueInternals&	workItemQueueInternals =
																						workItemInfo
																								.mOwningWorkItemQueueInternals;

													// Perform
													workItemInfo.perform();

													// Note completed
													workItemInfo.transitionTo(CWorkItem::kStateCompleted);
													workItemThreadInfo.mWorkItemInfo = nil;

													// Remove work item info from active
													OR<I<CWorkItemInfo> >	workItemInfoReference;
													workItemInfoReference =
															workItemQueueInternals.mActiveWorkItemInfos.
																	getFirst(
																			(TArray<I<CWorkItemInfo> >::IsMatchProc)
																					I<CWorkItemInfo>::doesInstanceMatch,
																			&workItemInfo);

													workItemQueueInternals.mWorkItemInfosLock.lock();
													workItemQueueInternals.mActiveWorkItemInfos -=
															*workItemInfoReference;
													workItemQueueInternals.mWorkItemInfosLock.unlock();

													workItemQueueInternals.mTotalWorkItemsCount.subtract(1);

													// Move this thread to idle
													OR<I<SWorkItemThreadInfo> >	workItemThreadInfoReference;
													workItemThreadInfoReference =
															mActiveWorkItemThreadInfos.
																	getFirst(
																			(TArray<I<SWorkItemThreadInfo> >::
																							IsMatchProc)
																					I<SWorkItemThreadInfo>::
																							doesInstanceMatch,
																			&workItemThreadInfo);
													mWorkItemThreadInfosLock.lock();
													mActiveWorkItemThreadInfos.move(*workItemThreadInfoReference,
															mIdleWorkItemThreadInfos);
													mWorkItemThreadInfosLock.unlock();

													// Process work items
													processWorkItems();

													// Check if waiting on workItem info
													if (workItemThreadInfo.mWorkItemInfo == nil)
														// Wait
														workItemThreadInfo.mSemaphore.waitFor();
												}
											}

	public:
		static	OR<CWorkItemQueueInternals>					mMainWorkItemQueueInternals;

	private:
				bool										mIsPaused;

				OR<CWorkItemQueueInternals>					mTargetWorkItemQueueInternals;
				UInt32										mMaximumConcurrentWorkItems;

				TNLockingArray<R<CWorkItemQueueInternals> >	mChildWorkItemQueueInternals;

				TNArray<I<CWorkItemInfo> >					mActiveWorkItemInfos;
				TNArray<I<CWorkItemInfo> >					mIdleWorkItemInfos;
				CLock										mWorkItemInfosLock;
				TLockingNumeric<UInt32>						mTotalWorkItemsCount;

		static	TNArray<I<SWorkItemThreadInfo> >			mActiveWorkItemThreadInfos;
		static	TNArray<I<SWorkItemThreadInfo> >			mIdleWorkItemThreadInfos;
		static	CLock										mWorkItemThreadInfosLock;
};

OR<CWorkItemQueueInternals>			CWorkItemQueueInternals::mMainWorkItemQueueInternals;
TNArray<I<SWorkItemThreadInfo> >	CWorkItemQueueInternals::mActiveWorkItemThreadInfos;
TNArray<I<SWorkItemThreadInfo> >	CWorkItemQueueInternals::mIdleWorkItemThreadInfos;
CLock								CWorkItemQueueInternals::mWorkItemThreadInfosLock;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CWorkItemQueue

static	bool	sCreatingMainWorkItemQueue = false;

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CWorkItemQueue& CWorkItemQueue::main()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	static	CWorkItemQueue*	sMainWorkItemQueue = nil;

	// Check if have main work item queue
	if (sMainWorkItemQueue == nil) {
		// Create main work item queue
		sCreatingMainWorkItemQueue = true;
		sMainWorkItemQueue = new CWorkItemQueue(-1);
		sCreatingMainWorkItemQueue = false;
	}

	return *sMainWorkItemQueue;
}

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CWorkItemQueue::CWorkItemQueue(SInt32 maximumConcurrentWorkItems)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt32	totalProcessorCoresCount = CCoreServices::getTotalProcessorCoresCount();
	UInt32	desiredMaximumConcurrentWorkItems;
	if (maximumConcurrentWorkItems > 0)
		// Requesting desired concurrency
		desiredMaximumConcurrentWorkItems = std::min<UInt32>(maximumConcurrentWorkItems, totalProcessorCoresCount);
	else if (maximumConcurrentWorkItems < 0)
		// Requesting up to certain capacity
		desiredMaximumConcurrentWorkItems = std::max<SInt32>(totalProcessorCoresCount + maximumConcurrentWorkItems, 1);
	else
		// Didn't request anything.  Weird.
		desiredMaximumConcurrentWorkItems = 1;
	
	// Check if creating main work item queue
	if (sCreatingMainWorkItemQueue) {
		// Main work item queue
		mInternals = new CWorkItemQueueInternals(desiredMaximumConcurrentWorkItems);
		CWorkItemQueueInternals::mMainWorkItemQueueInternals = OR<CWorkItemQueueInternals>(*mInternals);
	} else
		// Other Work Item Queue
		mInternals =
				new CWorkItemQueueInternals(desiredMaximumConcurrentWorkItems,
						OR<CWorkItemQueueInternals>(*main().mInternals));
}

//----------------------------------------------------------------------------------------------------------------------
CWorkItemQueue::CWorkItemQueue(CWorkItemQueue& targetWorkItemQueue, UInt32 maximumConcurrentWorkItems)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals =
			new CWorkItemQueueInternals(maximumConcurrentWorkItems,
					OR<CWorkItemQueueInternals>(*targetWorkItemQueue.mInternals));
}

//----------------------------------------------------------------------------------------------------------------------
CWorkItemQueue::~CWorkItemQueue()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CWorkItemQueue::add(const I<CWorkItem>& workItem, CWorkItem::Priority priority)
//----------------------------------------------------------------------------------------------------------------------
{
	// Add
	mInternals->add(workItem, priority);

	// Process work items
	CWorkItemQueueInternals::processWorkItems();
}

//----------------------------------------------------------------------------------------------------------------------
void CWorkItemQueue::cancel(CWorkItem& workItem)
//----------------------------------------------------------------------------------------------------------------------
{
	// Cancel
	mInternals->cancel((TArray<I<CWorkItemInfo> >::IsMatchProc) CWorkItemQueueInternals::workItemInfoMatches,
			&workItem);
}

//----------------------------------------------------------------------------------------------------------------------
void CWorkItemQueue::cancel(const TSet<CString>& workItemIDs)
//----------------------------------------------------------------------------------------------------------------------
{
	// Cancel
	mInternals->cancel((TArray<I<CWorkItemInfo> >::IsMatchProc) CWorkItemQueueInternals::workItemInfoHasID,
			(void*) &workItemIDs);
}

//----------------------------------------------------------------------------------------------------------------------
void CWorkItemQueue::cancelAll(const TSet<CString>& workItemReferences)
//----------------------------------------------------------------------------------------------------------------------
{
	// Cancel
	mInternals->cancel((TArray<I<CWorkItemInfo> >::IsMatchProc) CWorkItemQueueInternals::workItemInfoHasReference,
			(void*) &workItemReferences);
}

//----------------------------------------------------------------------------------------------------------------------
void CWorkItemQueue::cancelAll()
//----------------------------------------------------------------------------------------------------------------------
{
	// Cancel
	mInternals->cancel((TArray<I<CWorkItemInfo> >::IsMatchProc) CWorkItemQueueInternals::workItemInfoAlwaysMatches,
			nil);
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

//----------------------------------------------------------------------------------------------------------------------
void CWorkItemQueue::wait() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Wait
	mInternals->wait();
}
