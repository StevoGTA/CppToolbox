//----------------------------------------------------------------------------------------------------------------------
//	CArray.h			©2003 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CEquatable.h"
#include "CIterator.h"
#include "Compare.h"
#include "SNumber.h"
#include "TWrappers.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CArray

class CArrayInternals;
class CArray : public CEquatable {
	// Types:
	public:
		typedef			UInt32	ItemIndex;
		typedef			UInt32	ItemCount;
		typedef	const	void*	ItemRef;

	// Procs:
	public:
		typedef	void	(*ApplyProc)(ItemRef itemRef, void* userData);
		typedef	bool	(*CompareProc)(ItemRef itemRef1, ItemRef itemRef2, void* userData);
		typedef	ItemRef	(*CopyProc)(ItemRef itemRef);
		typedef	void	(*DisposeProc)(ItemRef itemRef);
		typedef bool	(*IsIncludedProc)(ItemRef itemRef, void* userData);

	// Methods
	public:
									// Lifecycle methods
		virtual						~CArray();

									// CEquatable methods
				bool				operator==(const CEquatable& other) const
										{ return equals((const CArray&) other); }

									// Instance methods
				ItemCount			getCount() const;
				bool				isEmpty() const
										{ return getCount() == 0; }

				ItemRef				getItemAt(ItemIndex itemIndex) const;

	protected:
									// Lifecycle methods
									CArray(ItemCount initialCapacity = 0, CopyProc copyProc = nil,
											DisposeProc disposeProc = nil);
									CArray(const CArray& other);

									// Instance methods
				ItemRef				copy(const ItemRef itemRef) const;

				CArray&				add(const ItemRef itemRef);
				CArray&				addFrom(const CArray& other);

				bool				contains(const ItemRef itemRef) const;

				ItemRef				getFirst() const
										{ return getItemAt(0); }
				ItemRef				getLast() const;
				OV<ItemIndex>		getIndexOf(const ItemRef itemRef) const;

				CArray&				insertAtIndex(const ItemRef itemRef, ItemIndex itemIndex);

				CArray&				detach(const ItemRef itemRef);		// Removes itemRef without calling itemDisposeProc
				CArray&				detachAtIndex(ItemIndex itemIndex);	// Removes at itemIndex without calling itemDisposeProc

				CArray&				move(const ItemRef itemRef, CArray& other);

				CArray				popFirst(ItemCount count);

				CArray&				remove(const ItemRef itemRef);
//				CArray&				removeFrom(const CArray& other);
				CArray&				removeAtIndex(ItemIndex itemIndex);
				CArray&				removeAll();

				bool				equals(const CArray& other) const;

				TIteratorS<ItemRef>	getIterator() const;

				CArray&				apply(ApplyProc applyProc, void* userData = nil);

				CArray&				sort(CompareProc compareProc, void* userData = nil);
				CArray				sorted(CompareProc compareProc, void* userData = nil) const;

				CArray				filtered(IsIncludedProc isIncludedProc, void* userData = nil) const;

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
	// Types
	public:
		typedef	bool	(*IsMatchProc)(const T& item, void* userData);

	// Methods
	public:
						// Lifecycle methods
						TArray(const TArray<T>& array) : CArray(array) {}

						// CArray methods
		bool			contains(const T& item) const
							{
								// Iterate all
								ItemCount	count = getCount();
								for (ItemIndex i = 0; i < count; i++) {
									// Check if same
									if (item == getAt(i))
										// Match
										return true;
								}

								return false;
							}

