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
		typedef bool	(*IsMatchProc)(ItemRef itemRef, void* userData);

	protected:
		typedef	ItemRef	(*CopyProc)(ItemRef itemRef);
		typedef	void	(*DisposeProc)(ItemRef itemRef);

	// Classes
	private:
		class Internals;
		class IteratorInfo;

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
				CArray&				attach(const ItemRef itemRef);		// Adds itemRef without calling copyProc
				
				CArray&				add(const ItemRef itemRef);
				CArray&				addFrom(const CArray& other);

				bool				contains(const ItemRef itemRef) const;

				ItemRef				getFirst() const
										{ return getItemAt(0); }
				ItemRef				getLast() const;
				OV<ItemIndex>		getIndexOf(const ItemRef itemRef) const;

				CArray&				insertAtIndex(const ItemRef itemRef, ItemIndex itemIndex);

				CArray&				detach(const ItemRef itemRef);		// Removes itemRef without calling disposeProc
				CArray&				detachAtIndex(ItemIndex itemIndex);	// Removes at itemIndex without calling disposeProc

				CArray&				move(const ItemRef itemRef, CArray& other);

				CArray&				remove(const ItemRef itemRef);
				CArray&				removeAtIndex(ItemIndex itemIndex);
				CArray&				removeAll();

				bool				equals(const CArray& other) const;

				TIteratorS<ItemRef>	getIterator() const;

				CArray&				apply(ApplyProc applyProc, void* userData = nil);

				CArray&				sort(CompareProc compareProc, void* userData = nil);
				CArray				sorted(CompareProc compareProc, void* userData = nil) const;

				CArray				filtered(IsMatchProc isMatchProc, void* userData = nil) const;

				CArray&				operator=(const CArray& other);
				CArray&				operator+=(const CArray& other)
										{ return addFrom(other); }

	// Properties
	private:
		Internals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TNumberArray

template <typename T> class TNumberArray : public CArray {
	// Types
	public:
		typedef	T	(*MapProc)(CArray::ItemRef item, void* userData);

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
														TNumberArray<T>::add(value);
												}
											TNumberArray(const CArray& other, MapProc mapProc, void* userData = nil) :
												CArray(0, (CArray::CopyProc) copy, dispose)
												{
													// Map from other array
													ItemCount	count = other.getCount();
													for (ItemIndex i = 0; i < count; i++)
														TNumberArray<T>::add(mapProc(other.getItemAt(i), userData));
												}
											TNumberArray(const TNumberArray<T>& other) : CArray(other) {}

											// CArray methods
				TNumberArray<T>&			add(T value)
												{ CArray::add(new TNumber<T>(value)); return *this; }

				T							getAt(ItemIndex index) const
												{ return ((TNumber<T>*) getItemAt(index))->mValue; }

				TNumberArray<T>&			removeAll()
												{ CArray::removeAll(); return *this; }

				TIteratorM<TNumber<T>, T>	getIterator() const
												{
													TIteratorS<ItemRef> iterator = CArray::getIterator();

													return TIteratorM<TNumber<T>, T>(
															(TIteratorM<TNumber<T>, T>*) &iterator,
															getValueForRawValue);
												}

											// Instance methods
				T							popFirst()
												{
													// Get first item
													T	value = **((TNumber<T>*) CArray::getFirst());

													// Remove
													removeAtIndex(0);

													return value;
												}
				TNumberArray<T>				popFirst(ItemCount itemCount)
												{
													// Setup
													TNumberArray<T>	array;
													for (CArray::ItemIndex i = 0; (i < itemCount) && !CArray::isEmpty();
															i++)
														// Move first item to other array
														array += popFirst();

													return array;
												}

				T							operator[](ItemIndex index) const
												{ return **((TNumber<T>*) getItemAt(index)); }
				TNumberArray<T>				operator+(const TNumberArray<T>& other) const
												{
													// Setup
													TNumberArray<T>	array(*this);

													// Add other
													array += other;

													return array;
												}
				TNumberArray<T>&			operator+=(const TNumberArray<T>& other)
												{
													// Iterate other array
													ItemCount	count = other.getCount();
													for (ItemIndex i = 0; i < count; i++)
														// Add item
														CArray::add(new TNumber<T>(other[i]));

													return *this;
												}
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

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TArray
//	TArrays are used when passing an array across an API boundary.  Because object lifetime management is tricky in
//		C++, the TArray handles it all internally.  TArrays are not to be instantiated directly, but instead to be
//		done through either TNArray or TCArray.  Objects in the TArray need to be reference counted to ensure proper
//		lifetime management.

