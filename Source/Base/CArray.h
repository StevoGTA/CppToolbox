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
// MARK: CArray

class CArrayInternals;
class CArray : public CEquatable {
	// Types:
	public:
		typedef	const	void*	ItemRef;

		typedef			UInt32	ItemIndex;

		typedef			UInt32	ItemCount;

//		enum SortOptions {
//			kSortOptionsDefault	= 0,
//		};

	// Procs:
	public:
		typedef	void			(*ApplyProc)(ItemRef itemRef, void* userData);
		typedef	ECompareResult	(*CompareProc)(ItemRef itemRef1, ItemRef itemRef2, void* userData);
		typedef	ItemRef			(*CopyProc)(ItemRef itemRef);
		typedef	void			(*DisposeProc)(ItemRef itemRef);
		typedef bool			(*IsIncludedProc)(ItemRef itemRef, void* userData);

	// Methods
	public:
									// Lifecycle methods
		virtual						~CArray();

									// CEquatable methods
				bool				operator==(const CEquatable& other) const
										{ return equals((const CArray&) other); }

									// Instance methods
		virtual	ItemCount			getCount() const;
				bool				isEmpty() const
										{ return getCount() == 0; }

				ItemRef				getItemAt(ItemIndex itemIndex) const;

				bool				isSorted() const;

	protected:
									// Lifecycle methods
									CArray(ItemCount initialCapacity = 0, CopyProc copyProc = nil,
											DisposeProc disposeProc = nil);
									CArray(const CArray& other);

									// Instance methods
				ItemRef				copy(const ItemRef itemRef) const;

				CArray&				add(const ItemRef itemRef, bool avoidDuplicates = false);
				CArray&				addFrom(const CArray& other, bool avoidDuplicates = false);

				bool				contains(const ItemRef itemRef) const;

				ItemRef				getFirst() const
										{ return getItemAt(0); }
				ItemRef				getLast() const;
				OV<ItemIndex>		getIndexOf(const ItemRef itemRef) const;

				CArray&				insertAtIndex(const ItemRef itemRef, ItemIndex itemIndex);

				CArray&				detach(const ItemRef itemRef);		// Removes itemRef without calling itemDisposeProc
				CArray&				detachAtIndex(ItemIndex itemIndex);	// Removes at itemIndex without calling itemDisposeProc

				CArray&				move(const ItemRef itemRef, CArray& other);

				CArray&				remove(const ItemRef itemRef);
				CArray&				removeFrom(const CArray& other);
				CArray&				removeAtIndex(ItemIndex itemIndex);
				CArray&				removeAll();

				bool				equals(const CArray& other) const;

				TIteratorS<ItemRef>	getIterator() const;
				CArray&				apply(ApplyProc applyProc, void* userData = nil);

				CArray&				sort(CompareProc compareProc, void* userData = nil);
				CArray				sorted(CompareProc compareProc, void* userData = nil) const;

				CArray				filtered(IsIncludedProc isIncludedProc, void* userData = nil);

				CArray&				operator=(const CArray& other);
				CArray&				operator+=(const CArray& other)
										{ return addFrom(other); }