		T&				getAt(ItemIndex index) const
							{ return *((T*) getItemAt(index)); }
		T&				getFirst() const
							{ return *((T*) CArray::getFirst()); }
		T&				getLast() const
							{ return *((T*) CArray::getLast()); }
		OV<ItemIndex>	getIndexOf(const T& item) const
							{
								// Iterate all
								ItemCount	count = getCount();
								for (ItemIndex i = 0; i < count; i++) {
									// Check if same
									if (item == getAt(i))
										// Match
										return OV<ItemIndex>(i);
								}

								return OV<ItemIndex>();
							}
		OV<ItemIndex>	getIndexWhere(IsMatchProc isMatchProc, void* userData)
							{
								// Iterate all items
								ItemCount	count = getCount();
								for (ItemIndex i = 0; i < count; i++) {
									// Get item
									const	T&	item = getAt(i);

									// Call proc
									if (isMatchProc(item, userData))
										// Proc indicates to return this item
										return OV<ItemIndex>(i);
								}

								return OV<ItemIndex>();
							}

		TIteratorD<T>	getIterator() const
							{ TIteratorS<ItemRef> iterator = CArray::getIterator();
								return TIteratorD<T>((TIteratorD<T>*) &iterator); }

						// Instance methods
		OR<T>			getFirst(IsMatchProc isMatchProc, void* userData = nil) const
							{
								// Iterate all items
								ItemCount	count = getCount();
								for (ItemIndex i = 0; i < count; i++) {
									// Get item
									T&	item = getAt(i);

									// Call proc
									if (isMatchProc(item, userData))
										// Proc indicates to return this item
										return OR<T>(item);
								}

								return OR<T>();
							}

		T&				operator[] (ItemIndex index) const
							{ return *((T*) getItemAt(index)); }
		TArray<T>		operator+(const TArray<T>& other)
							{ TArray<T>	array(*this); array += other; return array; }

