//----------------------------------------------------------------------------------------------------------------------
//	CTimer-Windows.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CTimer.h"

#undef Delete
#include <Windows.h>
#define Delete(x)	{ delete x; x = nil; }

//----------------------------------------------------------------------------------------------------------------------
// MARK: CTimer::Internals
class CTimer::Internals {
	public:
									Internals(CTimer& timer, UniversalTimeInterval interval, bool repeats,
											CTimer::Proc proc, void* userData, CTimer::Options options) :
										mTimer(timer), mInterval(interval), mRepeats(repeats), mProc(proc),
												mUserData(userData), mOptions(options),
												mTPTimer(::CreateThreadpoolTimer(timerProc, this, nullptr))
										{}
									~Internals()
										{
											// Cancel and wait for any pending callbacks
											::SetThreadpoolTimer(mTPTimer, NULL, 0, 0);
											::WaitForThreadpoolTimerCallbacks(mTPTimer, TRUE);

											// Close
											::CloseThreadpoolTimer(mTPTimer);
										}

				void				resume()
										{
											// Check if is set
											if (!::IsThreadpoolTimerSet(mTPTimer))
												// Set the timer
												set();
										}
				void				suspend()
										{
											// Check if is set
											if (::IsThreadpoolTimerSet(mTPTimer))
												// "Un-set"
												::SetThreadpoolTimer(mTPTimer, NULL, 0, 0);
										}
				void				set()
										{
											// Setup
											LONGLONG		interval100ns =
																	(LONGLONG) (mInterval * 10.0 * 1000.0 * 1000.0);
											ULARGE_INTEGER	dueTimeULI;
											if (mOptions & CTimer::kOptionsWallClock) {
												// Wall clock
												FILETIME	currentFiletime;
												::GetSystemTimeAsFileTime(&currentFiletime);
												dueTimeULI.HighPart = currentFiletime.dwHighDateTime;
												dueTimeULI.LowPart = currentFiletime.dwLowDateTime;
												dueTimeULI.QuadPart += (ULONGLONG) interval100ns;
											} else
												// Relative
												dueTimeULI.QuadPart = (ULONGLONG) -interval100ns;

											FILETIME	filetime;
											filetime.dwHighDateTime = dueTimeULI.HighPart;
											filetime.dwLowDateTime = dueTimeULI.LowPart;

											// Set the timer
											::SetThreadpoolTimer(mTPTimer, &filetime,
													(mRepeats && !(mOptions & CTimer::kOptionsWallClock)) ?
															(DWORD) (mInterval * 1000.0) : 0,
													0);
										}

		static	VOID	CALLBACK	timerProc(PTP_CALLBACK_INSTANCE callbackInstance, PVOID userData, PTP_TIMER timer)
										{
											// Setup
											Internals&	internals = *((Internals*) userData);

											// Call proc
											internals.mProc(internals.mTimer, internals.mUserData);

											// Check if wall clock repeats
											if (internals.mRepeats && (internals.mOptions & CTimer::kOptionsWallClock))
												// Set the next
												internals.set();
										}

		CTimer&					mTimer;
		UniversalTimeInterval	mInterval;
		bool					mRepeats;
		CTimer::Proc			mProc;
		void*					mUserData;
		CTimer::Options			mOptions;

		TP_TIMER*				mTPTimer;
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
	mInternals = new Internals(*this, interval, repeats, proc, userData, options);

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
	mInternals->resume();
}

//----------------------------------------------------------------------------------------------------------------------
void CTimer::suspend()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->suspend();
}
