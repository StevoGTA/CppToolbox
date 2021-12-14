//----------------------------------------------------------------------------------------------------------------------
//	TLockingValue.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "ConcurrencyPrimitives.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: TLockingValue

template <typename T> class TLockingValue : public CArray {
	// Methods
	public:
					// Lifecycle methods
					TLockingValue(T initialValue) : mValueInternal(initialValue) {}

					// Instance methods
		void		set(T value)
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

		void		wait(T value)
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
