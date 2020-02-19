//----------------------------------------------------------------------------------------------------------------------
//	CLock.h			©2018 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CTimeInfo.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CLock

class CLockInternals;
class CLock {
	// Methods
	public:
				// Lifecycle methods
				CLock();
				~CLock();

				// Instance methods
		bool	tryLock() const;
		void	lock() const;
		void	unlock() const;

	// Properties
	public:
		CLockInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CReadPreferringLock

class CReadPreferringLockInternals;
class CReadPreferringLock {
	// Methods
	public:
				// Lifecycle methods
				CReadPreferringLock();
				~CReadPreferringLock();

				// Instance methods
		void	lockForReading() const;
		void	unlockForReading() const;
		void	lockForWriting() const;
		void	unlockForWriting() const;

	// Properties
	public:
		CReadPreferringLockInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSemaphore

class CSemaphoreInternals;
class CSemaphore {
	// Methods
	public:
				// Lifecycle methods
				CSemaphore();
				~CSemaphore();

				// Instance methods
		void	signal() const;
		void	waitFor() const;
		void	timedWaitFor(UniversalTimeInterval maxWaitTimeInterval) const;

	// Properties
	public:
		CSemaphoreInternals*	mInternals;
};
