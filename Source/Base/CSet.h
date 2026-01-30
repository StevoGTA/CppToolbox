//----------------------------------------------------------------------------------------------------------------------
//	CSet.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CArray.h"
#include "CHashable.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSet

class CSet {
	// Types
	public:
		typedef	UInt32	ItemCount;

	// Procs
	public:
		typedef	void		(*ApplyProc)(CHashable& hashable, void* userData);

	protected:
		typedef	CHashable*	(*CopyProc)(const CHashable& hashable);
		typedef	void		(*DisposeProc)(const CHashable* hashable);

	// Classes
	private:
		class Internals;

	// Structs
	private:
		struct IteratorInfo;

	// Methods
	public:
												// Lifecycle methods
		virtual									~CSet();

												// Instance methods
						ItemCount				getCount() const;
						bool					isEmpty() const
													{ return getCount() == 0; }

						bool					contains(const CHashable& hashable) const;

	protected:
												// Lifecycle methods
												CSet(CopyProc copyProc = nil, DisposeProc disposeProc = nil);
												CSet(const CSet& other);

												// Instance methods
						CSet&					insert(const CHashable& hashable);

						CSet&					remove(const CHashable& hashable);
						CSet&					removeAll();

				const	OR<CHashable>			getAny() const;
						TIteratorS<CHashable>	getIterator() const;

						CSet&					apply(ApplyProc applyProc, void* userData = nil);

						CSet&					operator=(const CSet& other);

