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
// MARK: CProcWorkItem

class CProcWorkItem : public CWorkItem {
	// Methods
	public:
				// Lifecycle methods
				CProcWorkItem(Proc proc, void* userData) : CWorkItem(), mProc(proc), mUserData(userData) {}

				// CWorkItem methods
		void	perform(const I<CWorkItem>& workItem)
					{ mProc(workItem, mUserData); }

	// Properties
	private:
		Proc	mProc;
		void*	mUserData;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CWorkItemInfo

class CWorkItemInfo : public CEquatable {
	public:
				// Lifecycle methods
				CWorkItemInfo(CWorkItemQueueInternals& owningWorkItemQueueInternals, const I<CWorkItem>& workItem,
						CWorkItem::Priority priority) :
					mOwningWorkItemQueueInternals(owningWorkItemQueueInternals), mWorkItem(workItem),
							mPriority(priority), mIndex(CWorkItemInfo::mNextIndex++)
					{}
				CWorkItemInfo(const CWorkItemInfo& other) :
					mOwningWorkItemQueueInternals(other.mOwningWorkItemQueueInternals), mWorkItem(other.mWorkItem),
							mPriority(other.mPriority), mIndex(other.mIndex)
					{}

				// CEquatable methods
		bool	operator==(const CEquatable& other) const
					{ return this == &other; }

				// Instance methods
		void	transitionTo(CWorkItem::State state)
					{ mWorkItem->transitionTo(state); }
		void	perform()
					{ mWorkItem->perform(mWorkItem); }

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
// MARK: - CWorkItemThread

class CWorkItemThread : public CThread {
	public:
		typedef	void	(*WorkItemInfoCompleteProc)(const R<CWorkItemInfo>& workItemInfo,
								CWorkItemThread& workItemThread);

				CWorkItemThread(const CString& name, WorkItemInfoCompleteProc workItemInfoCompleteProc) :
					CThread(name),
							mWorkItemInfoCompleteProc(workItemInfoCompleteProc)
					{ start(); }

		void	run()
					{
						// Run forever
						while (true) {
							// Check for work item info
							if (!mWorkItemInfo.hasValue())
								// Wait
								mSemaphore.waitFor();

							// Setup
							const	R<CWorkItemInfo>	workItemInfo(*mWorkItemInfo);

							// Note active
							workItemInfo->transitionTo(CWorkItem::kStateActive);

							// Process
							workItemInfo->perform();

							// Note completed
							workItemInfo->transitionTo(CWorkItem::kStateCompleted);

							// Reset
							mWorkItemInfo.removeValue();

							// Call proc
							mWorkItemInfoCompleteProc(workItemInfo, *this);
						}
					}
		void	process(const R<CWorkItemInfo>& workItemInfo)
					{
						// Store
						mWorkItemInfo.setValue(workItemInfo);

						// Trigger
						mSemaphore.signal();
					}

	// Properties
	private:
		WorkItemInfoCompleteProc	mWorkItemInfoCompleteProc;
		CSemaphore					mSemaphore;

		OV<R<CWorkItemInfo> >		mWorkItemInfo;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CWorkItemQueueInternals

class CWorkItemQueueInternals {
	public:
									CWorkItemQueueInternals(const OR<CItemsProgress>& itemsProgress,
											UInt32 maximumConcurrentWorkItems,
											OR<CWorkItemQueueInternals> targetWorkItemQueueInternals =
													OR<CWorkItemQueueInternals>()) :
										mItemsProgress(itemsProgress),
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

				void				addChild(CWorkItemQueueInternals& childWorkItemQueueInternals)
										{ mChildWorkItemQueueInternals +=
												R<CWorkItemQueueInternals>(childWorkItemQueueInternals); }
				void				removeChild(CWorkItemQueueInternals& childWorkItemQueueInternals)
										{ mChildWorkItemQueueInternals -=
												R<CWorkItemQueueInternals>(childWorkItemQueueInternals); }