	protected:
						// Lifecycle methods
						TArray(CopyProc copyProc, DisposeProc disposeProc) : CArray(0, copyProc, disposeProc) {}
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TSArray (Static CArray)

template <typename T> class TSArray : public TArray<T> {
	// Methods
	public:
		// Lifecycle methods
		TSArray(const T& item, CArray::ItemCount repeatCount = 1) :
			TArray<T>(nil, nil)
			{ for (CArray::ItemIndex i = 0; i < repeatCount; i++) CArray::add(&item); }
		TSArray(const T& item1, const T& item2) :
			TArray<T>(nil, nil)
			{ CArray::add(&item1); CArray::add(&item2); }
		TSArray(const T items[], CArray::ItemCount itemCount) :
			TArray<T>(nil, nil)
			{ for (CArray::ItemIndex i = 0; i < itemCount; CArray::add(&items[i++])) ; }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TMArray (TArray which can be modified)

template <typename T> class TMArray : public TArray<T> {
	// Methods
	public:
						// CArray methods
		TMArray<T>&		add(const T& item)
							{ CArray::add(CArray::copy(&item)); return *this; }
		TMArray<T>&		addFrom(const TArray<T>& array)
							{
								// Iterate all
								ItemCount	count = array.getCount();
								for (CArray::ItemIndex i = 0; i < count; i++)
									// Add
									CArray::add(CArray::copy(array.getItemAt(i)));

								return *this;
							}

		TMArray<T>&		insertAtIndex(const T& item, CArray::ItemIndex itemIndex)
							{ CArray::insertAtIndex(new T(item), itemIndex); return *this; }

		TMArray<T>&		remove(const T& item)
							{
								// Check if found
								OV<CArray::ItemIndex>	index = TArray<T>::getIndexOf(item);
								if (index.hasValue())
									// Remove
									removeAtIndex(*index);

								return *this;
							}
		TMArray<T>&		removeFrom(const TArray<T>& array)
							{
								// Iterate all
								ItemCount	count = array.getCount();
								for (CArray::ItemIndex i = 0; i < count; i++)
									// Remove
									remove(array[i]);

								return *this;
							}
		TMArray<T>&		removeAtIndex(CArray::ItemIndex itemIndex)
							{ CArray::removeAtIndex(itemIndex); return *this; }
		TMArray<T>&		removeAll()
							{ CArray::removeAll(); return *this; }

		TMArray<T>&		sort(bool (compareProc)(const T& item1, const T& item2, void* userData), void* userData = nil)
							{ CArray::sort((CArray::CompareProc) compareProc, userData); return *this; }

						// Instance methods
		TMArray<T>&		move(const T& item, TMArray<T>& other)
							{ CArray::move(&item, other); return *this; }

		T				popFirst()
							{
								// Get first item
								T	item = TArray<T>::getFirst();

								// Remove
								removeAtIndex(0);

								return item;
							}
		OV<T>			popFirst(bool (proc)(const T& item, void* userData), void* userData = nil)
							{
								// Iterate all items
								CArray::ItemCount	count = CArray::getCount();
								for (CArray::ItemIndex i = 0; i < count; i++) {
									// Get item
									T&	item = TArray<T>::getAt(i);

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

		TMArray<T>&		operator+=(const T& item)
							{ return add(item); }
		TMArray<T>&		operator+=(const TArray<T>& other)
							{ return addFrom(other); }
		TMArray<T>&		operator-=(const T& item)
							{ return remove(item); }
		TMArray<T>&		operator-=(const TArray<T>& other)
							{ return removeFrom(other); }

	protected:
						// Lifecycle methods
						TMArray(CArray::CopyProc copyProc, CArray::DisposeProc disposeProc) :
							TArray<T>(copyProc, disposeProc) {}
						TMArray(const T& item, CArray::CopyProc copyProc, CArray::DisposeProc disposeProc) :
							TArray<T>(copyProc, disposeProc)
							{ add(item); }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TNArray (TArray where copy happens through new T())

template <typename T> class TNArray : public TMArray<T> {
	// Types
	public:
		typedef	T	(*MappingProc)(CArray::ItemRef item);

	// Methods
	public:
									// Lifecycle methods
									TNArray() : TMArray<T>((CArray::CopyProc) copy, (CArray::DisposeProc) dispose) {}
									TNArray(const T& item, ItemCount count = 1) :
										TMArray<T>((CArray::CopyProc) copy, (CArray::DisposeProc) dispose)
										{
											// Loop requested times
											for (CArray::ItemIndex i = 0; i < count; i++)
												// Add
												TMArray<T>::add(item);
										}
									TNArray(const TArray<T>& array) :
										TMArray<T>((CArray::CopyProc) copy, (CArray::DisposeProc) dispose)
										{ TMArray<T>::addFrom(array); }
									TNArray(const CArray& array, MappingProc mappingProc) :
										TMArray<T>((CArray::CopyProc) copy, (CArray::DisposeProc) dispose)
										{
											// Iterate all items
											ItemCount	count = array.getCount();
											for (CArray::ItemIndex i = 0; i < count; i++)
												// Add mapped item
												TMArray<T>::add(mappingProc(array.getItemAt(i)));
										}

									// Instance methods
				T					popFirst()
										{ return TMArray<T>::popFirst(); }
				OV<T>				popFirst(bool (proc)(const T& item, void* userData), void* userData = nil)
										{ return TMArray<T>::popFirst(proc, userData); }
				TArray<T>			popFirst(ItemCount count)
										{
											// Setup
											TNArray<T>	array;
											for (CArray::ItemIndex i = 0; i < count; i++)
												// Move first item to other array
												array += TMArray<T>::popFirst();

											return array;
										}

									// Class methods
		static	TArray<TArray<T> >	asChunksFrom(const TArray<T>& array, ItemCount chunkSize)
										{
											// Setup
											TNArray<TArray<T> >	chunks;
											ItemCount			count = array.getCount();

											// Iterate all values
											TNArray<T>	chunk;
											for (CArray::ItemIndex i = 0; i < count; i++) {
												// Add value to chunk array
												chunk += array.getAt(i);

												// Check chunk array size
												if (array.getCount() == chunkSize) {
													// At max, add to arrays and reset
													chunks += array;
													chunk = TNArray<T>();
												}
											}

											// Check if have any remaining
											if (!array.isEmpty())
												// Add to arrays
												chunks += chunk;

											return chunks;
										}

	private:
									// Class methods
		static	T*					copy(CArray::ItemRef itemRef)
										{ return new T(*((T*) itemRef)); }
		static	void				dispose(CArray::ItemRef itemRef)
										{ T* t = (T*) itemRef; Delete(t); }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TCArray (TArray where copy happens through itemRef->copy())

template <typename T> class TCArray : public TMArray<T> {
	// Methods
	public:
						// Lifecycle methods
						TCArray() : TMArray<T>((CArray::CopyProc) copy, (CArray::DisposeProc) dispose) {}
						TCArray(const T& item) :
							TMArray<T>(item, (CArray::CopyProc) copy, (CArray::DisposeProc) dispose)
							{}

	private:
						// Class methods
		static	T*		copy(CArray::ItemRef itemRef)
							{ return ((T*) itemRef)->copy(); }
		static	void	dispose(CArray::ItemRef itemRef)
							{ T* t = (T*) itemRef; Delete(t); }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TNumberArray

template <typename T> class TNumberArray : public CArray {
	// Types
	public:
		typedef	T	(*MappingProc)(CArray::ItemRef item);

	// Methods
	public:
											// Lifecycle methods
											TNumberArray(ItemCount initialCapacity = 0) :
												CArray(initialCapacity, (CArray::CopyProc) copy, dispose)
												{}
											TNumberArray(T value, ItemCount count = 1) :
												CArray(count, (CArray::CopyProc) copy, dispose)
												{
													// Loop requested times
													for (ItemIndex i = 0; i < count; i++)
														// Add
														CArray::add(new TNumber<T>(value));
												}
											TNumberArray(const CArray& array, MappingProc mappingProc) :
												CArray(0, (CArray::CopyProc) copy, dispose)
												{
													ItemCount	count = array.getCount();
													for (ItemIndex i = 0; i < count; i++)
														CArray::add(mappingProc(array.getItemAt(i)));
												}
											TNumberArray(const TNumberArray<T>& array) : CArray(array) {}

											// CArray methods
				TNumberArray<T>&			add(T value)
												{ CArray::add(new TNumber<T>(value)); return *this; }
				TNumberArray<T>&			addFrom(const TNumberArray<T>& array)
												{
													// Iterate all
													ItemCount	count = array.getCount();
													for (ItemIndex i = 0; i < count; i++)
														// Add
														CArray::add(CArray::copy(array.getItemAt(i)));

													return *this;
												}

				T							getAt(ItemIndex index) const
												{ return ((TNumber<T>*) getItemAt(index))->mValue; }

				TIteratorM<TNumber<T>, T>	getIterator() const
												{
													TIteratorS<ItemRef> iterator = CArray::getIterator();

													return TIteratorM<TNumber<T>, T>(
															(TIteratorM<TNumber<T>, T>*) &iterator,
															getValueForRawValue);
												}

											// Instance methods
				T							operator[] (ItemIndex index) const
												{ return **((TNumber<T>*) getItemAt(index)); }
				TNumberArray<T>&			operator+=(T value)
												{ CArray::add(new TNumber<T>(value)); return *this; }

											// Class methods
		static	T							getValue(ItemRef itemRef)
												{ return ((TNumber<T>*) itemRef)->mValue; }

	private:
											// Class methods
		static	TNumber<T>*					copy(ItemRef itemRef)
												{ return new TNumber<T>(*((TNumber<T>*) itemRef)); }
		static	void						dispose(ItemRef itemRef)
												{
													TNumber<T>*	numberWrapper = (TNumber<T>*) itemRef;
													Delete(numberWrapper);
												}
		static	T							getValueForRawValue(TNumber<T>** rawValue)
												{ return ***rawValue; }
};
