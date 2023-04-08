//----------------------------------------------------------------------------------------------------------------------
//	CTimer.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "TimeAndDate.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CTimer

class CTimer {
	// Procs
	public:
		// On macOS, this proc will be called on the Main thread.
		// On Windows, this proc will be called on a background thread.
		typedef	void	(*Proc)(CTimer& timer, void* userData);

	// Options:
	public:
		enum Options {
			kOptionsNone		= 0,
			kOptionsAutoResume	= 1 << 0,
		};

	// Methods
	public:
				// Lifecycle methods
				CTimer(UniversalTimeInterval interval, Proc proc, void* userData, bool repeats = false,
						Options options = kOptionsNone);
				~CTimer();

				// Instance methods
		void	resume();
		void	suspend();

	// Properties
	private:
		class Internals;
		Internals*	mInternals;
};
