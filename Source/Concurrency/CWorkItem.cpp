//----------------------------------------------------------------------------------------------------------------------
//	CWorkItem.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CWorkItem.h"

#include "ConcurrencyPrimitives.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CWorkItemInternals

class CWorkItemInternals {
	public:
		CWorkItemInternals() : mState(CWorkItem::kStateWaiting) {}
 
		CWorkItem::State	mState;
		CLock				mStateLock;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CWorkItem

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CWorkItem::CWorkItem()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CWorkItemInternals();
}

//----------------------------------------------------------------------------------------------------------------------
CWorkItem::~CWorkItem()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CWorkItem::State CWorkItem::getState() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mState;
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
			case kStateCompleted:	completed();	break;
			case kStateCancelled:	cancelled();	break;
			default:								break;
		}
}