				void				add(const I<CWorkItem>& workItem, CWorkItem::Priority priority)
										{
											// Add
											mWorkItemInfosLock.lock();
											mIdleWorkItemInfos += CWorkItemInfo(*this, workItem, priority);
											mWorkItemInfosLock.unlock();

											// Update
											if (mItemsProgress.hasReference())
												mItemsProgress->addTotalItemsCount(1);
											mInFlightWorkItemsCount.add(1);
										}
				void				cancel(TArray<CWorkItemInfo>::IsMatchProc isMatchProc, void* userData)
										{
											// Update info
											mWorkItemInfosLock.lock();

											// Process active work item infos
											for (TIteratorD<CWorkItemInfo> iterator =
															mActiveWorkItemInfos.getIterator();
													iterator.hasValue(); iterator.advance()) {
												// Check for match
												if (isMatchProc(*iterator, userData))
													// Transition to cancelled.  We do not remove from the array as
													//	by definition, this work item is in progress.
													iterator->transitionTo(CWorkItem::kStateCancelled);
											}

											// Process idle work item infos
											for (CArray::ItemIndex i = mIdleWorkItemInfos.getCount(); i > 0; i--) {
												// Get item
												CWorkItemInfo&	workItemInfo = mIdleWorkItemInfos[i - 1];

												// Check for match
												if (isMatchProc(workItemInfo, userData)) {
													// Transition to cancelled
													workItemInfo.transitionTo(CWorkItem::kStateCancelled);

													// Remove
													mIdleWorkItemInfos.removeAtIndex(i - 1);

													// Update
													if (mItemsProgress.hasReference())
														mItemsProgress->addCompletedItemsCount(1);
													mInFlightWorkItemsCount.subtract(1);
												}
											}

											// All done
											mWorkItemInfosLock.unlock();
										}

				void				pause()
										{ mIsPaused = true; }
				void				resume()
										{ mIsPaused = false; }
				void				wait()
										{ mInFlightWorkItemsCount.wait(); }

				UInt32				getActiveWorkItemInfosCountDeep()
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

				OR<CWorkItemInfo>	getNextWorkItemInfo()
										{
											// Check if paused
											if (mIsPaused)
												// Paused
												return OR<CWorkItemInfo>();

											// Check if have headroom
											if (getActiveWorkItemInfosCountDeep() >= mMaximumConcurrentWorkItems)
												// No more headroom
												return OR<CWorkItemInfo>();

											// Get our next work item info
											mWorkItemInfosLock.lock();
											mIdleWorkItemInfos.sort(workItemInfoCompare);
											OR<CWorkItemInfo>	workItemInfo =
																		!mIdleWorkItemInfos.isEmpty() ?
																				OR<CWorkItemInfo>(
																						mIdleWorkItemInfos[0]) :
																				OR<CWorkItemInfo>();
											mWorkItemInfosLock.unlock();

											// Check any child work item queues
											mChildWorkItemQueueInternals.apply(
													(TNLockingArray<R<CWorkItemQueueInternals> >::ApplyProc)
															getNextWorkItemInfo,
													&workItemInfo);

											return workItemInfo;
										}
				void				moveToActive(CWorkItemInfo& workItemInfo)
										{
											// Move from idle to active
											mWorkItemInfosLock.lock();
											mIdleWorkItemInfos.move(workItemInfo, mActiveWorkItemInfos);
											mWorkItemInfosLock.unlock();
										}
				void				removeFromActive(CWorkItemInfo& workItemInfo)
										{
											// Remove from active
											mWorkItemInfosLock.lock();
											mActiveWorkItemInfos -= workItemInfo;
											mWorkItemInfosLock.unlock();

											// Update
											if (mItemsProgress.hasReference())
												mItemsProgress->addCompletedItemsCount(1);
											mInFlightWorkItemsCount.subtract(1);
										}

