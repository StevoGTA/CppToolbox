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
			SWorkItemInfo(CWorkItemQueueInternals& owningWorkItemQueueInternals, CWorkItem& workItem,
					CWorkItem::Priority priority) :
				mOwningWorkItemQueueInternals(owningWorkItemQueueInternals), mWorkItem(workItem), mPriority(priority),
						mIndex(SWorkItemInfo::mNextIndex++)
				{}
			SWorkItemInfo(CWorkItemQueueInternals& owningWorkItemQueueInternals, CProcWorkItem::Proc proc,
					void* userData, CWorkItem::Priority priority) :
				mOwningWorkItemQueueInternals(owningWorkItemQueueInternals),
						mProcWorkItem(new CProcWorkItem(proc, userData)), mPriority(priority),
						mIndex(SWorkItemInfo::mNextIndex++)
				{}

			// CEquatable methods
	bool	operator==(const CEquatable& other) const
				{ return this == &other; }

			// Instance methods
	void	transitionTo(CWorkItem::State state)
				{
					// Check what we have
					if (mWorkItem.hasReference())
						// Work item
						mWorkItem->transitionTo(state);
					else
						// Proc work item
						mProcWorkItem->transitionTo(state);
				}
	void	perform()
				{
					// Check what we have
					if (mWorkItem.hasReference())
						// Work item
						mWorkItem->perform();
					else
						// Proc work item
						mProcWorkItem->perform();
				}

	// Properties
			CWorkItemQueueInternals&	mOwningWorkItemQueueInternals;
			OR<CWorkItem>				mWorkItem;
			OI<CProcWorkItem>			mProcWorkItem;
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
	SWorkItemThreadInfo(SWorkItemInfo& initialWorkItemInfo, CThread::ThreadProc threadProc, const CString& name) :
		mThread(threadProc, this, name), mWorkItemInfo(&initialWorkItemInfo)
		{}

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
			UInt32				getActiveWorkItemInfosCountDeep();
			OR<SWorkItemInfo>	getNextWorkItemInfo();

								// Class methods
	static	void				getActiveWorkItemInfosCountDeep(SWorkItemQueueReference& workItemQueueReference,
										void* userData)
									{
										// Setup
										UInt32&	count = *((UInt32*) userData);

										// Update count
										count += workItemQueueReference.getActiveWorkItemInfosCountDeep();
									}
	static	void				getNextWorkItemInfo(SWorkItemQueueReference& workItemQueueReference, void* userData)
									{
										// Setup
										OR<SWorkItemInfo>&	workItemInfo = *((OR<SWorkItemInfo>*) userData);

										// Update work item info
										OR<SWorkItemInfo>	childWorkItemInfo =
																	workItemQueueReference.getNextWorkItemInfo();
										if (childWorkItemInfo.hasReference() &&
												(!workItemInfo.hasReference() ||
														(childWorkItemInfo->mIndex < workItemInfo->mIndex)))
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
										mIsPaused(false), mTargetWorkItemQueueInternals(targetWorkItemQueueInternals),
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

				void				add(CWorkItem& workItem, CWorkItem::Priority priority)
										{
											// Add
											mWorkItemInfosLock.lock();
											mIdleWorkItemInfos += new SWorkItemInfo(*this, workItem, priority);
											mWorkItemInfosLock.unlock();
										}
				CWorkItem&			add(CProcWorkItem::Proc proc, void* userData, CWorkItem::Priority priority)
										{
											// Add
											mWorkItemInfosLock.lock();
											mIdleWorkItemInfos += new SWorkItemInfo(*this, proc, userData, priority);
											CWorkItem&	workItem = *mIdleWorkItemInfos.getLast().mProcWorkItem;
											mWorkItemInfosLock.unlock();

											return workItem;
										}

				void				pause()
										{ mIsPaused = true; }
				void				resume()
										{ mIsPaused = false; }

		static	void				processWorkItems()
										{
											// Check if can do another workItem
											mWorkItemThreadInfosLock.lock();

											// Get next work item info
											OR<SWorkItemInfo>	workItemInfo =
																		mMainWorkItemQueueInternals->
																				getNextWorkItemInfo();
											while (
													(mActiveWorkItemThreadInfos.getCount() <
															mMainWorkItemQueueInternals->mMaximumConcurrentWorkItems) &&
													workItemInfo.hasReference()) {
												// Move from idle to active
												workItemInfo->mOwningWorkItemQueueInternals.mWorkItemInfosLock.lock();
												workItemInfo->mOwningWorkItemQueueInternals.mIdleWorkItemInfos.move(
														*workItemInfo,
														workItemInfo->mOwningWorkItemQueueInternals
																.mActiveWorkItemInfos);
												workItemInfo->transitionTo(CWorkItem::kStateActive);
												workItemInfo->mOwningWorkItemQueueInternals.mWorkItemInfosLock.unlock();

												// Find thread
												if (mIdleWorkItemThreadInfos.getCount() > 0) {
													// Resume an idle thread
													SWorkItemThreadInfo&	workItemThreadInfo =
																					mIdleWorkItemThreadInfos.getFirst();
													workItemThreadInfo.mWorkItemInfo = &*workItemInfo;
													workItemThreadInfo.mSemaphore.signal();

													mIdleWorkItemThreadInfos.move(workItemThreadInfo,
															mActiveWorkItemThreadInfos);
												} else {
													// Create new active thread
													UInt32	threadNumber = mActiveWorkItemThreadInfos.getCount() + 1;
													CString	threadName =
																	CString(OSSTR("CWorkItemQueue #")) +
																			CString(threadNumber);
													mActiveWorkItemThreadInfos +=
															new SWorkItemThreadInfo(*workItemInfo, threadProc,
																	threadName);
												}

												// Get next work item info
												workItemInfo = mMainWorkItemQueueInternals->getNextWorkItemInfo();
											}
											mWorkItemThreadInfosLock.unlock();
										}

				UInt32				getActiveWorkItemInfosCountDeep()
										{
											// Setup
											UInt32	count = 0;

											// Add our active work items
											mWorkItemInfosLock.lock();
											count += mActiveWorkItemInfos.getCount();
											mWorkItemInfosLock.unlock();

											// Add active work items from all child work item queues
											mChildWorkItemQueueReferences.apply(
													SWorkItemQueueReference::getActiveWorkItemInfosCountDeep, &count);

											return count;
										}
				OR<SWorkItemInfo>	getNextWorkItemInfo()
										{
											// Check if paused
											if (mIsPaused)
												// Paused
												return OR<SWorkItemInfo>();

											// Check if have headroom
											if (getActiveWorkItemInfosCountDeep() >= mMaximumConcurrentWorkItems)
												// No more headroom
												return OR<SWorkItemInfo>();

											// Get our next work item info
											mWorkItemInfosLock.lock();
											mIdleWorkItemInfos.sort(workItemInfoCompareProc);
											OR<SWorkItemInfo>	workItemInfo =
																		!mIdleWorkItemInfos.isEmpty() ?
																				OR<SWorkItemInfo>(mIdleWorkItemInfos[0]) :
																				OR<SWorkItemInfo>();
											mWorkItemInfosLock.unlock();

											// Check any child work item queues
											mChildWorkItemQueueReferences.apply(
													SWorkItemQueueReference::getNextWorkItemInfo, &workItemInfo);

											return workItemInfo;
										}

	private:
		static	ECompareResult		workItemInfoCompareProc(const SWorkItemInfo& workItemInfo1,
											const SWorkItemInfo& workItemInfo2, void* userData)
										{
											// Sort in this order:
											//	Priority
											//	Index
											if (workItemInfo1.mPriority < workItemInfo2.mPriority)
												// Work item 1 has a higher priority
												return kCompareResultBefore;
											else if (workItemInfo2.mPriority < workItemInfo1.mPriority)
												// Work item 2 has a higher priority
												return kCompareResultAfter;
											else if (workItemInfo1.mIndex < workItemInfo2.mIndex)
												// Work item 1 has an earlier index
												return kCompareResultBefore;
											else
												// Work item 1 has a later index
												return kCompareResultAfter;
										}
		static	void				threadProc(CThread& thread, void* userData)
										{
											// Setup
											SWorkItemThreadInfo&	workItemThreadInfo =
																			*((SWorkItemThreadInfo*) userData);

											// Run forever
											while (true) {
												// Setup
												SWorkItemInfo&				workItemInfo =
																					*workItemThreadInfo.mWorkItemInfo;
												CWorkItemQueueInternals&	workItemQueueInternals =
																					workItemInfo
																							.mOwningWorkItemQueueInternals;

												// Perform
												workItemInfo.perform();

												// Note completed
												workItemInfo.transitionTo(CWorkItem::kStateCompleted);
												workItemThreadInfo.mWorkItemInfo = nil;

												// Remove work item info from active
												workItemQueueInternals.mWorkItemInfosLock.lock();
												workItemQueueInternals.mActiveWorkItemInfos -= workItemInfo;
												workItemQueueInternals.mWorkItemInfosLock.unlock();

												// Move this thread to idle
												mWorkItemThreadInfosLock.lock();
												mActiveWorkItemThreadInfos.move(workItemThreadInfo,
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

				TIArray<SWorkItemInfo>					mActiveWorkItemInfos;
				TIArray<SWorkItemInfo>					mIdleWorkItemInfos;
				CLock									mWorkItemInfosLock;

		static	TIArray<SWorkItemThreadInfo>			mActiveWorkItemThreadInfos;
		static	TIArray<SWorkItemThreadInfo>			mIdleWorkItemThreadInfos;
		static	CLock									mWorkItemThreadInfosLock;
};

OR<CWorkItemQueueInternals>		CWorkItemQueueInternals::mMainWorkItemQueueInternals;
TIArray<SWorkItemThreadInfo>	CWorkItemQueueInternals::mActiveWorkItemThreadInfos;
TIArray<SWorkItemThreadInfo>	CWorkItemQueueInternals::mIdleWorkItemThreadInfos;
CLock							CWorkItemQueueInternals::mWorkItemThreadInfosLock;

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
OR<SWorkItemInfo> SWorkItemQueueReference::getNextWorkItemInfo()
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
		sMainWorkItemQueue = new CWorkItemQueue(CCoreServices::getTotalProcessorCoresCount() - 1);
		sCreatingMainWorkItemQueue = false;
	}

	return *sMainWorkItemQueue;
}

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CWorkItemQueue::CWorkItemQueue(UInt32 maximumConcurrentWorkItems)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if creating main work item queue
	if (sCreatingMainWorkItemQueue) {
		// Main work item queue
		mInternals = new CWorkItemQueueInternals(maximumConcurrentWorkItems);
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
void CWorkItemQueue::add(CWorkItem& workItem, CWorkItem::Priority priority)
//----------------------------------------------------------------------------------------------------------------------
{
	// Add
	mInternals->add(workItem, priority);

	// Process work items
	CWorkItemQueueInternals::processWorkItems();
}

//----------------------------------------------------------------------------------------------------------------------
CWorkItem& CWorkItemQueue::add(CProcWorkItem::Proc proc, void* userData, CWorkItem::Priority priority)
//----------------------------------------------------------------------------------------------------------------------
{
	// Add
	CWorkItem&	workItem = mInternals->add(proc, userData, priority);

	// Process work items
	CWorkItemQueueInternals::processWorkItems();

	return workItem;
}

//----------------------------------------------------------------------------------------------------------------------
void CWorkItemQueue::cancel(CWorkItem& workItem)
//----------------------------------------------------------------------------------------------------------------------
{
// TODO
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
