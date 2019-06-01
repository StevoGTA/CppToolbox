//----------------------------------------------------------------------------------------------------------------------
//	COperationQueue.cpp			©2012 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "COperationQueue.h"

#include "CArray.h"
#include "CCoreServices.h"
#include "CLock.h"
#include "CThread.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SOperationInfo

struct SOperationInfo {
	// Lifecycle methods
	SOperationInfo(COperationQueueInternals& owningOperationQueueInternals, COperation& operation,
			EOperationPriority priority) :
		mOwningOperationQueueInternals(owningOperationQueueInternals), mOperation(operation), mPriority(priority),
				mIsPaused(false)
		{}

	// Properties
	COperationQueueInternals&	mOwningOperationQueueInternals;
	COperation&					mOperation;
	EOperationPriority			mPriority;
	bool						mIsPaused;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - SOperationThreadInfo
struct SOperationThreadInfo {
	// Lifecycle methods
	SOperationThreadInfo(SOperationInfo* initialOperationInfo, CThreadProc threadProc, const CString& name) :
		mOperationInfo(initialOperationInfo), mThread(threadProc, this, name)
		{}

	// Properties
	CSemaphore		mSemaphore;
	CThread			mThread;
	SOperationInfo*	mOperationInfo;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - COperationInternals

class COperationInternals {
	public:
		COperationInternals(bool disposeOnCompletion) :
			mDisposeOnCompletion(disposeOnCompletion), mIsActive(false), mIsCancelled(false)
			{}
		~COperationInternals() {}

		bool		mDisposeOnCompletion;
		bool		mIsActive;
		bool		mIsCancelled;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - COperationQueueInternals

class COperationQueueInternals {
	public:
		COperationQueueInternals() {}
		~COperationQueueInternals() {}

		TArray<SOperationInfo*>	mOperationInfos;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CProcOperation

class CProcOperation : public COperation {
	public:
				CProcOperation(CProcOperationProc proc, void* userData) :
						COperation(true), mProc(proc), mUserData(userData)
						{}
				~CProcOperation() {}

		void	perform()
					{ mProc(mUserData, *this); }

