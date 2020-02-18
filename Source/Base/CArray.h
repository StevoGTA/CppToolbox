//----------------------------------------------------------------------------------------------------------------------
//	CArray.h			©2003 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CEquatable.h"
#include "CIterator.h"
#include "Compare.h"
#include "SNumber.h"
#include "TOptional.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Types

typedef	const	void*	CArrayItemRef;

typedef			UInt32	CArrayItemIndex;

typedef			UInt32	CArrayItemCount;

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Procs

typedef	void			(*CArrayApplyProc)(CArrayItemRef itemRef, void* userData);
typedef	CArrayItemRef	(*CArrayItemCopyProc)(CArrayItemRef itemRef);
typedef	void			(*CArrayItemDisposeProc)(CArrayItemRef itemRef);
typedef bool			(*CArrayItemIsIncludedProc)(CArrayItemRef itemRef, void* userData);

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
class CArray : public CEquatable {
	// Methods
	public:
											// Lifecycle methods
		virtual								~CArray();

											// CEquatable methods
				bool						operator==(const CEquatable& other) const
												{ return equals((const CArray&) other); }

											// Instance methods
		virtual	CArrayItemCount				getCount() const;
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
				CArrayItemRef				copy(const CArrayItemRef itemRef) const;

				CArray&						add(const CArrayItemRef itemRef, bool avoidDuplicates = false);
				CArray&						addFrom(const CArray& other, bool avoidDuplicates = false);

				bool						contains(const CArrayItemRef itemRef) const;

				CArrayItemRef				getFirst() const
												{ return getItemAt(0); }
				CArrayItemRef				getLast() const;
				OV<CArrayItemIndex>			getIndexOf(const CArrayItemRef itemRef) const;

				CArray&						insertAtIndex(const CArrayItemRef itemRef, CArrayItemIndex itemIndex);

				CArray&						detach(const CArrayItemRef itemRef);		// Removes itemRef without calling itemDisposeProc
				CArray&						detachAtIndex(CArrayItemIndex itemIndex);	// Removes at itemIndex without calling itemDisposeProc

				CArray&						remove(const CArrayItemRef itemRef);
				CArray&						removeFrom(const CArray& other);
				CArray&						removeAtIndex(CArrayItemIndex itemIndex);
				CArray&						removeAll();

				bool						equals(const CArray& other) const;

				TIteratorS<CArrayItemRef>	getIterator() const;
				CArray&						apply(CArrayApplyProc applyProc, void* userData = nil);

				CArray&						sort(CArrayCompareProc compareProc, void* userData = nil);
				CArray						sorted(CArrayCompareProc compareProc, void* userData = nil) const;

				CArray						filtered(CArrayItemIsIncludedProc isIncludedProc, void* userData = nil);

				CArray&						operator=(const CArray& other);
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
											TArray(CArrayItemCopyProc itemCopyProc) :
												CArray(0, itemCopyProc, dispose)
												{}
//											TArray(bool ownsItems) : CArray(0, ownsItems ? dispose : nil) {}
											TArray(const T& item, CArrayItemCopyProc itemCopyProc) :
												CArray(0, itemCopyProc, dispose)
												{ CArray::add(new T(item), false); }
//											TArray(const CArray& array, T (mappingProc)(CArrayItemRef item)) : CArray()
//												{
//													// Iterate all items
//													for (CArrayItemIndex i = 0; i < array.getCount(); i++) {
//															// Map item
//														T	t = mappingProc(array.getItemAt(i));
//
//														// Add result
//														CArray::add(&t);
//													}
//												}
											TArray(const TArray<T>& array) : CArray(array) {}

											// CArray methods
						TArray<T>&			add(const T& item)
												{ CArray::add(CArray::copy(&item)); return *this; }
						TArray<T>&			addFrom(const TArray<T>& array)
												{
													// Iterate all
													for (CArrayItemIndex i = 0; i < array.getCount(); i++)
														// Add
														CArray::add(new T(array[i]));

													return *this;
												}