		static	void				processWorkItems()
										{
											// Check if can do another workItem
											mWorkItemThreadsLock.lock();

											// Continue until we run out of threads
											while (mActiveWorkItemThreads.getCount() <
													mMainWorkItemQueueInternals->mMaximumConcurrentWorkItems) {
												// Get next work item info
												OR<CWorkItemInfo>	workItemInfo =
																			mMainWorkItemQueueInternals->
																					getNextWorkItemInfo();
												if (!workItemInfo.hasReference())
													// No more work items
													break;

												// Move from idle to active
												workItemInfo->mOwningWorkItemQueueInternals.moveToActive(*workItemInfo);

												// Find thread
												if (mIdleWorkItemThreads.getCount() > 0) {
													// Resume an idle thread
													I<CWorkItemThread>&	workItemThread = mIdleWorkItemThreads[0];
													mIdleWorkItemThreads.move(workItemThread, mActiveWorkItemThreads);

													// Process work item info
													workItemThread->process(R<CWorkItemInfo>(*workItemInfo));
												} else {
													// Create new active thread
													UInt32	threadNumber = mActiveWorkItemThreads.getCount() + 1;
													CString	threadName =
																	CString(OSSTR("CWorkItemQueue #")) +
																			CString(threadNumber);
													I<CWorkItemThread>	workItemThread(
																				new CWorkItemThread(threadName,
																						workItemInfoComplete));
													mActiveWorkItemThreads += workItemThread;

													// Process work item info
													workItemThread->process(R<CWorkItemInfo>(*workItemInfo));
												}

												// Get next work item info
												workItemInfo = mMainWorkItemQueueInternals->getNextWorkItemInfo();
											}
											mWorkItemThreadsLock.unlock();
										}

		static	bool				workItemMatches(const I<CWorkItemInfo>& workItemInfo, I<CWorkItem>* workItem)
										{ return workItemInfo->mWorkItem == *workItem; }
		static	bool				workItemInfoHasID(const CWorkItemInfo& workItemInfo, TSet<CString>* workItemIDs)
										{ return workItemIDs->contains(workItemInfo.mWorkItem->getID()); }
		static	bool				workItemInfoHasReference(const CWorkItemInfo& workItemInfo,
											TSet<CString>* workitemReferences)
										{ return workItemInfo.mWorkItem->getReference().hasValue() &&
												workitemReferences->contains(*workItemInfo.mWorkItem->getReference()); }
		static	bool				workItemInfoAlwaysMatches(const CWorkItemInfo& workItemInfo, void* userData)
										{ return true; }

	private:
		static	void				getActiveWorkItemInfosCountDeep(R<CWorkItemQueueInternals>& workItemQueueInternals,
											void* userData)
										{ *((UInt32*) userData) +=
												workItemQueueInternals->getActiveWorkItemInfosCountDeep(); }
		static	void				getNextWorkItemInfo(R<CWorkItemQueueInternals>& workItemQueueInternals,
											void* userData)
										{
											// Setup
											OR<CWorkItemInfo>&	workItemInfo = *((OR<CWorkItemInfo>*) userData);

											// Update work item info
											OR<CWorkItemInfo>	childWorkItemInfo =
																		workItemQueueInternals->getNextWorkItemInfo();
											if (childWorkItemInfo.hasReference() &&
													(!workItemInfo.hasReference() ||
															(childWorkItemInfo->mPriority > workItemInfo->mPriority) ||
															((childWorkItemInfo->mPriority ==
																			workItemInfo->mPriority) &&
																	(childWorkItemInfo->mIndex <
																			workItemInfo->mIndex))))
												// Use child work item info
												workItemInfo = childWorkItemInfo;
										}

		static	bool				workItemInfoCompare(const CWorkItemInfo& workItemInfo1,
											const CWorkItemInfo& workItemInfo2, void* userData)
										{
											// Sort in this order:
											//	Priority
											//	Index
											if (workItemInfo1.mPriority > workItemInfo2.mPriority)
												// Work item 1 has a higher priority
												return true;
											else if (workItemInfo1.mPriority < workItemInfo2.mPriority)
												// Work item 2 has a higher priority
												return false;
											else
												// Use index
												return workItemInfo1.mIndex < workItemInfo2.mIndex;
										}
		static	void				workItemInfoComplete(const R<CWorkItemInfo>& workItemInfo,
											CWorkItemThread& workItemThread)
										{
											// Transition work item info to complete
											workItemInfo->mOwningWorkItemQueueInternals.removeFromActive(*workItemInfo);

											// Transition thread to idle
											mWorkItemThreadsLock.lock();
											I<CWorkItemThread>&	workItemThreadInstance =
																		*mActiveWorkItemThreads.getFirst(
																				(TArray<I<CWorkItemThread> >::
																								IsMatchProc)
																						I<CWorkItemThread>::
																								doesInstanceMatch,
																				&workItemThread);
											mActiveWorkItemThreads.move(workItemThreadInstance, mIdleWorkItemThreads);
											mWorkItemThreadsLock.unlock();

											// Process work items
											processWorkItems();
										}

	public:
		static	OR<CWorkItemQueueInternals>					mMainWorkItemQueueInternals;

