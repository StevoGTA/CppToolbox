//----------------------------------------------------------------------------------------------------------------------
//	CArray.h			©2003 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CIterator.h"
#include "Compare.h"
#include "SNumber.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Types

typedef	const	void*	CArrayItemRef;

typedef			UInt32	CArrayItemIndex;
const CArrayItemIndex	kCArrayItemIndexNotFound = 0xFFFFFFFF;

typedef			UInt32	CArrayItemCount;

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Procs

typedef	void			(*CArrayApplyProc)(CArrayItemRef itemRef, void* userData);
typedef	CArrayItemRef	(*CArrayItemCopyProc)(CArrayItemRef itemRef);
typedef	void			(*CArrayItemDisposeProc)(CArrayItemRef itemRef);

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Sorting

enum EArraySortOptions {
	kArraySortOptionsDefault	= 0,
};

typedef	ECompareResult	(*CArrayCompareProc)(CArrayItemRef itemRef1, CArrayItemRef itemRef2, void* userData);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CArray

class CArrayInternals;
class CArray {
	// Methods
	public:
											// Lifecycle methods
		virtual								~CArray();

											// Instance methods
				CArrayItemCount				getCount() const;
				bool						isEmpty() const
												{ return getCount() == 0; }

				CArrayItemRef				getItemAt(CArrayItemIndex itemIndex) const;


				bool						isSorted() const;

	protected:
											// Lifecycle methods
											CArray(CArrayItemCount initialCapacity = 0,
													CArrayItemCopyProc itemCopyProc = nil,
													CArrayItemDisposeProc itemDisposeProc = nil);
											CArray(const CArray& other);

											// Instance methods
				CArray&						add(const CArrayItemRef itemRef, bool avoidDuplicates = false);
				CArray&						addFrom(const CArray& other, bool avoidDuplicates = false);

				bool						contains(const CArrayItemRef itemRef) const;

				CArrayItemRef				getFirst() const
												{ return getItemAt(0); }
				CArrayItemRef				getLast() const;
				CArrayItemIndex				getIndexOf(const CArrayItemRef itemRef) const;

				CArray&						insertAtIndex(const CArrayItemRef itemRef, CArrayItemIndex itemIndex);

				CArray&						remove(const CArrayItemRef itemRef);
				CArray&						removeFrom(const CArray& other);
				CArray&						removeAtIndex(CArrayItemIndex itemIndex);
				CArray&						removeAll();

				TIterator<CArrayItemRef>	getIterator() const;
				CArray&						apply(CArrayApplyProc applyProc, void* userData = nil);

				CArray&						sort(CArrayCompareProc compareProc, void* userData = nil);

				CArray&						operator=(const CArray& other);
				bool						operator==(const CArray& other) const;
				bool						operator!=(const CArray& other) const
												{ return !operator==(other); }
				CArray&						operator+=(const CArray& other)
												{ return addFrom(other); }