template <typename T> class TArray : public CArray {
	// Types
	public:
		typedef	void	(*ApplyProc)(T& item, void* userData);
		typedef	bool	(*IsMatchProc)(const T& item, void* userData);

	// Methods
	public:
								// Lifecycle methods
								TArray(const TArray<T>& other) : CArray(other) {}

								// CArray methods
		bool					contains(const T& item) const
									{ return getIndexOf(item).hasValue(); }

		T&						getFirst() const
									{ return *((T*) CArray::getFirst()); }
		T&						getLast() const
									{ return *((T*) CArray::getLast()); }
		OV<ItemIndex>			getIndexOf(const T& item) const
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

		TIteratorD<T>			getIterator() const
									{ TIteratorS<ItemRef> iterator = CArray::getIterator();
										return TIteratorD<T>((TIteratorD<T>*) &iterator); }

								// Instance methods
		T&						getAt(ItemIndex index) const
									{ return *((T*) getItemAt(index)); }
		OR<T>					getFirst(IsMatchProc isMatchProc, void* userData = nil) const
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
		OV<ItemIndex>			getIndexWhere(IsMatchProc isMatchProc, void* userData = nil) const
									{
										// Iterate all items
										ItemCount	count = getCount();
										for (ItemIndex i = 0; i < count; i++) {
											// Call proc
											if (isMatchProc(getAt(i), userData))
												// Match
												return OV<ItemIndex>(i);
										}

										return OV<ItemIndex>();
									}
		TNumberArray<ItemIndex>	getIndexesWhere(IsMatchProc isMatchProc, void* userData = nil) const
									{
										// Setup
										TNumberArray<ItemIndex>	itemIndexes;

										// Iterate all items
										ItemCount	count = getCount();
										for (ItemIndex i = 0; i < count; i++) {
											// Call proc
											if (isMatchProc(getAt(i), userData))
												// Match
												itemIndexes += i;
										}

										return itemIndexes;
									}

		TArray<T>				apply(ApplyProc applyProc, void* userData = nil)
									{ CArray::apply((CArray::ApplyProc) applyProc, userData); return *this; }

		T&						operator[](ItemIndex index) const
									{ return *((T*) getItemAt(index)); }
		TArray<T>				operator+(const TArray<T>& other) const
									{ TArray<T> array(*this); array += other; return array; }

	protected:
								// Lifecycle methods
								TArray(CopyProc copyProc, DisposeProc disposeProc) : CArray(0, copyProc, disposeProc) {}
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TSArray (Static TArray)

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
		TSArray(const T& item1, const T& item2, const T& item3) :
			TArray<T>(nil, nil)
			{ CArray::add(&item1); CArray::add(&item2); CArray::add(&item3); }
		TSArray(const T& item1, const T& item2, const T& item3, const T& item4) :
			TArray<T>(nil, nil)
			{ CArray::add(&item1); CArray::add(&item2); CArray::add(&item3); CArray::add(&item4); }
		TSArray(const T& item1, const T& item2, const T& item3, const T& item4, const T& item5) :
			TArray<T>(nil, nil)
			{ CArray::add(&item1); CArray::add(&item2); CArray::add(&item3); CArray::add(&item4); CArray::add(&item5); }
		TSArray(const T& item1, const T& item2, const T& item3, const T& item4, const T& item5, const T& item6) :
			TArray<T>(nil, nil)
			{ CArray::add(&item1); CArray::add(&item2); CArray::add(&item3); CArray::add(&item4); CArray::add(&item5);
					CArray::add(&item6); }
		TSArray(const T& item1, const T& item2, const T& item3, const T& item4, const T& item5, const T& item6,
				const T& item7) :
			TArray<T>(nil, nil)
			{ CArray::add(&item1); CArray::add(&item2); CArray::add(&item3); CArray::add(&item4); CArray::add(&item5);
					CArray::add(&item6); CArray::add(&item7); }
		TSArray(const T& item1, const T& item2, const T& item3, const T& item4, const T& item5, const T& item6,
				const T& item7, const T& item8) :
			TArray<T>(nil, nil)
			{ CArray::add(&item1); CArray::add(&item2); CArray::add(&item3); CArray::add(&item4); CArray::add(&item5);
					CArray::add(&item6); CArray::add(&item7); CArray::add(&item8); }
		TSArray(const T& item1, const T& item2, const T& item3, const T& item4, const T& item5, const T& item6,
				const T& item7, const T& item8, const T& item9) :
			TArray<T>(nil, nil)
			{ CArray::add(&item1); CArray::add(&item2); CArray::add(&item3); CArray::add(&item4); CArray::add(&item5);
					CArray::add(&item6); CArray::add(&item7); CArray::add(&item8); CArray::add(&item9); }
		TSArray(const T& item1, const T& item2, const T& item3, const T& item4, const T& item5, const T& item6,
				const T& item7, const T& item8, const T& item9, const T& item10) :
			TArray<T>(nil, nil)
			{ CArray::add(&item1); CArray::add(&item2); CArray::add(&item3); CArray::add(&item4); CArray::add(&item5);
					CArray::add(&item6); CArray::add(&item7); CArray::add(&item8); CArray::add(&item9);
					CArray::add(&item10); }
		TSArray(const T items[], CArray::ItemCount itemCount) :
			TArray<T>(nil, nil)
			{ for (CArray::ItemIndex i = 0; i < itemCount; CArray::add(&items[i++])) ; }
};

