//----------------------------------------------------------------------------------------------------------------------
//	TLockingSet.h			©2026 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CSet.h"
#include "ConcurrencyPrimitives.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: TLockingNumberSet

template <typename T> class TLockingNumberSet : public TNumberSet<T> {
	// Methods
	public:
								// Lifecycle methods
								TLockingNumberSet() : TNumberSet<T>() {}

								// Instance methods
		bool					contains(T value) const
									{
										// Check under lock
										mLock.lockForReading();
										bool	contains = TNumberSet<T>::contains(value);
										mLock.unlockForReading();

										return contains;
									}

		TLockingNumberSet<T>&	insert(T value)
									{
										// Insert under lock
										mLock.lockForWriting();
										TNumberSet<T>::insert(value);
										mLock.unlockForWriting();

										return *this;
									}
		TLockingNumberSet<T>&	remove(T value)
									{
										// Remove under lock
										mLock.lockForWriting();
										TNumberSet<T>::remove(value);
										mLock.unlockForWriting();

										return *this;
									}

		TLockingNumberSet<T>&	operator+=(T value)
									{ return insert(value); }
		TLockingNumberSet<T>&	operator-=(T value)
									{ return remove(value); }

	// Properties
	private:
		CReadPreferringLock	mLock;
};