						bool				contains(const T& item) const
												{ return getIndexOf(item).hasValue(); }

						T&					getAt(CArrayItemIndex index) const
												{ return *((T*) getItemAt(index)); }
						T&					getFirst() const
												{ return *((T*) CArray::getFirst()); }
						T&					getLast() const
												{ return *((T*) CArray::getLast()); }
						OV<CArrayItemIndex>	getIndexOf(const T& item) const
												{
													// Iterate all
													for (CArrayItemIndex i = 0; i < getCount(); i++) {
														// Check if same
														if (item == getAt(i))
															// Match
															return OV<CArrayItemIndex>(i);
													}

													return OV<CArrayItemIndex>();
												}
						OV<CArrayItemIndex>	getIndexWhere(bool (proc)(const T& item, void* userData),
													void* userData = nil)
												{
													// Iterate all items
													for (CArrayItemIndex i = 0; i < getCount(); i++) {
														// Get item
														const	T&	item = getAt(i);

														// Call proc
														if (proc(item, userData))
															// Proc indicates to return this item
															return OV<CArrayItemIndex>(i);
													}

													return OV<CArrayItemIndex>();
												}

						TArray<T>&			insertAtIndex(const T& item, CArrayItemIndex itemIndex)
												{ CArray::insertAtIndex(new T(item), itemIndex); return *this; }

						TArray<T>&			remove(const T& item)
												{
													// Check if found
													OV<CArrayItemIndex>	index = getIndexOf(item);
													if (index.hasValue())
														// Remove
														removeAtIndex(*index);

													return *this;
												}
						TArray<T>&			removeFrom(const TArray<T>& array)
												{
													// Iterate all
													for (CArrayItemIndex i = 0; i < array.getCount(); i++)
														// Remove
														remove(array[i]);

													return *this;
												}
						TArray<T>&			removeAtIndex(CArrayItemIndex itemIndex)
												{ CArray::removeAtIndex(itemIndex); return *this; }
						TArray<T>&			removeAll()
												{ CArray::removeAll(); return *this; }

						TIteratorD<T>		getIterator() const
												{ TIteratorS<CArrayItemRef> iterator = CArray::getIterator();
													return TIteratorD<T>((TIteratorD<T>*) &iterator); }
//						TArray<T>&			apply(void (proc)(T& item, void* userData), void* userData = nil)
//												{ CArray::apply((CArrayApplyProc) proc, userData); return *this; }

//						TArray<T>&			sort(ECompareResult (proc)(const T& item1, const T& item2, void* userData),
//													void* userData = nil)
//												{ CArray::sort((CArrayCompareProc) proc, userData); return *this; }

											// Instance methods
						OR<T>				getFirst(bool (proc)(const T& item, void* userData), void* userData = nil)
												{
													// Iterate all items
													for (CArrayItemIndex i = 0; i < getCount(); i++) {
														// Get item
														T&	item = getAt(i);

														// Call proc
														if (proc(item, userData))
															// Proc indicates to return this item
															return OR<T>(item);
													}

													return OR<T>();
												}

						T					popFirst()
												{
													// Get first item
													T	item = getFirst();

													// Remove
													removeAtIndex(0);

													return item;
												}
						OV<T>				popFirst(bool (proc)(const T& item, void* userData), void* userData = nil)
												{
													// Iterate all items
													for (CArrayItemIndex i = 0; i < getCount(); i++) {
														// Get item
														T&	item = getAt(i);

														// Call proc
														if (proc(item, userData)) {
															// Proc indicates to return this item
															OV<T>	reference(item);
															removeAtIndex(i);

															return reference;
														}
													}

													return OV<T>();
												}

						T&					operator[] (CArrayItemIndex index) const
												{ return *((T*) getItemAt(index)); }
						TArray<T>&			operator+=(const T& item)
												{ return add(item); }
						TArray<T>&			operator+=(const TArray<T>& other)
												{ return addFrom(other); }
						TArray<T>&			operator-=(const T& item)
												{ return remove(item); }
						TArray<T>&			operator-=(const TArray<T>& other)
												{ return removeFrom(other); }

