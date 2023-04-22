//----------------------------------------------------------------------------------------------------------------------
//	CTimer-Windows.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CTimer.h"

#include <Windows.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CTimer::Internals
class CTimer::Internals {
	public:
									Internals(CTimer& timer, UniversalTimeInterval interval, bool repeats,
											CTimer::Proc proc, void* userData) :
										mTimer(timer), mInterval(interval), mRepeats(repeats), mProc(proc),
												mUserData(userData),
												mTPTimer(::CreateThreadpoolTimer(timerProc, this, nullptr))
										{}
									~Internals()
										{ ::CloseThreadpoolTimer(mTPTimer); }

				void				resume()
										{
											// Check if is set
											if (!::IsThreadpoolTimerSet(mTPTimer)) {
												// Setup
												ULARGE_INTEGER	intervalULI;
												intervalULI.QuadPart =
														(ULONGLONG) -((LONGLONG) (mInterval * 10.0 * 1000.0 * 1000.0));

												FILETIME	filetime;
												filetime.dwHighDateTime = intervalULI.HighPart;
												filetime.dwLowDateTime = intervalULI.LowPart;

												// Set the timer
												::SetThreadpoolTimer(mTPTimer, &filetime,
														mRepeats ? (DWORD) (mInterval * 1000.0) : 0, 0);
											}
										}
				void				suspend()
										{
											// Check if is set
											if (::IsThreadpoolTimerSet(mTPTimer))
												// "Un-set"
												::SetThreadpoolTimer(mTPTimer, NULL, 0, 0);
										}

		static	VOID	CALLBACK	timerProc(PTP_CALLBACK_INSTANCE callbackInstance, PVOID userData, PTP_TIMER timer)
										{
											// Setup
											Internals&	internals = *((Internals*) userData);

											// Call proc
											internals.mProc(internals.mTimer, internals.mUserData);
										}

		CTimer&					mTimer;
		UniversalTimeInterval	mInterval;
		bool					mRepeats;
		CTimer::Proc			mProc;
		void*					mUserData;

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
	mInternals->resume();
}

//----------------------------------------------------------------------------------------------------------------------
void CTimer::suspend()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->suspend();
}