		CProcOperationProc	mProc;
		void*				mUserData;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local data

static	TArray<SOperationInfo*>	sOperationInfos;
static	CLock					sOperationInfosLock;

static	TArray<SOperationThreadInfo*>	sActiveOperationThreadInfos;
static	TArray<SOperationThreadInfo*>	sIdleOperationThreadInfos;
static	CLock							sOperationThreadInfosLock;
static	CArrayItemCount					sMaxOperationThreads = CCoreServices::getTotalProcessorCoresCount() - 1;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc declarations

static	ECompareResult	sOperationInfoCompareProc(SOperationInfo* const operationInfo1,
								SOperationInfo* const operationInfo2, void* userData);
static	void			sProcessOperations();
static	void			sThreadProc(const CThread& thread, void* userData);
static	void			sOperationInfoCleanup(SOperationInfo* operationInfo);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - COperation

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
COperation::COperation(bool disposeOnCompletion)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new COperationInternals(disposeOnCompletion);
}

//----------------------------------------------------------------------------------------------------------------------
COperation::~COperation()
//----------------------------------------------------------------------------------------------------------------------
{
	DisposeOf(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void COperation::cancel()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mIsCancelled = true;
}

//----------------------------------------------------------------------------------------------------------------------
bool COperation::isCancelled() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mIsCancelled;
}

//----------------------------------------------------------------------------------------------------------------------
void COperation::setActive(bool isActive)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mIsActive = isActive;
}

//----------------------------------------------------------------------------------------------------------------------
bool COperation::isActive() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mIsActive;
}

//----------------------------------------------------------------------------------------------------------------------
void COperation::finished() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if need dispose
	if (mInternals->mDisposeOnCompletion) {
		// Dispose
		COperation*	THIS = (COperation*) this;
		DisposeOf(THIS);
	}
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - COperationQueue

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
COperationQueue::COperationQueue()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new COperationQueueInternals();
}

//----------------------------------------------------------------------------------------------------------------------
COperationQueue::~COperationQueue()
//----------------------------------------------------------------------------------------------------------------------
{
	DisposeOf(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void COperationQueue::add(COperation& operation, EOperationPriority priority)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SOperationInfo*	operationInfo = new SOperationInfo(*mInternals, operation, priority);

	// Add
	sOperationInfosLock.lock();
	mInternals->mOperationInfos += operationInfo;
	sOperationInfos += operationInfo;
	sOperationInfosLock.unlock();

	// Process operations
	sProcessOperations();
}

//----------------------------------------------------------------------------------------------------------------------
COperation& COperationQueue::add(CProcOperationProc proc, void* userData, EOperationPriority priority)
//----------------------------------------------------------------------------------------------------------------------
{
	// Create operation
	CProcOperation*	procOperation = new CProcOperation(proc, userData);

	// Add
	add(*procOperation, priority);

	return *procOperation;
}

//----------------------------------------------------------------------------------------------------------------------
void COperationQueue::pause()
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all operation infos
	sOperationInfosLock.lock();
	for (CArrayItemIndex i = 0; i < mInternals->mOperationInfos.getCount(); i++)
		// Mark as paused
		mInternals->mOperationInfos[i]->mIsPaused = true;
	sOperationInfosLock.unlock();
}

//----------------------------------------------------------------------------------------------------------------------
void COperationQueue::resume()
//----------------------------------------------------------------------------------------------------------------------
{
	// Iterate all operation infos
	sOperationInfosLock.lock();
	for (CArrayItemIndex i = 0; i < mInternals->mOperationInfos.getCount(); i++)
		// Mark as not paused
		mInternals->mOperationInfos[i]->mIsPaused = false;
	sOperationInfosLock.unlock();

	// Process operations
	sProcessOperations();
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: Local proc definitions

//----------------------------------------------------------------------------------------------------------------------
ECompareResult sOperationInfoCompareProc(SOperationInfo* const operationInfo1, SOperationInfo* const operationInfo2,
		void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Sort in this order:
	//	Active
	//	Cancelled
	//	Priority
	if (operationInfo1->mOperation.isActive())
		// Operation 1 is acive
		return kCompareResultBefore;
	else if (operationInfo2->mOperation.isActive())
		// Operation 2 is active
		return kCompareResultAfter;
	else if (operationInfo1->mOperation.isCancelled())
		// Operation 1 is cancelled
		return kCompareResultBefore;
	else if (operationInfo2->mOperation.isCancelled())
		// Operation 2 is cancelled
		return kCompareResultAfter;
	else if (operationInfo1->mPriority < operationInfo2->mPriority)
		// Operation 1 has a higher priority
		return kCompareResultBefore;
	else if (operationInfo2->mPriority < operationInfo1->mPriority)
		// Operation 2 has a higher priority
		return kCompareResultAfter;
	else
		// They be equivalent
		return kCompareResultEquivalent;
}

//----------------------------------------------------------------------------------------------------------------------
void sProcessOperations()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if can do another operation
	sOperationThreadInfosLock.lock();
	if (sActiveOperationThreadInfos.getCount() < sMaxOperationThreads) {
		// Lock
		sOperationInfosLock.lock();

		// Sort
		sOperationInfos.sort(sOperationInfoCompareProc);

		// Iterate until we find an operation to perform
		for (CArrayItemIndex i = 0; i < sOperationInfos.getCount();) {
			// Check next operation
			SOperationInfo*	operationInfo = sOperationInfos[i];

			// Check if active
			if (operationInfo->mOperation.isActive()) {
				// Skip
				i++;
				continue;
			}

			if (operationInfo->mOperation.isCancelled())
				// Cancelled
				sOperationInfoCleanup(operationInfo);
			else if (operationInfo->mIsPaused)
				// Skip
				i++;
			else {
				// Perform this operation
				operationInfo->mOperation.setActive(true);
				if (sIdleOperationThreadInfos.getCount() > 0) {
					// Resume an idle thread
					SOperationThreadInfo*	operationThreadInfo = sIdleOperationThreadInfos[0];
					sIdleOperationThreadInfos.removeAtIndex(0);

					operationThreadInfo->mOperationInfo = operationInfo;
					operationThreadInfo->mSemaphore.signal();

					sActiveOperationThreadInfos += operationThreadInfo;
				} else
					// Create new active thread
					sActiveOperationThreadInfos +=
							new SOperationThreadInfo(operationInfo, sThreadProc,
									CString("COperationQueue Thread #") +
											CString(sActiveOperationThreadInfos.getCount() + 1));

				break;
			}
		}

		// Unlock
		sOperationInfosLock.unlock();
	}
	sOperationThreadInfosLock.unlock();
}

//----------------------------------------------------------------------------------------------------------------------
void sThreadProc(const CThread& thread, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SOperationThreadInfo*	operationThreadInfo = (SOperationThreadInfo*) userData;

	// Run forever
	while (true) {
		// Run operation
		SOperationInfo*	operationInfo = operationThreadInfo->mOperationInfo;
		operationInfo->mOperation.perform();
		operationInfo->mOperation.setActive(false);
		operationInfo->mOperation.finished();

		// Update
		sOperationInfoCleanup(operationInfo);
		operationThreadInfo->mOperationInfo = nil;

		sOperationThreadInfosLock.lock();
		sActiveOperationThreadInfos -= operationThreadInfo;
		sIdleOperationThreadInfos += operationThreadInfo;
		sOperationThreadInfosLock.unlock();

		// Next
		sProcessOperations();

		// Check if waiting on operation info
		if (operationThreadInfo->mOperationInfo == nil)
			// Wait
			operationThreadInfo->mSemaphore.waitFor();
	}
}

//----------------------------------------------------------------------------------------------------------------------
void sOperationInfoCleanup(SOperationInfo* operationInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	// Remove from arrays
	sOperationInfosLock.lock();
	sOperationInfos -= operationInfo;
	operationInfo->mOwningOperationQueueInternals.mOperationInfos -= operationInfo;
	sOperationInfosLock.unlock();
}