	private:
											// Class methods
		static	void						dispose(CArrayItemRef itemRef)
												{ T* t = (T*) itemRef; DisposeOf(t); }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TNArray (TArray where copy happens through new T())

template <typename T> class TNArray : public TArray<T> {
	// Methods
	public:
					// Lifecycle methods
					TNArray() : TArray<T>((CArrayItemCopyProc) copy) {}
					TNArray(const T& item) : TArray<T>(item, (CArrayItemCopyProc) copy) {}
					TNArray(const CArray& array, T (mappingProc)(CArrayItemRef item)) :
						TArray<T>((CArrayItemCopyProc) copy)
						{
							// Iterate all items
							for (CArrayItemIndex i = 0; i < array.getCount(); i++)
								// Add mapped item
								this->add(mappingProc(array.getItemAt(i)));
						}

	private:
					// Class methods
		static	T*	copy(CArrayItemRef itemRef)
						{ return new T(*((T*) itemRef)); }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TCArray (TArray where copy happens trhough itemRef->copy())

template <typename T> class TCArray : public TArray<T> {
	// Methods
	public:
					// Lifecycle methods
					TCArray() : TArray<T>((CArrayItemCopyProc) copy) {}
					TCArray(const T& item) : TArray<T>(item, (CArrayItemCopyProc) copy) {}

	private:
					// Class methods
		static	T*	copy(CArrayItemRef itemRef)
						{ return ((T*) itemRef)->copy(); }
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
													TNumericArray(const CArray& array,
															T (mappingProc)(CArrayItemRef item)) :
														CArray(0, (CArrayItemCopyProc) copy, dispose)
														{
															for (CArrayItemIndex i = 0; i < array.getCount(); i++)
																CArray::add(mappingProc(array.getItemAt(i)));
														}
													TNumericArray(const TNumericArray<T>& array) : CArray(array) {}

													// CArray methods
				TNumericArray<T>&					add(T value)
														{ CArray::add(new SNumberWrapper<T>(value)); return *this; }

				T									getAt(CArrayItemIndex index) const
														{ return ((SNumberWrapper<T>*) getItemAt(index))->mValue; }

				TIteratorM<SNumberWrapper<T>, T>	getIterator() const
														{
															TIteratorS<CArrayItemRef> iterator = CArray::getIterator();

															return TIteratorM<SNumberWrapper<T>, T>(
																	(TIteratorM<SNumberWrapper<T>, T>*) &iterator,
																	getValueForRawValue);
														}

													// Instance methods
				T									operator[] (CArrayItemIndex index) const
														{ return ((SNumberWrapper<T>*) getItemAt(index))->mValue; }
				TNumericArray<T>&					operator+=(T value)
														{ CArray::add(new SNumberWrapper<T>(value)); return *this; }

													// Class methods
		static	SNumberWrapper<T>*					copy(CArrayItemRef itemRef)
														{ return new SNumberWrapper<T>(
																*((SNumberWrapper<T>*) itemRef)); }
		static	void								dispose(CArrayItemRef itemRef)
														{
															SNumberWrapper<T>*	numberWrapper =
																						(SNumberWrapper<T>*) itemRef;
															DisposeOf(numberWrapper);
														}

	private:
													// Class methods
		static	T									getValueForRawValue(SNumberWrapper<T>** rawValue)
														{ return (*rawValue)->mValue; }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TPtrArray

template <typename T> class TPtrArray : public CArray {
	// Methods
	public:
											// Lifecycle methods
											TPtrArray(CArrayItemCount initialCapacity = 0, bool ownsItems = false) :
												CArray(initialCapacity, nil, ownsItems ? dispose : nil)
												{}
											TPtrArray(bool ownsItems) : CArray(0, nil, ownsItems ? dispose : nil) {}
											TPtrArray(const T item, bool ownsItems = false) :
												CArray(0, nil, ownsItems ? dispose : nil)
												{ CArray::add(item, false); }
											TPtrArray(const CArray& array, T (mappingProc)(CArrayItemRef item)) :
												CArray()
												{
													// Iterate all items
													for (CArrayItemIndex i = 0; i < array.getCount(); i++)
														// Call mapping proc and add result
														CArray::add(mappingProc(array.getItemAt(i)));
												}
											TPtrArray(const TPtrArray<T>& array) : CArray(array) {}

											// CArray methods
						TPtrArray<T>&		add(const T item, bool avoidDuplicates = false)
												{ CArray::add(item, avoidDuplicates); return *this; }
						TPtrArray<T>&		addFrom(const TPtrArray<T>& array, bool avoidDuplicates = false)
												{ CArray::addFrom(array, avoidDuplicates); return *this; }

						bool				contains(const T item) const
												{ return CArray::contains(item); }

				const	T					getAt(CArrayItemIndex index) const
												{ return (T) getItemAt(index); }
				const	T					getFirst() const
												{ return (T) CArray::getFirst(); }
				const	T					getLast() const
												{ return (T) CArray::getLast(); }
						OV<CArrayItemIndex>	getIndexOf(const T item) const
												{ return CArray::getIndexOf(item); }

						TPtrArray<T>&		insertAtIndex(const T item, CArrayItemIndex itemIndex)
												{ CArray::insertAtIndex(item, itemIndex); return *this; }

						TPtrArray<T>&		detach(const T item)
												{ CArray::detach(item); return *this; }
						TPtrArray<T>&		detachAtIndex(CArrayItemIndex itemIndex)
												{ CArray::detachAtIndex(itemIndex); return *this; }
						TPtrArray<T>&		remove(const T item)
												{ CArray::remove(item); return *this; }
						TPtrArray<T>&		removeFrom(const TPtrArray<T>& array)
												{ CArray::removeFrom(array); return *this; }
						TPtrArray<T>&		removeAtIndex(CArrayItemIndex itemIndex)
												{ CArray::removeAtIndex(itemIndex); return *this; }
						TPtrArray<T>&		removeAll()
												{ CArray::removeAll(); return *this; }

						TIteratorS<T>		getIterator() const
												{ TIteratorS<CArrayItemRef> iterator = CArray::getIterator();
														return TIteratorS<T>((TIteratorS<T>*) &iterator); }
						TPtrArray<T>&		apply(void (proc)(const T item, void* userData), void* userData = nil)
												{ CArray::apply((CArrayApplyProc) proc, userData); return *this; }

						TPtrArray<T>&		sort(ECompareResult (proc)(const T item1, const T item2, void* userData),
													void* userData = nil)
												{ CArray::sort((CArrayCompareProc) proc, userData); return *this; }

											// Instance methods
						OV<T>				getFirst(bool (proc)(const T item, void* userData), void* userData = nil)
												{
													// Iterate all items
													for (CArrayItemIndex i = 0; i < getCount(); i++) {
														// Get item
														const	T	item = (T) getItemAt(i);

														// Call proc
														if (proc(item, userData))
															// Proc indicates to return this item
															return OV<T>(item);
													}

													return OV<T>();
												}

				const	T					operator[] (CArrayItemIndex index) const
												{ return (T) getItemAt(index); }
						TPtrArray<T>&		operator+=(const T item)
												{ CArray::add(item); return *this; }
						TPtrArray<T>&		operator+=(const TPtrArray<T>& array)
												{ CArray::addFrom(array, false); return *this; }
						TPtrArray<T>&		operator-=(const T item)
												{ CArray::remove(item); return *this; }
						TPtrArray<T>&		operator-=(const TPtrArray<T>& array)
												{ CArray::removeFrom(array); return *this; }

	private:
											// Class methods
		static	void						dispose(CArrayItemRef itemRef)
												{ T t = (T) itemRef; DisposeOf(t); }
};