	private:
				OR<CItemsProgress>							mItemsProgress;

				bool										mIsPaused;

				OR<CWorkItemQueueInternals>					mTargetWorkItemQueueInternals;
				UInt32										mMaximumConcurrentWorkItems;

				TNLockingArray<R<CWorkItemQueueInternals> >	mChildWorkItemQueueInternals;

				TNArray<CWorkItemInfo>						mActiveWorkItemInfos;
				TNArray<CWorkItemInfo>						mIdleWorkItemInfos;
				CLock										mWorkItemInfosLock;
				TLockingNumeric<UInt32>						mInFlightWorkItemsCount;

		static	TNArray<I<CWorkItemThread> >				mActiveWorkItemThreads;
		static	TNArray<I<CWorkItemThread> >				mIdleWorkItemThreads;
		static	CLock										mWorkItemThreadsLock;
};

OR<CWorkItemQueueInternals>		CWorkItemQueueInternals::mMainWorkItemQueueInternals;

TNArray<I<CWorkItemThread> >	CWorkItemQueueInternals::mActiveWorkItemThreads;
TNArray<I<CWorkItemThread> >	CWorkItemQueueInternals::mIdleWorkItemThreads;
CLock							CWorkItemQueueInternals::mWorkItemThreadsLock;

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
		sMainWorkItemQueue = new CWorkItemQueue(OR<CItemsProgress>(), -1);
		sCreatingMainWorkItemQueue = false;
	}

	return *sMainWorkItemQueue;
}

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CWorkItemQueue::CWorkItemQueue(const OR<CItemsProgress>& itemsProgress, SInt32 maximumConcurrentWorkItems)
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
		mInternals = new CWorkItemQueueInternals(itemsProgress, desiredMaximumConcurrentWorkItems);
		CWorkItemQueueInternals::mMainWorkItemQueueInternals = OR<CWorkItemQueueInternals>(*mInternals);
	} else
		// Other Work Item Queue
		mInternals =
				new CWorkItemQueueInternals(itemsProgress, desiredMaximumConcurrentWorkItems,
						OR<CWorkItemQueueInternals>(*main().mInternals));
}

//----------------------------------------------------------------------------------------------------------------------
CWorkItemQueue::CWorkItemQueue(CWorkItemQueue& targetWorkItemQueue, const OR<CItemsProgress>& itemsProgress,
		UInt32 maximumConcurrentWorkItems)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals =
			new CWorkItemQueueInternals(itemsProgress, maximumConcurrentWorkItems,
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
I<CWorkItem> CWorkItemQueue::add(CWorkItem::Proc proc, void* userData, CWorkItem::Priority priority)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	I<CWorkItem>	workItem(new CProcWorkItem(proc, userData));

	// Add
	add(workItem, priority);

	return workItem;
}

//----------------------------------------------------------------------------------------------------------------------
void CWorkItemQueue::cancel(const I<CWorkItem>& workItem)
//----------------------------------------------------------------------------------------------------------------------
{
	// Cancel
	mInternals->cancel((TArray<CWorkItemInfo>::IsMatchProc) CWorkItemQueueInternals::workItemMatches,
			(void*) &workItem);
}

//----------------------------------------------------------------------------------------------------------------------
void CWorkItemQueue::cancel(const TSet<CString>& workItemIDs)
//----------------------------------------------------------------------------------------------------------------------
{
	// Cancel
	mInternals->cancel((TArray<CWorkItemInfo>::IsMatchProc) CWorkItemQueueInternals::workItemInfoHasID,
			(void*) &workItemIDs);
}

//----------------------------------------------------------------------------------------------------------------------
void CWorkItemQueue::cancelAll(const TSet<CString>& workItemReferences)
//----------------------------------------------------------------------------------------------------------------------
{
	// Cancel
	mInternals->cancel((TArray<CWorkItemInfo>::IsMatchProc) CWorkItemQueueInternals::workItemInfoHasReference,
			(void*) &workItemReferences);
}

//----------------------------------------------------------------------------------------------------------------------
void CWorkItemQueue::cancelAll()
//----------------------------------------------------------------------------------------------------------------------
{
	// Cancel
	mInternals->cancel((TArray<CWorkItemInfo>::IsMatchProc) CWorkItemQueueInternals::workItemInfoAlwaysMatches,
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
