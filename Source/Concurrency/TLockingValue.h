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
		void	set(bool value = true)
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
						mSemaphore.setInstance(new CSemaphore());

						// Check value
						while (**this != value)
							// Wait
							mSemaphore->waitFor();

						// Cleanup
						mSemaphore.setInstance();
					}

		bool	operator*() const
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
		T		add(T value)
					{
						// Lock
						mLock.lockForWriting();

						// Update value
						mValueInternal += value;
						T	newValue = mValueInternal;

						// Check if have semaphore
						if (mSemaphore.hasInstance())
							// Signal
							mSemaphore->signal();

						// Unlock
						mLock.unlockForWriting();

						return newValue;
					}
		T		subtract(T value)
					{
						// Lock
						mLock.lockForWriting();

						// Update value
						mValueInternal -= value;
						T	newValue = mValueInternal;

						// Check if have semaphore
						if (mSemaphore.hasInstance())
							// Signal
							mSemaphore->signal();

						// Unlock
						mLock.unlockForWriting();

						return newValue;
					}

		void	wait(T value = 0)
					{
						// Setup
						mSemaphore.setInstance(new CSemaphore());

						// Check value
						while (**this != value)
							// Wait
							mSemaphore->waitFor();

						// Cleanup
						mSemaphore.setInstance();
					}

		T		operator*() const
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

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TLockingValue

template <typename T> struct TLockingValue {
	// Methods
	public:
				// Lifecycle methods
				TLockingValue(const T& initialValue) : mValueInternal(initialValue) {}

				// Instance methods
		void	set(const T& value)
					{
						// Lock
						mLock.lockForWriting();

						// Update value
						mValueInternal = value;

						// Unlock
						mLock.unlockForWriting();
					}

		T		operator*() const
					{
						// Lock
						mLock.lockForReading();

						// Copy value
						T	value(mValueInternal);

						// Unlock
						mLock.unlockForReading();

						return value;
					}
		void	operator=(const T& value)
					{ set(value); }

	// Properties
	private:
		CReadPreferringLock	mLock;
		T					mValueInternal;
};