	// Properties
	private:
		CArrayInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TArray

template <typename T> class TArray : public CArray {
	// Methods
	public:
										// Lifecycle methods
										TArray(CArrayItemCount initialCapacity = 0, bool ownsItems = false) :
											CArray(initialCapacity, nil, ownsItems ? dispose : nil)
											{}
										TArray(bool ownsItems) : CArray(0, nil, ownsItems ? dispose : nil) {}
										TArray(const T item, bool ownsItems = false) :
											CArray(0, nil, ownsItems ? dispose : nil)
											{ CArray::add(item, false); }
										TArray(const CArray& array, T (mappingProc)(CArrayItemRef item)) : CArray()
											{
												// Iterate all items
												for (CArrayItemIndex i = 0; i < array.getCount(); i++)
													// Call mapping proc and add result
													CArray::add(mappingProc(array.getItemAt(i)));
											}
										TArray(const TArray<T>& array) : CArray(array) {}

										// CArray methods
						TArray<T>&		add(const T item, bool avoidDuplicates = false)
											{ CArray::add(item, avoidDuplicates); return *this; }
						TArray<T>&		addFrom(const TArray<T>& array, bool avoidDuplicates = false)
											{ CArray::addFrom(array, avoidDuplicates); return *this; }

						bool			contains(const T item) const
											{ return CArray::contains(item); }

				const	T				getAt(CArrayItemIndex index) const
											{ return (T) getItemAt(index); }
				const	T				getFirst() const
											{ return (T) CArray::getFirst(); }
				const	T				getLast() const
											{ return (T) CArray::getLast(); }
						CArrayItemIndex	getIndexOf(const T item) const
											{ return CArray::getIndexOf(item); }

						TArray<T>&		insertAtIndex(const T item, CArrayItemIndex itemIndex)
											{ CArray::insertAtIndex(item, itemIndex); return *this; }

						TArray<T>&		remove(const T item)
											{ CArray::remove(item); return *this; }
						TArray<T>&		removeFrom(const TArray<T>& array)
											{ CArray::removeFrom(array); return *this; }
						TArray<T>&		removeAtIndex(CArrayItemIndex itemIndex)
											{ CArray::removeAtIndex(itemIndex); return *this; }
						TArray<T>&		removeAll()
											{ CArray::removeAll(); return *this; }

						TIterator<T>	getIterator() const
											{ TIterator<CArrayItemRef> iterator = CArray::getIterator();
													return TIterator<T>((TIterator<T>*) &iterator); }
						TArray<T>&		apply(void (proc)(const T item, void* userData), void* userData = nil)
											{ CArray::apply((CArrayApplyProc) proc, userData); return *this; }

						TArray<T>&		sort(ECompareResult (proc)(const T item1, const T item2, void* userData),
												void* userData = nil)
											{ CArray::sort((CArrayCompareProc) proc, userData); return *this; }

										// Instance methods
				const	T				getFirst(bool (proc)(const T item, void* userData), void* userData = nil)
											{
												// Iterate all items
												for (CArrayItemIndex i = 0; i < getCount(); i++) {
													// Get item
													const	T	item = (T) getItemAt(i);

													// Call proc
													if (proc(item, userData))
														// Proc indicates to return this item
														return item;
												}

												return nil;
											}

				const	T				operator[] (CArrayItemIndex index) const
											{ return (T) getItemAt(index); }
						TArray<T>&		operator+=(const T item)
											{ CArray::add(item); return *this; }
						TArray<T>&		operator+=(const TArray<T>& array)
											{ CArray::addFrom(array, false); return *this; }
						TArray<T>&		operator-=(const T item)
											{ CArray::remove(item); return *this; }
						TArray<T>&		operator-=(const TArray<T>& array)
											{ CArray::removeFrom(array); return *this; }

	private:
										// Class methods
		static	void					dispose(CArrayItemRef itemRef)
											{ T t = (T) itemRef; DisposeOf(t); }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TArrayT

template <typename T> class TArrayT : public CArray {
	// Methods
	public:
										// Lifecycle methods
										TArrayT(CArrayItemCount initialCapacity = 0) :
											CArray(initialCapacity, (CArrayItemCopyProc) copy, dispose)
											{}
//										TArrayT(bool ownsItems) : CArray(0, ownsItems ? dispose : nil) {}
										TArrayT(const T& item) : CArray(0, (CArrayItemCopyProc) copy, dispose)
											{ CArray::add(new T(item), false); }
//										TArrayT(const CArray& array, T (mappingProc)(CArrayItemRef item)) : CArray()
//											{
//												// Iterate all items
//												for (CArrayItemIndex i = 0; i < array.getCount(); i++) {
//													// Map item
//													T	t = mappingProc(array.getItemAt(i));
//
//													// Add result
//													CArray::add(&t);
//												}
//											}
										TArrayT(const TArrayT<T>& array) : CArray(array) {}

										// CArray methods
						TArrayT<T>&		add(const T& item)
											{ CArray::add(new T(item)); return *this; }
						TArrayT<T>&		addFrom(const TArrayT<T>& array)
											{
												// Iterate all
												for (CArrayItemIndex i = 0; i < array.getCount(); i++)
													// Add
													CArray::add(new T(array[i]));

												return *this;
											}

						bool			contains(const T& item) const
											{ return getIndexOf(item) != kCArrayItemIndexNotFound; }

				const	T&				getAt(CArrayItemIndex index) const
											{ return *((T*) getItemAt(index)); }
				const	T&				getFirst() const
											{ return *((T*) CArray::getFirst()); }
				const	T&				getLast() const
											{ return *((T*) CArray::getLast()); }
						CArrayItemIndex	getIndexOf(const T& item) const
											{
												// Iterate all
												for (CArrayItemIndex i = 0; i < getCount(); i++) {
													// Check if same
													if (item == getAt(i))
														// Match
														return i;
												}

												return kCArrayItemIndexNotFound;
											}

						TArrayT<T>&		insertAtIndex(const T& item, CArrayItemIndex itemIndex)
											{ CArray::insertAtIndex(new T(item), itemIndex); return *this; }

						TArrayT<T>&		remove(const T& item)
											{
												// Check if found
												CArrayItemIndex	index = getIndexOf(item);
												if (index != kCArrayItemIndexNotFound)
													// Remove
													removeAtIndex(index);

												return *this;
											}
						TArrayT<T>&		removeFrom(const TArrayT<T>& array)
											{
												// Iterate all
												for (CArrayItemIndex i = 0; i < array.getCount(); i++)
													// Remove
													remove(array[i]);

												return *this;
											}
						TArrayT<T>&		removeAtIndex(CArrayItemIndex itemIndex)
											{ CArray::removeAtIndex(itemIndex); return *this; }
						TArrayT<T>&		removeAll()
											{ CArray::removeAll(); return *this; }

						TIterator<T*>	getIterator() const
											{ TIterator<CArrayItemRef> iterator = CArray::getIterator();
													return TIterator<T*>((TIterator<T*>*) &iterator); }
//						TArrayT<T>&		apply(void (proc)(T& item, void* userData), void* userData = nil)
//											{ CArray::apply((CArrayApplyProc) proc, userData); return *this; }

//						TArrayT<T>&		sort(ECompareResult (proc)(const T& item1, const T& item2, void* userData),
//												void* userData = nil)
//											{ CArray::sort((CArrayCompareProc) proc, userData); return *this; }

										// Instance methods
//				const	T*				getFirst(bool (proc)(const T& item, void* userData), void* userData = nil)
//											{
//												// Iterate all items
//												for (CArrayItemIndex i = 0; i < getCount(); i++) {
//													// Get item
//													const	T&	item = getAt(i);
//
//													// Call proc
//													if (proc(item, userData))
//														// Proc indicates to return this item
//														return &item;
//												}
//
//												return nil;
//											}

				const	T&				operator[] (CArrayItemIndex index) const
											{ return *((T*) getItemAt(index)); }
						TArrayT<T>&		operator+=(const T& item)
											{ return add(item); }
						TArrayT<T>&		operator+=(const TArrayT<T>& array)
											{ return addFrom(array); }
						TArrayT<T>&		operator-=(const T& item)
											{ return remove(item); }
						TArrayT<T>&		operator-=(const TArray<T>& array)
											{ return removeFrom(array); }

	private:
										// Class methods
		static	T*						copy(CArrayItemRef itemRef)
											{ return new T(*((T*) itemRef)); }
		static	void					dispose(CArrayItemRef itemRef)
											{ T* t = (T*) itemRef; DisposeOf(t); }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TNumericArray

template <typename T> class TNumericArray : public CArray {

	// Methods
	public:
											// Lifecycle methods
											TNumericArray(CArrayItemCount initialCapacity = 0) :
												CArray(initialCapacity, (CArrayItemCopyProc) copy, dispose)
												{}
											TNumericArray(const CArray& array, T (mappingProc)(CArrayItemRef item)) :
												CArray(0, (CArrayItemCopyProc) copy, dispose)
												{
													for (CArrayItemIndex i = 0; i < array.getCount(); i++)
														CArray::add(mappingProc(array.getItemAt(i)));
												}
											TNumericArray(const TNumericArray<T>& array) : CArray(array) {}

											// CArray methods
						TNumericArray<T>&	add(T value)
												{ CArray::add(new SNumberWrapper<T>(value)); return *this; }

						T					getAt(CArrayItemIndex index) const
												{ return ((SNumberWrapper<T>*) getItemAt(index))->mValue; }

											// Instance methods
						T					operator[] (CArrayItemIndex index) const
												{ return ((SNumberWrapper<T>*) getItemAt(index))->mValue; }
						TNumericArray<T>&	operator+=(T value)
												{ CArray::add(new SNumberWrapper<T>(value)); return *this; }

											// Class methods
		static	SNumberWrapper<T>*			copy(CArrayItemRef itemRef)
												{ return new SNumberWrapper<T>(*((SNumberWrapper<T>*) itemRef)); }
		static	void						dispose(CArrayItemRef itemRef)
												{
													SNumberWrapper<T>*	numberWrapper = (SNumberWrapper<T>*) itemRef;
													DisposeOf(numberWrapper);
												}
};
