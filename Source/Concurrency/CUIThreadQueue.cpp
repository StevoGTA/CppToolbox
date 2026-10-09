//----------------------------------------------------------------------------------------------------------------------
//	CUIThreadQueue.cpp			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CUIThreadQueue.h"

#include "TWrappers.h"

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS)
	#include <dispatch/dispatch.h>
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: CUIThreadQueue::Internals

class CUIThreadQueue::Internals {
	public:
#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS)
		Internals() : mIsActive(new bool(true)) {}
#elif defined(TARGET_OS_WINDOWS)
		Internals(const DispatcherQueue& dispatcherQueue) :
			mIsActive(new bool(true)), mDispatcherQueue(dispatcherQueue)
			{}
#endif
		~Internals()
			{ *mIsActive = false; }

		I<bool>				mIsActive;

#if defined(TARGET_OS_WINDOWS)
		DispatcherQueue		mDispatcherQueue;
#endif
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CUIThreadQueue

// MARK: Lifecycle methods

#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS)
//----------------------------------------------------------------------------------------------------------------------
CUIThreadQueue::CUIThreadQueue()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals();
}
#elif defined(TARGET_OS_WINDOWS)
//----------------------------------------------------------------------------------------------------------------------
CUIThreadQueue::CUIThreadQueue(const DispatcherQueue& dispatcherQueue)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(dispatcherQueue);
}
#endif

//----------------------------------------------------------------------------------------------------------------------
CUIThreadQueue::~CUIThreadQueue()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CUIThreadQueue::add(Proc proc, void* userData, bool isRequired)
//----------------------------------------------------------------------------------------------------------------------
{
	add([proc, userData](){ proc(userData); }, isRequired);
}

//----------------------------------------------------------------------------------------------------------------------
void CUIThreadQueue::add(const std::function<void()>& proc, bool isRequired)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	I<bool>	isActive = mInternals->mIsActive;

	// Queue
#if defined(TARGET_OS_IOS) || defined(TARGET_OS_MACOS) || defined(TARGET_OS_TVOS)
	std::function<void()>	proc_ = proc;
	dispatch_async(dispatch_get_main_queue(), ^{
		// Check if still active or required
		if (*isActive || isRequired)
			// Call proc
			proc_();
	});
#elif defined(TARGET_OS_WINDOWS)
	mInternals->mDispatcherQueue.TryEnqueue([isActive, proc, isRequired]() {
		// Check if still active or required
		if (*isActive || isRequired)
			// Call proc
			proc();
	});
#endif
}
