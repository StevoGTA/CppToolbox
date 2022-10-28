//----------------------------------------------------------------------------------------------------------------------
//	TLockingValue.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "ConcurrencyPrimitives.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SLockingBoolean

struct SLockingBoolean {
	// Methods
	public:
				// Lifecycle methods
				SLockingBoolean(bool initialValue = false) : mValueInternal(initialValue) {}

				// Instance methods
		void	set(bool value)
					{
						// Lock
						mLock.lockForWriting();

						// Update value
						mValueInternal = value;

						// Check if have semaphore
						if (mSemaphore.hasInstance())
							// Signal
							mSemaphore->signal();

						// Unlock
						mLock.unlockForWriting();
					}

		void	wait(bool value = true)
					{
						// Setup
						mSemaphore = OI<CSemaphore>(new CSemaphore());

						// Check value
						while (**this != value)
							// Wait
							mSemaphore->waitFor();

						// Cleanup
						mSemaphore = OI<CSemaphore>();
					}

		bool	operator*()
					{
						// Setup
						bool value;

						// Lock
						mLock.lockForReading();

						// Copy value
						value = mValueInternal;

						// Unlock
						mLock.unlockForReading();

						return value;
					}
		void	operator=(bool value)
					{ set(value); }

	// Properties
	private:
		CReadPreferringLock	mLock;
		bool				mValueInternal;
		OI<CSemaphore>		mSemaphore;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TLockingNumeric

template <typename T> struct TLockingNumeric {
	// Methods
	public:
				// Lifecycle methods
				TLockingNumeric(T initialValue = 0) : mValueInternal(initialValue) {}

				// Instance methods
		void	set(T value)
					{
						// Lock
						mLock.lockForWriting();

						// Update value
						mValueInternal = value;

						// Check if have semaphore
						if (mSemaphore.hasInstance())
							// Signal
							mSemaphore->signal();

						// Unlock
						mLock.unlockForWriting();
					}
		void	add(T value)
					{
						// Lock
						mLock.lockForWriting();

						// Update value
						mValueInternal += value;

						// Check if have semaphore
						if (mSemaphore.hasInstance())
							// Signal
							mSemaphore->signal();

						// Unlock
						mLock.unlockForWriting();
					}
		void	subtract(T value)
					{
						// Lock
						mLock.lockForWriting();

						// Update value
						mValueInternal -= value;

						// Check if have semaphore
						if (mSemaphore.hasInstance())
							// Signal
							mSemaphore->signal();

						// Unlock
						mLock.unlockForWriting();
					}

		void	wait(T value = 0)
					{
						// Setup
						mSemaphore = OI<CSemaphore>(new CSemaphore());

						// Check value
						while (**this != value)
							// Wait
							mSemaphore->waitFor();

						// Cleanup
						mSemaphore = OI<CSemaphore>();
					}

		T		operator*()
					{
						// Setup
						T value;

						// Lock
						mLock.lockForReading();

						// Copy value
						value = mValueInternal;

						// Unlock
						mLock.unlockForReading();

						return value;
					}
		void	operator=(T value)
					{ set(value); }

	// Properties
	private:
		CReadPreferringLock	mLock;
		T					mValueInternal;
		OI<CSemaphore>		mSemaphore;
};
