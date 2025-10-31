//----------------------------------------------------------------------------------------------------------------------
//	CWorkItem.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CWorkItem.h"

#include "ConcurrencyPrimitives.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CWorkItem::Internals

class CWorkItem::Internals {
	public:
		Internals(const CString& id, const OV<CString>& reference, CWorkItem::CompletedProc completedProc,
				CWorkItem::CancelledProc cancelledProc, void* userData) :
			mID(id), mReference(reference), mCompletedProc(completedProc), mCancelledProc(cancelledProc),
					mUserData(userData),
					mIsCancelled(false), mState(CWorkItem::kStateWaiting)
			{}

 		CString						mID;
 		OV<CString>					mReference;
 		CWorkItem::CompletedProc	mCompletedProc;
 		CWorkItem::CancelledProc	mCancelledProc;
 		void*						mUserData;

		bool						mIsCancelled;
		CWorkItem::State			mState;
		CLock						mStateLock;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CWorkItem

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CWorkItem::CWorkItem(const CString& id, const OV<CString>& reference, CompletedProc completedProc,
		CancelledProc cancelledProc, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(id, reference, completedProc, cancelledProc, userData);
}

//----------------------------------------------------------------------------------------------------------------------
CWorkItem::~CWorkItem()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
const CString& CWorkItem::getID() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mID;
}

//----------------------------------------------------------------------------------------------------------------------
const OV<CString>& CWorkItem::getReference() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mReference;
}

//----------------------------------------------------------------------------------------------------------------------
CWorkItem::State CWorkItem::getState() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mState;
}

//----------------------------------------------------------------------------------------------------------------------
bool CWorkItem::isWaiting() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Determine value
	mInternals->mStateLock.lock();
	bool	isWaiting = !mInternals->mIsCancelled && (mInternals->mState == kStateWaiting);
	mInternals->mStateLock.unlock();

	return isWaiting;
}

//----------------------------------------------------------------------------------------------------------------------
bool CWorkItem::isActive() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Determine value
	mInternals->mStateLock.lock();
	bool	isWaiting = !mInternals->mIsCancelled && (mInternals->mState == kStateActive);
	mInternals->mStateLock.unlock();

	return isWaiting;
}

//----------------------------------------------------------------------------------------------------------------------
bool CWorkItem::isCompleted() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Determine value
	mInternals->mStateLock.lock();
	bool	isWaiting = !mInternals->mIsCancelled && (mInternals->mState == kStateCompleted);
	mInternals->mStateLock.unlock();

	return isWaiting;
}

//----------------------------------------------------------------------------------------------------------------------
bool CWorkItem::isCancelled() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mIsCancelled;
}

//----------------------------------------------------------------------------------------------------------------------
void CWorkItem::transitionTo(State state)
//----------------------------------------------------------------------------------------------------------------------
{
	// Update state
	bool	didChangeState;
	mInternals->mStateLock.lock();
	if ((mInternals->mState == kStateWaiting) || (mInternals->mState == kStateActive)) {
		// Transition to new state
		mInternals->mState = state;
		didChangeState = true;
	} else
		// Ignore - already in final state
		didChangeState = false;
	mInternals->mStateLock.unlock();

	// Check if did change state
	if (didChangeState)
		// Check state
		switch (mInternals->mState) {
			case kStateCompleted:
				// Completed
				if (mInternals->mCompletedProc != nil)
					// Call proc
					mInternals->mCompletedProc(*this, mInternals->mUserData);
				else
					// Call subclass
					completed();
				break;

			default:
				break;
		}
}

//----------------------------------------------------------------------------------------------------------------------
void CWorkItem::cancel()
//----------------------------------------------------------------------------------------------------------------------
{
	// Update
	mInternals->mIsCancelled = true;
}
