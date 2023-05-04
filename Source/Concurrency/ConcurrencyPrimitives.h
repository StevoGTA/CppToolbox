//----------------------------------------------------------------------------------------------------------------------
//	ConcurrencyPrimitives.h			©2018 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "TimeAndDate.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CLock

class CLock {
	// Classes
	private:
		class Internals;

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
	private:
		Internals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CReadPreferringLock

class CReadPreferringLock {
	// Classes
	private:
		class Internals;

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
	private:
		Internals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSemaphore

class CSemaphore {
	// Classes
	private:
		class Internals;

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
	private:
		Internals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CSharedResource

class CSharedResource {
	// Classes
	private:
		class Internals;

	// Methods
	public:
				// Lifecycle methods
				CSharedResource(UInt32 count);
				~CSharedResource();

				// Instance methods
		void	consume() const;
		void	release() const;

	// Properties
	private:
		Internals*	mInternals;
};
