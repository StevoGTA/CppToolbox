//----------------------------------------------------------------------------------------------------------------------
//	CWorkItem.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CWorkItem.h"

#include "PlatformDefinitions.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CWorkItemInternals

class CWorkItemInternals {
	public:
		CWorkItemInternals() : mState(CWorkItem::kStateWaiting) {}
 
		CWorkItem::State	mState;
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
	mInternals->mState = state;

	// Check state
	switch (mInternals->mState) {
		case kStateCompleted:	completed();	break;
		case kStateCancelled:	cancelled();	break;
		default:								break;
	}
}
