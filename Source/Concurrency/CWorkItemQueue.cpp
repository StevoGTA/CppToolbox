//----------------------------------------------------------------------------------------------------------------------
//	CWorkItemQueue.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CWorkItemQueue.h"

#include "CArray.h"
#include "CCoreServices.h"
#include "ConcurrencyPrimitives.h"
#include "CThread.h"
#include "TLockingArray.h"

/*
	TODOs:
		-Implement cancel(workItem)
		-Remove cancelled work items immediately
*/

//----------------------------------------------------------------------------------------------------------------------
// MARK: SWorkItemInfo

struct SWorkItemInfo : public CEquatable {
			// Lifecycle methods
			SWorkItemInfo(CWorkItemQueueInternals& owningWorkItemQueueInternals, const I<CWorkItem>& workItem,
					CWorkItem::Priority priority) :
				mOwningWorkItemQueueInternals(owningWorkItemQueueInternals), mWorkItem(workItem), mPriority(priority),
						mIndex(SWorkItemInfo::mNextIndex++)
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

UInt32	SWorkItemInfo::mNextIndex = 0;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - SWorkItemThreadInfo

struct SWorkItemThreadInfo {
	// Lifecycle methods
	SWorkItemThreadInfo(SWorkItemInfo& initialWorkItemInfo, CThread::ThreadProc threadPerform, const CString& name) :
		mThread(threadPerform, this, name, CThread::kOptionsNone), mWorkItemInfo(&initialWorkItemInfo)
		{
			// Start
			mThread.start();
		}

	// Properties
	CSemaphore		mSemaphore;
	CThread			mThread;
	SWorkItemInfo*	mWorkItemInfo;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - SWorkItemQueueReference

struct SWorkItemQueueReference {
									// Lifecycle methods
									SWorkItemQueueReference(CWorkItemQueueInternals& workItemQueueInternals) :
										mWorkItemQueueInternals(workItemQueueInternals)
										{}

									// Instance methods
			UInt32					getActiveWorkItemInfosCountDeep();
			OR<I<SWorkItemInfo> >	getNextWorkItemInfo();

									// Class methods
	static	void					getActiveWorkItemInfosCountDeep(SWorkItemQueueReference& workItemQueueReference,
											void* userData)
										{
											// Setup
											UInt32&	count = *((UInt32*) userData);

											// Update count
											count += workItemQueueReference.getActiveWorkItemInfosCountDeep();
										}
	static	void					getNextWorkItemInfo(SWorkItemQueueReference& workItemQueueReference, void* userData)
										{
											// Setup
											OR<I<SWorkItemInfo> >&	workItemInfo = *((OR<I<SWorkItemInfo> >*) userData);

											// Update work item info
											OR<I<SWorkItemInfo> >	childWorkItemInfo =
																			workItemQueueReference
																					.getNextWorkItemInfo();
											if (childWorkItemInfo.hasReference() &&
													(!workItemInfo.hasReference() ||
															((*childWorkItemInfo)->mIndex < (*workItemInfo)->mIndex)))
												// Start with this child work item info
												workItemInfo = childWorkItemInfo;
										}

	// Properties
	CWorkItemQueueInternals&	mWorkItemQueueInternals;
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
												// Check if have target
												if (mTargetWorkItemQueueInternals.hasReference()) {
													// Add as child
													mWorkItemQueueReference = new SWorkItemQueueReference(*this);
													mTargetWorkItemQueueInternals->mChildWorkItemQueueReferences +=
															mWorkItemQueueReference;
												} else
													// No target
													mWorkItemQueueReference = nil;
											}
										~CWorkItemQueueInternals()
											{
												// Check if have target
												if (mTargetWorkItemQueueInternals.hasReference()) {
													// Remove as child
													mTargetWorkItemQueueInternals->mChildWorkItemQueueReferences -=
															*mWorkItemQueueReference;
												}
											}

