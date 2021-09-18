//----------------------------------------------------------------------------------------------------------------------
//	CTimer.mm			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#import "CTimer.h"

#import <Foundation/Foundation.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CTimerInternals
class CTimerInternals {
	public:
		CTimerInternals(CTimer& timer, UniversalTimeInterval interval, bool repeats, CTimer::Proc proc,
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
						// Done
						dispatch_source_cancel((__bridge dispatch_source_t) mDispatchSourceTimer);
						mDispatchSourceTimer = nil;
					}
				});

				mDispatchSourceTimer = (void*) ::CFBridgingRetain(dispatchSourceTimer);
			}
		~CTimerInternals()
			{
				// Cleanup
				if (mDispatchSourceTimer != nil) {
					// Cancel
					dispatch_source_cancel((__bridge dispatch_source_t) mDispatchSourceTimer);
					mDispatchSourceTimer = nil;
				}
			}

		void*	mDispatchSourceTimer;
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
	mInternals = new CTimerInternals(*this, interval, repeats, proc, userData);

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
	// Check if have internal timer
	if (mInternals->mDispatchSourceTimer != nil) {
		// Suspend
		dispatch_resume((__bridge dispatch_source_t) mInternals->mDispatchSourceTimer);
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CTimer::suspend()
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if have internal timer
	if (mInternals->mDispatchSourceTimer != nil) {
		// Suspend
		dispatch_suspend((__bridge dispatch_source_t) mInternals->mDispatchSourceTimer);
	}
}
