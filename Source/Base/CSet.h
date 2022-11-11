//----------------------------------------------------------------------------------------------------------------------
//	CSet.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CArray.h"
#include "CHashing.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSet

class CSetInternals;
class CSet {
	// Types
	public:
		typedef	UInt32	ItemCount;

	// Procs
	public:
		typedef	void		(*ApplyProc)(CHashable& hashable, void* userData);
		typedef	CHashable*	(*CopyProc)(const CHashable& hashable);
		typedef	void		(*DisposeProc)(const CHashable& hashable);

	// Methods
	public:
										// Lifecycle methods
		virtual							~CSet();

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
				CHashable&				copy(const CHashable& hashable) const;

				CSet&					insert(const CHashable& hashable);

				CSet&					remove(const CHashable& hashable);
				CSet&					removeAll();

				TIteratorS<CHashable>	getIterator() const;
				CSet&					apply(ApplyProc applyProc, void* userData = nil);

				CSet&					operator=(const CSet& other);

	// Properties
	private:
		CSetInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TSet
//	TSets are used when passing a set across an API boundary.  Because object lifetime management is tricky in
//		C++, the TSet handles it all internally.  TSets are not to be instantiated directly, but instead to be
//		done through either TNSet or TCSet.  Objects in the TSet need to be reference counted to ensure proper
//		lifetime management.

template <typename T> class TSet : public CSet {
	// Types
	public:
		typedef	bool	(*IsIncludedProc)(const T& item, void* userData);

	// Methods
	public:
						// Lifecycle methods
						TSet(const TSet<T>& other) : CSet(other) {}

						// Instance methods
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

		TIteratorS<T>	getIterator() const
							{ TIteratorS<CHashable> iterator = CSet::getIterator();
									return TIteratorS<T>((TIteratorS<T>*) &iterator); }

	protected:
						// Lifecycle methods
						TSet(CopyProc copyProc, DisposeProc disposeProc) : CSet(copyProc, disposeProc) {}
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TSSet (Static TSet)

template <typename T> class TSSet : public TSet<T> {
	// Methods
	public:
		// Lifecycle methods
		TSSet(const T& item) : TSet<T>(nil, nil) { CSet::insert(item); }
		TSSet(const T items[], CArray::ItemCount itemCount) :
			TSet<T>(nil, nil)
			{ for (CArray::ItemIndex i = 0; i < itemCount; CSet::insert(items[i++])) ; }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TMSet (TSet which can be modified)

template <typename T> class TMSet : public TSet<T> {
	// Methods
	public:
						// CSet methods
		TMSet<T>&		insert(const T& item)
							{ CSet::insert(item); return *this; }
		TMSet<T>&		remove(const T item)
							{ CSet::remove(item); return *this; }
		TMSet<T>&		removeAll()
							{ CSet::removeAll(); return *this; }

						// Instance methods
		TMSet<T>&		insertFrom(const TArray<T>& array)
							{
								// Iterate all
								for (TIteratorD<T> iterator = array.getIterator(); iterator.hasValue();
										iterator.advance())
									// Insert
									CSet::insert(*iterator);

								return *this;
							}
		TMSet<T>&		insertFrom(const TSet<T>& other)
							{
								// Iterate all
								for (TIteratorS<T> iterator = other.getIterator(); iterator.hasValue();
										iterator.advance())
									// Insert
									CSet::insert(*iterator);

								return *this;
							}
		TMSet<T>&		removeFrom(const TArray<T>& array)
							{
								// Iterate all
								for (TIteratorD<T> iterator = array.getIterator(); iterator.hasValue();
										iterator.advance())
									// Remove
									CSet::remove(T(*iterator));

								return *this;
							}
		TMSet<T>&		removeFrom(const TSet<T>& other)
							{
								// Iterate all
								for (TIteratorS<T> iterator = other.getIterator(); iterator.hasValue();
										iterator.advance())
									// Remove
									CSet::remove(T(*iterator));

								return *this;
							}

//		TMSet<T>&		apply(void (proc)(const T item, void* userData), void* userData = nil)
//							{ CSet::apply((ApplyProc) proc, userData); return *this; }

		TMSet<T>&		operator=(const TSet<T>& other)
							{ CSet::operator=(other); return *this; }
		TMSet<T>&		operator+=(const T& item)
							{ return insert(item); }
		TMSet<T>&		operator+=(const TSet<T>& other)
							{ return insertFrom(other); }
		TMSet<T>&		operator-=(const T& item)
							{ return remove(item); }
		TMSet<T>&		operator-=(const TSet<T>& other)
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
						TNSet(const TSet<T>& other, typename TSet<T>::IsIncludedProc isIncludedProc, void* userData) :
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

	private:
						// Class methods
		static	T*		copy(const CHashable& hashable)
							{ return new T(*((T*) &hashable)); }
		static	void	dispose(const CHashable& hashable)
							{ T* t = (T*) &hashable; Delete(t); }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TNumberSet

template <typename T> class TNumberSet : public TNSet<TNumber<T> > {
	// Methods
	public:
						// Instance methods
		bool			contains(T value) const
							{ return TNSet<TNumber<T> >::contains(TNumber<T>(value)); }

		TNumberSet<T>&	insert(T value)
							{ TNSet<TNumber<T> >::insert(TNumber<T>(value)); return *this; }

		TNumberSet<T>&	operator+=(T value)
							{ return insert(value); }
};