#define TSARRAY_FROM_C_ARRAY(T, array)	TSArray<T>(array, sizeof(array) / sizeof(T))

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TMArray (TArray which can be modified)

template <typename T> class TMArray : public TArray<T> {
	// Types
	public:
		typedef	bool	(*CompareProc)(const T& item1, const T& item2, void* userData);
		typedef	bool	(*IsMatchProc)(const T& item, void* userData);

	// Methods
	public:
						// CArray methods
		TMArray<T>&		add(const T& item)
							{ CArray::add(&item); return *this; }

		TMArray<T>&		insertAtIndex(const T& item, CArray::ItemIndex itemIndex)
							{ CArray::insertAtIndex(&item, itemIndex); return *this; }

		TMArray<T>&		move(const T& item, TMArray<T>& other)
							{ CArray::move(&item, other); return *this; }

		TMArray<T>&		remove(const T& item)
							{
								// Check if found
								OV<CArray::ItemIndex>	index = TArray<T>::getIndexOf(item);
								if (index.hasValue())
									// Remove
									removeAtIndex(*index);

								return *this;
							}
		TMArray<T>&		removeAtIndex(CArray::ItemIndex itemIndex)
							{ CArray::removeAtIndex(itemIndex); return *this; }
		TMArray<T>&		removeAll()
							{ CArray::removeAll(); return *this; }

		TMArray<T>&		sort(CompareProc compareProc, void* userData = nil)
							{ CArray::sort((CArray::CompareProc) compareProc, userData); return *this; }

						// Instance methods
		T				popFirst()
							{
								// Get first item
								T	item = TArray<T>::getFirst();

								// Remove
								removeAtIndex(0);

								return item;
							}
		OV<T>			popFirst(IsMatchProc isMatchProc, void* userData = nil)
							{
								// Iterate all items
								CArray::ItemCount	count = CArray::getCount();
								for (CArray::ItemIndex i = 0; i < count; i++) {
									// Get item
									T&	item = TArray<T>::getAt(i);

									// Call proc
									if (isMatchProc(item, userData)) {
										// Proc indicates to return this item
										OV<T>	reference(item);
										removeAtIndex(i);

										return reference;
									}
								}

								return OV<T>();
							}

		TMArray<T>&		remove(IsMatchProc isMatchProc, void* userData = nil)
							{
								// Iterate all items
								for (CArray::ItemIndex i = CArray::getCount(); i > 0; i--) {
									// Get item
									T&	item = TArray<T>::getAt(i - 1);

									// Call proc
									if (isMatchProc(item, userData))
										// Remove this item
										CArray::removeAtIndex(i - 1);
								}

								return *this;
							}
		TMArray<T>&		removeFrom(const TArray<T>& other)
							{
								// Iterate all
								ItemCount	count = other.getCount();
								for (CArray::ItemIndex i = 0; i < count; i++)
									// Remove
									remove(other[i]);

								return *this;
							}


		TMArray<T>&		operator+=(const T& item)
							{ return add(item); }
		TMArray<T>&		operator+=(const TArray<T>& other)
							{ return (TMArray<T>&) TArray<T>::addFrom(other); }
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
// MARK: - TNArray (TMArray where copy happens through new T())

template <typename T> class TNArray : public TMArray<T> {
	// Types
	public:
		typedef	T		(*MapProc)(CArray::ItemRef item, void* userData);
		typedef	bool	(*IsMatchProc)(const T& item, void* userData);

	// Methods
	public:
									// Lifecycle methods
									TNArray() : TMArray<T>((CArray::CopyProc) copy, (CArray::DisposeProc) dispose) {}
									TNArray(const T& item, ItemCount itemCount = 1) :
										TMArray<T>((CArray::CopyProc) copy, (CArray::DisposeProc) dispose)
										{
											// Loop requested times
											for (CArray::ItemIndex i = 0; i < itemCount; i++)
												// Add
												TMArray<T>::add(item);
										}
									TNArray(const TArray<T>& other) :
										TMArray<T>((CArray::CopyProc) copy, (CArray::DisposeProc) dispose)
										{ TMArray<T>::addFrom(other); }
									TNArray(const CArray& other, MapProc mapProc, void* userData = nil) :
										TMArray<T>((CArray::CopyProc) copy, (CArray::DisposeProc) dispose)
										{
											// Iterate all items
											ItemCount	itemCount = other.getCount();
											for (CArray::ItemIndex i = 0; i < itemCount; i++)
												// Add mapped item
												TMArray<T>::add(mapProc(other.getItemAt(i), userData));
										}

									// CArray methods
				TNArray<T>			filtered(IsMatchProc isMatchProc, void* userData = nil)
										{
											// Setup
											TNArray<T>	array;
											ItemCount	itemCount = CArray::getCount();
											for (CArray::ItemIndex i = 0; i < itemCount; i++) {
												// Check if match
												const	T&	item = (*this)[i];
												if (isMatchProc(item, userData))
													// Not a match
													array.add(item);
											}

											return array;
										}

									// Instance methods
				T					popFirst()
										{ return TMArray<T>::popFirst(); }
				TArray<T>			popFirst(ItemCount itemCount)
										{
											// Setup
											TNArray<T>	array;
											for (CArray::ItemIndex i = 0; (i < itemCount) && !CArray::isEmpty(); i++)
												// Move first item to other array
												array += TMArray<T>::popFirst();

											return array;
										}
				OV<T>				popFirst(IsMatchProc isMatchProc, void* userData = nil)
										{ return TMArray<T>::popFirst(isMatchProc, userData); }

									// Class methods
		static	TArray<TArray<T> >	asChunksFrom(const TArray<T>& other, ItemCount chunkSize)
										{
											// Setup
											TNArray<TArray<T> >	chunks;
											ItemCount			itemCount = other.getCount();

											// Iterate all values
											TNArray<T>	chunk;
											for (CArray::ItemIndex i = 0; i < itemCount; i++) {
												// Add value to chunk
												chunk += other.getAt(i);

												// Check chunk item count
												if (chunk.getCount() == chunkSize) {
													// At max, add to chunks and reset
													chunks += chunk;
													chunk = TNArray<T>();
												}
											}

											// Check if have any remaining
											if (!chunk.isEmpty())
												// Add to chunks
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