	// Properties
	private:
		Internals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TSet
//	TSets are used when passing a set across an API boundary.  Because object lifetime management is tricky in
//		C++, the TSet handles it all internally.  TSets are not to be instantiated directly, but instead to be
//		done through TNSet.  Objects in the TSet need to be reference counted to ensure proper lifetime management.

template <typename T> class TSet : public CSet {
	// Types
	public:
		typedef	bool	(*IsIncludedProc)(const T& item, void* userData);

	// Methods
	public:
										// Lifecycle methods
										TSet(const TSet<T>& other) : CSet(other) {}

										// Instance methods
						bool			contains(const T& item) const
											{ return CSet::contains(item); }
						bool			intersects(const TSet<T>& other) const
											{
												// Iterate values
												for (TIteratorS<T> iterator = other.getIterator(); iterator.hasValue();
														iterator.advance()) {
													// Check if have value
													if (CSet::contains(*iterator))
														// Has value
														return true;
												}

												return false;
											}

				const	OR<T>			getAny() const
											{
												// Get item
												const	OR<CHashable>	item = CSet::getAny();

												return item.hasReference() ? OR<T>((T&) *item) : OR<T>();
											}
						TIteratorS<T>	getIterator() const
											{ TIteratorS<CHashable> iterator = CSet::getIterator();
													return *((TIteratorS<T>*) &iterator); }

										// Class methods
		static			bool			containsItem(const T& item, TSet<T>* set)
											{ return set->contains(item); }
		static			bool			doesNotContainItem(const T& item, TSet<T>* set)
											{ return !set->contains(item); }

	protected:
										// Lifecycle methods
										TSet(CopyProc copyProc, DisposeProc disposeProc) :
											CSet(copyProc, disposeProc)
											{}
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TSSet (Static TSet)

template <typename T> class TSSet : public TSet<T> {
	// Methods
	public:
		// Lifecycle methods
		TSSet(const T& item) : TSet<T>(nil, nil) { CSet::insert(item); }
		TSSet(const T items[], UInt32 itemCount) :
			TSet<T>(nil, nil)
			{ for (UInt32 i = 0; i < itemCount; CSet::insert(items[i++])) ; }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TMSet (TSet which can be modified)

template <typename T> class TMSet : public TSet<T> {
	// Methods
	public:
					// CSet methods
		TMSet<T>&	insert(const T& item)
						{ CSet::insert(item); return *this; }
		TMSet<T>&	remove(const T item)
						{ CSet::remove(item); return *this; }
		TMSet<T>&	removeAll()
						{ CSet::removeAll(); return *this; }

					// Instance methods
		TMSet<T>&	insertFrom(const TArray<T>& array)
						{
							// Iterate all
							for (TIteratorD<T> iterator = array.getIterator(); iterator.hasValue();
									iterator.advance())
								// Insert
								CSet::insert(*iterator);

							return *this;
						}
		TMSet<T>&	insertFrom(const TSet<T>& other)
						{
							// Iterate all
							for (TIteratorS<T> iterator = other.getIterator(); iterator.hasValue();
									iterator.advance())
								// Insert
								CSet::insert(*iterator);

							return *this;
						}
		TMSet<T>&	removeFrom(const TArray<T>& array)
						{
							// Iterate all
							for (TIteratorD<T> iterator = array.getIterator(); iterator.hasValue();
									iterator.advance())
								// Remove
								CSet::remove(*iterator);

							return *this;
						}
		TMSet<T>&	removeFrom(const TSet<T>& other)
						{
							// Iterate all
							for (TIteratorS<T> iterator = other.getIterator(); iterator.hasValue();
									iterator.advance())
								// Remove
								CSet::remove(*iterator);

							return *this;
						}

		TMSet<T>&	operator=(const TSet<T>& other)
						{ CSet::operator=(other); return *this; }
		TMSet<T>&	operator+=(const T& item)
						{ return insert(item); }
		TMSet<T>&	operator+=(const TArray<T>& array)
						{ return insertFrom(array); }
		TMSet<T>&	operator+=(const TSet<T>& other)
						{ return insertFrom(other); }
		TMSet<T>&	operator-=(const T& item)
						{ return remove(item); }
		TMSet<T>&	operator-=(const TArray<T>& array)
						{ return removeFrom(array); }
		TMSet<T>&	operator-=(const TSet<T>& other)
						{ return removeFrom(other); }

	protected:
					// Lifecycle methods
					TMSet(CSet::CopyProc copyProc, CSet::DisposeProc disposeProc) :
						TSet<T>(copyProc, disposeProc)
						{}
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TNSet (TArray where copy happens through new T())

template <typename T> class TNSet : public TMSet<T> {
	// Types
	public:
		typedef	T	(*ArrayMapProc)(CArray::ItemRef);

	// Methods
	public:
							// Lifecycle methods
							TNSet() : TMSet<T>((CSet::CopyProc) copy, (CSet::DisposeProc) dispose) {}
							TNSet(const T& item) :
								TMSet<T>((CSet::CopyProc) copy, (CSet::DisposeProc) dispose)
								{ CSet::insert(item); }
							TNSet(const TArray<T>& array) :
								TMSet<T>((CSet::CopyProc) copy, (CSet::DisposeProc) dispose)
								{
									// Iterate all
									for (TIteratorD<T> iterator = array.getIterator(); iterator.hasValue();
											iterator.advance())
										// Insert
										CSet::insert(*iterator);
								}
							TNSet(const CArray& array, ArrayMapProc arrayMapProc) :
								TMSet<T>((CSet::CopyProc) copy, (CSet::DisposeProc) dispose)
								{
									// Iterate all items
									ItemCount	count = array.getCount();
									for (CArray::ItemIndex i = 0; i < count; i++)
										// Insert mapped item
										CSet::insert(arrayMapProc(array.getItemAt(i)));
								}
							TNSet(const TSet<T>& other) :
								TMSet<T>((CSet::CopyProc) copy, (CSet::DisposeProc) dispose)
								{ TNSet<T>::insertFrom(other); }
							TNSet(const TSet<T>& other, typename TSet<T>::IsIncludedProc isIncludedProc,
									void* userData) :
								TMSet<T>((CSet::CopyProc) copy, (CSet::DisposeProc) dispose)
								{
									// Iterate all items
									for (TIteratorS<T> iterator = other.getIterator(); iterator.hasValue();
											iterator.advance()) {
										// Call proc
										if (isIncludedProc(*iterator, userData))
											// Insert
											CSet::insert(*iterator);
									}
								}

				TNArray<T>	getArray() const
								{
									// Setup
									TNArray<T>	array;

									// Iterate all hashables
									for (TIteratorS<T> iterator = TSet<T>::getIterator(); iterator.hasValue();
											iterator.advance())
										// Add
										array += *iterator;

									return array;
								}

				TNSet<T>	getIntersection(const TSet<T>& other) const
								{
									// Setup
									TNSet<T>	items;

									// Iterate values
									for (TIteratorS<T> iterator = other.getIterator(); iterator.hasValue();
											iterator.advance()) {
										// Check if have value
										if (CSet::contains(*iterator))
											// Has value
											items += *iterator;
									}

									return items;
								}
				TNSet<T>	getDifference(const TSet<T>& other) const
								{
									// Setup
									TNSet<T>	items;

									// Iterate values
									for (TIteratorS<T> iterator = TSet<T>::getIterator(); iterator.hasValue();
											iterator.advance()) {
										// Check if have value
										if (!other.contains(*iterator))
											// Has value
											items += *iterator;
									}

									return items;
								}

				TMSet<T>	operator+(const TSet<T>& other) const
								{ TNSet<T>	set(*this); set += other; return set; }

	private:
							// Class methods
		static	T*			copy(const CHashable& hashable)
								{ return new T(*((T*) &hashable)); }
		static	void		dispose(const CHashable* hashable)
								{ T* t = (T*) hashable; Delete(t); }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TNumberSet

template <typename T> class TNumberSet : public TMSet<TNumber<T> > {
	// Types
	public:
		typedef	T	(*ArrayMapProc)(CArray::ItemRef item);

	// Methods
	public:
								// Lifecycle methods
								TNumberSet() : TMSet<TNumber<T> >((CSet::CopyProc) copy, (CSet::DisposeProc) dispose) {}
								TNumberSet(T value) :
									TMSet<TNumber<T> >((CSet::CopyProc) copy, (CSet::DisposeProc) dispose)
									{ TMSet<TNumber<T> >::insert(TNumber<T>(value)); }
								TNumberSet(const CArray& array, ArrayMapProc arrayMapProc) :
									TMSet<TNumber<T> >((CSet::CopyProc) copy, (CSet::DisposeProc) dispose)
									{
										// Iterate items
										ItemCount	count = array.getCount();
										for (CArray::ItemIndex i = 0; i < count; i++)
											// Insert
											insert(arrayMapProc(array.getItemAt(i)));
									}

								// Instance methods
				bool			contains(T value) const
									{ return TMSet<TNumber<T> >::contains(TNumber<T>(value)); }

				TNumberSet<T>&	insert(T value)
									{ TMSet<TNumber<T> >::insert(TNumber<T>(value)); return *this; }
				TNumberSet<T>&	remove(T value)
									{ TMSet<TNumber<T> >::remove(TNumber<T>(value)); return *this; }

				TNumberArray<T>	getNumberArray() const
									{
										// Setup
										TNumberArray<T>	array;

										// Iterate all hashables
										for (TIteratorS<TNumber<T> > iterator = TMSet<TNumber<T> >::getIterator();
												iterator.hasValue(); iterator.advance())
											// Add
											array += **iterator;

										return array;
									}

				TNumberSet<T>&	operator+=(T value)
									{ return insert(value); }
				TNumberSet<T>&	operator-=(T value)
									{ return remove(value); }

	private:
								// Class methods
		static	TNumber<T>*		copy(const CHashable& hashable)
									{ return new TNumber<T>(*((TNumber<T>*) &hashable)); }
		static	void			dispose(const CHashable* hashable)
									{ TNumber<T>* t = (TNumber<T>*) hashable; Delete(t); }
};