	// Properties
	private:
		CArrayInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TArray
//	TArrays are used when passing an array across an API boundary.  Because object lifetime management is tricky in
//		C++, the TArray handles it all internally.  TArrays are not to be instantiated directly, but instead to be
//		done through either TNArray or TCArray.  Objects in the TArray need to be reference counted to ensure proper
//		lifetime management.

template <typename T> class TArray : public CArray {
	// Methods
	public:
								// Lifecycle methods
								TArray(const TArray<T>& array) : CArray(array) {}

								// CArray methods
				TArray<T>&		add(const T& item)
									{ CArray::add(CArray::copy(&item)); return *this; }
				TArray<T>&		addFrom(const TArray<T>& array)
									{
										// Iterate all
										for (ItemIndex i = 0; i < array.getCount(); i++)
											// Add
											CArray::add(new T(array[i]));

										return *this;
									}

				bool			contains(const T& item) const
									{ return getIndexOf(item).hasValue(); }

				T&				getAt(ItemIndex index) const
									{ return *((T*) getItemAt(index)); }
				T&				getFirst() const
									{ return *((T*) CArray::getFirst()); }
				T&				getLast() const
									{ return *((T*) CArray::getLast()); }
				OV<ItemIndex>	getIndexOf(const T& item) const
									{
										// Iterate all
										for (ItemIndex i = 0; i < getCount(); i++) {
											// Check if same
											if (item == getAt(i))
												// Match
												return OV<ItemIndex>(i);
										}

										return OV<ItemIndex>();
									}
				OV<ItemIndex>	getIndexWhere(bool (proc)(const T& item, void* userData), void* userData = nil)
									{
										// Iterate all items
										for (ItemIndex i = 0; i < getCount(); i++) {
											// Get item
											const	T&	item = getAt(i);

											// Call proc
											if (proc(item, userData))
												// Proc indicates to return this item
												return OV<ItemIndex>(i);
										}

										return OV<ItemIndex>();
									}

				TArray<T>&		insertAtIndex(const T& item, ItemIndex itemIndex)
									{ CArray::insertAtIndex(new T(item), itemIndex); return *this; }

				TArray<T>&		remove(const T& item)
									{
										// Check if found
										OV<ItemIndex>	index = getIndexOf(item);
										if (index.hasValue())
											// Remove
											removeAtIndex(*index);

										return *this;
									}
				TArray<T>&		removeFrom(const TArray<T>& array)
									{
										// Iterate all
										for (ItemIndex i = 0; i < array.getCount(); i++)
											// Remove
											remove(array[i]);

										return *this;
									}
				TArray<T>&		removeAtIndex(ItemIndex itemIndex)
									{ CArray::removeAtIndex(itemIndex); return *this; }
				TArray<T>&		removeAll()
									{ CArray::removeAll(); return *this; }

				TIteratorD<T>	getIterator() const
									{ TIteratorS<ItemRef> iterator = CArray::getIterator();
										return TIteratorD<T>((TIteratorD<T>*) &iterator); }

				TArray<T>&		sort(ECompareResult (compareProc)(const T& item1, const T& item2, void* userData),
										void* userData = nil)
									{ CArray::sort((CompareProc) compareProc, userData); return *this; }

								// Instance methods
				OR<T>			getFirst(bool (proc)(const T& item, void* userData), void* userData = nil)
									{
										// Iterate all items
										for (ItemIndex i = 0; i < getCount(); i++) {
											// Get item
											T&	item = getAt(i);

											// Call proc
											if (proc(item, userData))
												// Proc indicates to return this item
												return OR<T>(item);
										}

										return OR<T>();
									}

				T				popFirst()
									{
										// Get first item
										T	item = getFirst();

										// Remove
										removeAtIndex(0);

										return item;
									}
				OV<T>			popFirst(bool (proc)(const T& item, void* userData), void* userData = nil)
									{
										// Iterate all items
										for (ItemIndex i = 0; i < getCount(); i++) {
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

				T&				operator[] (ItemIndex index) const
									{ return *((T*) getItemAt(index)); }
				TArray<T>&		operator+=(const T& item)
									{ return add(item); }
				TArray<T>&		operator+=(const TArray<T>& other)
									{ return addFrom(other); }
				TArray<T>&		operator-=(const T& item)
									{ return remove(item); }
				TArray<T>&		operator-=(const TArray<T>& other)
									{ return removeFrom(other); }

	protected:
								// Lifecycle methods
								TArray(CopyProc copyProc) : CArray(0, copyProc, dispose) {}
								TArray(const T& item, CopyProc copyProc) : CArray(0, copyProc, dispose) { add(item); }

	private:
								// Class methods
		static	void			dispose(ItemRef itemRef)
									{ T* t = (T*) itemRef; Delete(t); }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TNArray (TArray where copy happens through new T())

template <typename T> class TNArray : public TArray<T> {
	// Methods
	public:
					// Lifecycle methods
					TNArray() : TArray<T>((CArray::CopyProc) copy) {}
					TNArray(const T& item) : TArray<T>(item, (CArray::CopyProc) copy) {}
					TNArray(const CArray& array, T (mappingProc)(CArray::ItemRef item)) :
						TArray<T>((CArray::CopyProc) copy)
						{
							// Iterate all items
							for (CArray::ItemIndex i = 0; i < array.getCount(); i++)
								// Add mapped item
								this->add(mappingProc(array.getItemAt(i)));
						}

	private:
					// Class methods
		static	T*	copy(CArray::ItemRef itemRef)
						{ return new T(*((T*) itemRef)); }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TCArray (TArray where copy happens through itemRef->copy())

template <typename T> class TCArray : public TArray<T> {
	// Methods
	public:
					// Lifecycle methods
					TCArray() : TArray<T>((CArray::CopyProc) copy) {}
					TCArray(const T& item) : TArray<T>(item, (CArray::CopyProc) copy) {}

	private:
					// Class methods
		static	T*	copy(CArray::ItemRef itemRef)
						{ return ((T*) itemRef)->copy(); }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TNumericArray

template <typename T> class TNumericArray : public CArray {

	// Methods
	public:
													// Lifecycle methods
													TNumericArray(CArray::ItemCount initialCapacity = 0) :
														CArray(initialCapacity, (CArray::CopyProc) copy, dispose)
														{}
													TNumericArray(const CArray& array,
															T (mappingProc)(ItemRef item)) :
														CArray(0, (CArray::CopyProc) copy, dispose)
														{
															for (ItemIndex i = 0; i < array.getCount(); i++)
																CArray::add(mappingProc(array.getItemAt(i)));
														}
													TNumericArray(const TNumericArray<T>& array) : CArray(array) {}

													// CArray methods
				TNumericArray<T>&					add(T value)
														{ CArray::add(new SNumberWrapper<T>(value)); return *this; }

				T									getAt(ItemIndex index) const
														{ return ((SNumberWrapper<T>*) getItemAt(index))->mValue; }

				TIteratorM<SNumberWrapper<T>, T>	getIterator() const
														{
															TIteratorS<ItemRef> iterator = CArray::getIterator();

															return TIteratorM<SNumberWrapper<T>, T>(
																	(TIteratorM<SNumberWrapper<T>, T>*) &iterator,
																	getValueForRawValue);
														}

													// Instance methods
				T									operator[] (ItemIndex index) const
														{ return ((SNumberWrapper<T>*) getItemAt(index))->mValue; }
				TNumericArray<T>&					operator+=(T value)
														{ CArray::add(new SNumberWrapper<T>(value)); return *this; }

													// Class methods
		static	SNumberWrapper<T>*					copy(ItemRef itemRef)
														{ return new SNumberWrapper<T>(
																*((SNumberWrapper<T>*) itemRef)); }
		static	void								dispose(ItemRef itemRef)
														{
															SNumberWrapper<T>*	numberWrapper =
																						(SNumberWrapper<T>*) itemRef;
															Delete(numberWrapper);
														}

	private:
													// Class methods
		static	T									getValueForRawValue(SNumberWrapper<T>** rawValue)
														{ return (*rawValue)->mValue; }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TIArray
//	TIArray is to be used when an object needs to manage an internal arroy of objects.  TIArray provides lifecycle
//		management through simply calling Delete() (delete abc) when the object is being removed from the array or
//		the array is being deleted itself.

template <typename T> class TIArray : public CArray {
	// Methods
	public:
								// Lifecycle methods
								TIArray(CArray::ItemCount initialCapacity = 0) :
									CArray(initialCapacity, nil, dispose)
									{}

								// CArray methods
				TIArray<T>&		add(T* item)	// Pointer to mirror that this is an instance array
									{ CArray::add(item); return *this; }

				bool			contains(const T& item) const
									{ return getIndexOf(item).hasValue(); }

				T&				getAt(ItemIndex index) const
									{ return *((T*) getItemAt(index)); }
				T&				getFirst() const
									{ return *((T*) CArray::getFirst()); }
				T&				getLast() const
									{ return *((T*) CArray::getLast()); }
				OV<ItemIndex>	getIndexOf(const T& item) const
									{
										// Iterate all
										for (ItemIndex i = 0; i < getCount(); i++) {
											// Check if same
											T&	testItem = getAt(i);
											if (&item == &testItem)
												// Match
												return OV<ItemIndex>(i);
										}

										return OV<ItemIndex>();
									}

				TIArray<T>&		move(T& item, TIArray<T>& other)
									{ CArray::move(&item, other); return *this; }

				TIArray<T>&		remove(T& item)
									{
										// Check if found
										OV<ItemIndex>	index = getIndexOf(item);
										if (index.hasValue())
											// Remove
											removeAtIndex(*index);

										return *this;
									}
				TIArray<T>&		removeAll()
									{ CArray::removeAll(); return *this; }

				TIteratorD<T>	getIterator() const
									{
										// Setip
										TIteratorS<ItemRef> iterator = CArray::getIterator();

										return TIteratorD<T>((TIteratorD<T>*) &iterator);
									}

				TIArray<T>&		sort(ECompareResult (proc)(const T& item1, const T& item2, void* userData),
										void* userData = nil)
									{ CArray::sort((CompareProc) proc, userData); return *this; }

								// Instance methods
				T&				operator[] (ItemIndex index) const
									{ return *((T*) getItemAt(index)); }
				TIArray<T>&		operator+=(T* item)	// Pointer to mirror that this is an instance array
									{ return add(item); }
				TIArray<T>&		operator-=(T& item)
									{ return remove(item); }

	private:
								// Class methods
		static	void			dispose(ItemRef itemRef)
									{ T* t = (T*) itemRef; Delete(t); }
};