				void					add(const I<CWorkItem>& workItem, CWorkItem::Priority priority)
											{
												// Add
												mWorkItemInfosLock.lock();
												mIdleWorkItemInfos +=
														I<SWorkItemInfo>(new SWorkItemInfo(*this, workItem, priority));
												mWorkItemInfosLock.unlock();
											}
				void					cancel(TArray<I<SWorkItemInfo> >::IsMatchProc isMatchProc, void* userData)
											{
												// Update info
												mWorkItemInfosLock.lock();

												// Process active work item infos
												for (TIteratorD<I<SWorkItemInfo> > iterator =
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
													I<SWorkItemInfo>&	workItemInfo = mIdleWorkItemInfos[i - 1];

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

				UInt32					getActiveWorkItemInfosCountDeep()
											{
												// Setup
												UInt32	count = 0;

												// Add our active work items
												mWorkItemInfosLock.lock();
												count += mActiveWorkItemInfos.getCount();
												mWorkItemInfosLock.unlock();

												// Add active work items from all child work item queues
												mChildWorkItemQueueReferences.apply(
														SWorkItemQueueReference::getActiveWorkItemInfosCountDeep,
														&count);

												return count;
											}
				OR<I<SWorkItemInfo> >	getNextWorkItemInfo()
											{
												// Check if paused
												if (mIsPaused)
													// Paused
													return OR<I<SWorkItemInfo> >();

												// Check if have headroom
												if (getActiveWorkItemInfosCountDeep() >= mMaximumConcurrentWorkItems)
													// No more headroom
													return OR<I<SWorkItemInfo> >();

												// Get our next work item info
												mWorkItemInfosLock.lock();
												mIdleWorkItemInfos.sort(workItemInfoCompare);
												OR<I<SWorkItemInfo> >	workItemInfo =
																				!mIdleWorkItemInfos.isEmpty() ?
																						OR<I<SWorkItemInfo> >(
																								mIdleWorkItemInfos[0]) :
																						OR<I<SWorkItemInfo> >();
												mWorkItemInfosLock.unlock();

												// Check any child work item queues
												mChildWorkItemQueueReferences.apply(
														SWorkItemQueueReference::getNextWorkItemInfo, &workItemInfo);

												return workItemInfo;
											}

		static	void					processWorkItems()
											{
												// Check if can do another workItem
												mWorkItemThreadInfosLock.lock();

												// Get next work item info
												OR<I<SWorkItemInfo> >	workItemInfo =
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

		static	bool					workItemInfoMatches(const I<SWorkItemInfo>& workItemInfo, CWorkItem* workItem)
											{ return workItemInfo->mWorkItem->getID() == workItem->getID(); }
		static	bool					workItemInfoHasID(const I<SWorkItemInfo>& workItemInfo,
												TSet<CString>* workItemIDs)
											{ return workItemIDs->contains(workItemInfo->mWorkItem->getID()); }
		static	bool					workItemInfoHasReference(const I<SWorkItemInfo>& workItemInfo,
												TSet<CString>* workitemReferences)
											{ return workItemInfo->mWorkItem->getReference().hasValue() &&
													workitemReferences->contains(
															*workItemInfo->mWorkItem->getReference()); }
		static	bool					workItemInfoAlwaysMatches(const I<SWorkItemInfo>& workItemInfo, void* userData)
											{ return true; }

	private:
		static	bool					workItemInfoCompare(const I<SWorkItemInfo>& workItemInfo1,
												const I<SWorkItemInfo>& workItemInfo2, void* userData)
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
													SWorkItemInfo&				workItemInfo =
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
													OR<I<SWorkItemInfo> >	workItemInfoReference;
													workItemInfoReference =
															workItemQueueInternals.mActiveWorkItemInfos.
																	getFirst(
																			(TArray<I<SWorkItemInfo> >::IsMatchProc)
																					I<SWorkItemInfo>::doesInstanceMatch,
																			&workItemInfo);

													workItemQueueInternals.mWorkItemInfosLock.lock();
													workItemQueueInternals.mActiveWorkItemInfos -=
															*workItemInfoReference;
													workItemQueueInternals.mWorkItemInfosLock.unlock();

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
		static	OR<CWorkItemQueueInternals>				mMainWorkItemQueueInternals;

	private:
				bool									mIsPaused;

				OR<CWorkItemQueueInternals>				mTargetWorkItemQueueInternals;
				UInt32									mMaximumConcurrentWorkItems;

				SWorkItemQueueReference*				mWorkItemQueueReference;
				TILockingArray<SWorkItemQueueReference>	mChildWorkItemQueueReferences;

				TNArray<I<SWorkItemInfo> >				mActiveWorkItemInfos;
				TNArray<I<SWorkItemInfo> >				mIdleWorkItemInfos;
				CLock									mWorkItemInfosLock;

		static	TNArray<I<SWorkItemThreadInfo> >		mActiveWorkItemThreadInfos;
		static	TNArray<I<SWorkItemThreadInfo> >		mIdleWorkItemThreadInfos;
		static	CLock									mWorkItemThreadInfosLock;
};

OR<CWorkItemQueueInternals>			CWorkItemQueueInternals::mMainWorkItemQueueInternals;
TNArray<I<SWorkItemThreadInfo> >	CWorkItemQueueInternals::mActiveWorkItemThreadInfos;
TNArray<I<SWorkItemThreadInfo> >	CWorkItemQueueInternals::mIdleWorkItemThreadInfos;
CLock								CWorkItemQueueInternals::mWorkItemThreadInfosLock;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - SWorkItemQueueReference

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
UInt32 SWorkItemQueueReference::getActiveWorkItemInfosCountDeep()
//----------------------------------------------------------------------------------------------------------------------
{
	return mWorkItemQueueInternals.getActiveWorkItemInfosCountDeep();
}

//----------------------------------------------------------------------------------------------------------------------
OR<I<SWorkItemInfo> > SWorkItemQueueReference::getNextWorkItemInfo()
//----------------------------------------------------------------------------------------------------------------------
{
	return mWorkItemQueueInternals.getNextWorkItemInfo();
}

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
				new CWorkItemQueueInternals(maximumConcurrentWorkItems,
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
	mInternals->cancel((TArray<I<SWorkItemInfo> >::IsMatchProc) CWorkItemQueueInternals::workItemInfoMatches,
			&workItem);
}

//----------------------------------------------------------------------------------------------------------------------
void CWorkItemQueue::cancel(const TSet<CString>& workItemIDs)
//----------------------------------------------------------------------------------------------------------------------
{
	// Cancel
	mInternals->cancel((TArray<I<SWorkItemInfo> >::IsMatchProc) CWorkItemQueueInternals::workItemInfoHasID,
			(void*) &workItemIDs);
}

//----------------------------------------------------------------------------------------------------------------------
void CWorkItemQueue::cancelAll(const TSet<CString>& workItemReferences)
//----------------------------------------------------------------------------------------------------------------------
{
	// Cancel
	mInternals->cancel((TArray<I<SWorkItemInfo> >::IsMatchProc) CWorkItemQueueInternals::workItemInfoHasReference,
			(void*) &workItemReferences);
}

//----------------------------------------------------------------------------------------------------------------------
void CWorkItemQueue::cancelAll()
//----------------------------------------------------------------------------------------------------------------------
{
	// Cancel
	mInternals->cancel((TArray<I<SWorkItemInfo> >::IsMatchProc) CWorkItemQueueInternals::workItemInfoAlwaysMatches,
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
