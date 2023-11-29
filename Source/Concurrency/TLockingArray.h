//----------------------------------------------------------------------------------------------------------------------
//	TLockingArray.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CArray.h"
#include "ConcurrencyPrimitives.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: TNLockingArray

template <typename T> class TNLockingArray : public TNArray<T> {
	// Methods
	public:
								// Lifecycle methods
								TNLockingArray() : TNArray<T>() {}
								TNLockingArray(const T& item) : TNArray<T>(item) {}
								TNLockingArray(const TArray<T>& other) : TNArray<T>(other) {}

								// CArray methods
		CArray::ItemCount		getCount() const
									{
										// Query under lock
										mLock.lockForReading();
										CArray::ItemCount	count = CArray::getCount();
										mLock.unlockForReading();

										return count;
									}

		TNLockingArray<T>&		add(const T& item)
									{
										// Add under lock
										mLock.lockForWriting();
										TNArray<T>::add(item);
										mLock.unlockForWriting();

										return *this;
									}
		TNLockingArray<T>&		addFrom(const TArray<T>& array)
									{
										// Add all under lock
										mLock.lockForWriting();
										TNArray<T>::addFrom(array);
										mLock.unlockForWriting();

										return *this;
									}

		bool					contains(const T& item) const
									{
										// Check under lock
										mLock.lockForReading();
										bool	contains = TNArray<T>::contains(item);
										mLock.unlockForReading();

										return contains;
									}

		T&						getAt(CArray::ItemIndex index) const
									{
										// Get under lock
										mLock.lockForReading();
										T&	item = TNArray<T>::getAt(index);
										mLock.unlockForReading();

										return item;
									}
		T&						getFirst() const
									{
										// Get under lock
										mLock.lockForReading();
										T&	item = TNArray<T>::getFirst();
										mLock.unlockForReading();

										return item;
									}
		T&						getLast() const
									{
										// Get under lock
										mLock.lockForReading();
										T&	item = TNArray<T>::getLast();
										mLock.unlockForReading();

										return item;
									}
		OV<CArray::ItemIndex>	getIndexOf(const T& item) const
									{
										// Query under lock
										mLock.lockForReading();
										OV<CArray::ItemIndex>	index = TNArray<T>::getIndexOf(item);
										mLock.unlockForReading();

										return index;
									}

		TNLockingArray<T>&		insertAtIndex(const T& item, CArray::ItemIndex itemIndex)
									{
										// Insert under lock
										mLock.lockForWriting();
										TNArray<T>::insertAtIndex(new T(item), itemIndex);
										mLock.unlockForWriting();

										return *this;
									}

		TNLockingArray<T>&		remove(const T& item)
									{
										// Remove under lock
										mLock.lockForWriting();
										TNArray<T>::remove(item);
										mLock.unlockForWriting();

										return *this;
									}
		TNLockingArray<T>&		removeFrom(const TArray<T>& array)
									{
										// Remove under lock
										mLock.lockForWriting();
										TNArray<T>::removeFrom(array);
										mLock.unlockForWriting();

										return *this;
									}
		TNLockingArray<T>&		removeAtIndex(CArray::ItemIndex itemIndex)
									{
										// Remove under lock
										mLock.lockForWriting();
										TNArray<T>::removeAtIndex(itemIndex);
										mLock.unlockForWriting();

										return *this;
									}
		TNLockingArray<T>&		removeAll()
									{
										// Remove under lock
										mLock.lockForWriting();
										TNArray<T>::removeAll();
										mLock.unlockForWriting();

										return *this;
									}

		T						popFirst()
									{
										// Perform under lock
										mLock.lockForWriting();
										T	item = TNArray<T>::popFirst();
										mLock.unlockForWriting();

										return item;
									}

		TNLockingArray<T>&		apply(void (proc)(T& item, void* userData), void* userData = nil)
									{
										// Perform under lock
										mLock.lockForReading();
										TNArray<T>::apply((TArray<T>::ApplyProc) proc, userData);
										mLock.unlockForReading();

										return *this;
									}

		TNLockingArray<T>&		sort(bool (compareProc)(const T& item1, const T& item2, void* userData),
										void* userData = nil)
									{
										// Perform under lock
										mLock.lockForWriting();
										TNArray<T>::sort((CArray::CompareProc) compareProc, userData);
										mLock.unlockForWriting();

										return *this;
									}

								// Instance methods
		T&						operator[] (CArray::ItemIndex index) const
									{ return getAt(index); }
		TNLockingArray<T>&		operator+=(const T& item)
									{ return add(item); }
		TNLockingArray<T>&		operator+=(const TArray<T>& other)
									{ return addFrom(other); }
		TNLockingArray<T>&		operator+=(const TNLockingArray<T>& other)
									{ return addFrom(other); }
		TNLockingArray<T>&		operator-=(const T& item)
									{ return remove(item); }
		TNLockingArray<T>&		operator-=(const TArray<T>& other)
									{ return removeFrom(other); }
		TNLockingArray<T>&		operator-=(const TNLockingArray<T>& other)
									{ return removeFrom(other); }

	// Properties
	private:
		CReadPreferringLock	mLock;
};
