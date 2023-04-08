//----------------------------------------------------------------------------------------------------------------------
//	CTimer-Apple.mm			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#import "CTimer.h"

#import <Foundation/Foundation.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CTimer::Internals
class CTimer::Internals {
	public:
		Internals(CTimer& timer, UniversalTimeInterval interval, bool repeats, CTimer::Proc proc,
				void* userData)
			{
				// Setup
				dispatch_source_t	dispatchSourceTimer =
											dispatch_source_create(DISPATCH_SOURCE_TYPE_TIMER, 0, 0,
													dispatch_get_main_queue());
				dispatch_source_set_timer(dispatchSourceTimer, DISPATCH_TIME_NOW, interval * NSEC_PER_SEC, 0);
				dispatch_source_set_event_handler(dispatchSourceTimer, ^{
					// Call proc
					proc(timer, userData);

					// Check repeats
					if (!repeats) {
						// Don
						dispatch_source_cancel((__bridge dispatch_source_t) mDispatchSourceTimer);
						mDispatchSourceTimer = nil;
						mIsResumed = false;
					}
				});

				mDispatchSourceTimer = (void*) ::CFBridgingRetain(dispatchSourceTimer);
				mIsResumed = false;
			}
		~Internals()
			{
				// Cleanup
				if (mDispatchSourceTimer != nil) {
					// Cancel
					dispatch_source_cancel((__bridge dispatch_source_t) mDispatchSourceTimer);
					mDispatchSourceTimer = nil;
				}
			}

		void*	mDispatchSourceTimer;
		bool	mIsResumed;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CTimer

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CTimer::CTimer(UniversalTimeInterval interval, Proc proc, void* userData, bool repeats, Options options)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals = new Internals(*this, interval, repeats, proc, userData);

	// Check options
	if (options & kOptionsAutoResume)
		// Resume
		resume();
}

//----------------------------------------------------------------------------------------------------------------------
CTimer::~CTimer()
//----------------------------------------------------------------------------------------------------------------------
{
	// Cleanup
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
void CTimer::resume()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if have internal timer and not already resumed
	if ((mInternals->mDispatchSourceTimer != nil) && !mInternals->mIsResumed) {
		// Resume
		dispatch_resume((__bridge dispatch_source_t) mInternals->mDispatchSourceTimer);
		mInternals->mIsResumed = true;
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CTimer::suspend()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if have internal timer and resumed
	if ((mInternals->mDispatchSourceTimer != nil) && mInternals->mIsResumed) {
		// Suspend
		dispatch_suspend((__bridge dispatch_source_t) mInternals->mDispatchSourceTimer);
		mInternals->mIsResumed = false;
	}
}
