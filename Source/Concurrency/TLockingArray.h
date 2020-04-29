//----------------------------------------------------------------------------------------------------------------------
//	TLockingArray.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CArray.h"
#include "CLock.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: TLockingArray

template <typename T> class TLockingArray : public CArray {
	// Methods
	public:
											// Lifecycle methods
											TLockingArray(CArrayItemCount initialCapacity = 0) :
												CArray(initialCapacity, (CArrayItemCopyProc) copy, dispose)
												{}
											TLockingArray(const T& item) : CArray(0, (CArrayItemCopyProc) copy, dispose)
												{ CArray::add(new T(item), false); }
											TLockingArray(const TArray<T>& array) : CArray(array) {}

											// CArray methods
						CArrayItemCount		getCount() const
												{
													// Query under lock
													mLock.lockForReading();
													CArrayItemCount	count = CArray::getCount();
													mLock.unlockForReading();

													return count;
												}

						TLockingArray<T>&	add(const T& item)
												{
													// Add under lock
													mLock.lockForWriting();
													CArray::add(new T(item));
													mLock.unlockForWriting();

													return *this;
												}
						TLockingArray<T>&	addFrom(const TArray<T>& array)
												{
													// Add all under lock
													mLock.lockForWriting();
													for (CArrayItemIndex i = 0; i < array.getCount(); i++)
														// Add
														CArray::add(new T(array[i]));
													mLock.unlockForWriting();

													return *this;
												}

						bool				contains(const T& item) const
												{
													// Check under lock
													mLock.lockForReading();
													bool	contains = getIndexOf(item).hasValue();
													mLock.unlockForReading();

													return contains;

												}

				const	T&					getAt(CArrayItemIndex index) const
												{
													// Get under lock
													mLock.lockForReading();
													const	T&	item = *((T*) getItemAt(index));
													mLock.unlockForReading();

													return item;

												}
				const	T&					getFirst() const
												{
													// Get under lock
													mLock.lockForReading();
													const	T&	item = *((T*) CArray::getFirst());
													mLock.unlockForReading();

													return item;
												}
				const	T&					getLast() const
												{
													// Get under lock
													mLock.lockForReading();
													const	T&	item = *((T*) CArray::getLast());
													mLock.unlockForReading();

													return item;
												}
						OV<CArrayItemIndex>	getIndexOf(const T& item) const
												{
													// Query under lock
													mLock.lockForReading();
													OV<CArrayItemIndex>	index;
													for (CArrayItemIndex i = 0; i < getCount(); i++) {
														// Check if same
														if (item == getAt(i))
															// Match
															index = i;
															break;
													}
													mLock.unlockForReading();

													return index;
												}

						TLockingArray<T>&	insertAtIndex(const T& item, CArrayItemIndex itemIndex)
												{
													// Insert under lock
													mLock.lockForWriting();
													CArray::insertAtIndex(new T(item), itemIndex);
													mLock.unlockForWriting();

													return *this;
												}

						TLockingArray<T>&	remove(const T& item)
												{
													// Remove under lock
													mLock.lockForWriting();
													OV<CArrayItemIndex>	index = getIndexOf(item);
													if (index.hasValue())
														// Remove
														removeAtIndex(index);
													mLock.unlockForWriting();

													return *this;
												}
						TLockingArray<T>&	removeFrom(const TArray<T>& array)
												{
													// Remove under lock
													mLock.lockForWriting();
													for (CArrayItemIndex i = 0; i < array.getCount(); i++)
														// Remove
														remove(array[i]);
													mLock.unlockForWriting();

													return *this;
												}
						TLockingArray<T>&	removeAtIndex(CArrayItemIndex itemIndex)
												{
													// Remove under lock
													mLock.lockForWriting();
													CArray::removeAtIndex(itemIndex);
													mLock.unlockForWriting();

													return *this;
												}
						TLockingArray<T>&	removeAll()
												{
													// Remove under lock
													mLock.lockForWriting();
													CArray::removeAll();
													mLock.unlockForWriting();

													return *this;
												}

				const	T					popFirst()
												{
													// Perform under lock
													mLock.lockForWriting();
													T	item = *((T*) CArray::getFirst());
													CArray::removeAtIndex(0);
													mLock.unlockForWriting();

													return item;
												}

											// Instance methods
				const	T&					operator[] (CArrayItemIndex index) const
												{ return *((T*) getItemAt(index)); }
						TLockingArray<T>&	operator+=(const T& item)
												{ return add(item); }
						TLockingArray<T>&	operator+=(const TArray<T>& other)
												{ return addFrom(other); }
						TLockingArray<T>&	operator+=(const TLockingArray<T>& other)
												{ return addFrom(other); }
						TLockingArray<T>&	operator-=(const T& item)
												{ return remove(item); }
						TLockingArray<T>&	operator-=(const TArray<T>& other)
												{ return removeFrom(other); }
						TLockingArray<T>&	operator-=(const TLockingArray<T>& other)
												{ return removeFrom(other); }

	private:
											// Class methods
		static	T*							copy(CArrayItemRef itemRef)
												{ return new T(*((T*) itemRef)); }
		static	void						dispose(CArrayItemRef itemRef)
												{ T* t = (T*) itemRef; Delete(t); }

	// Properties
	private:
		CReadPreferringLock	mLock;
};
