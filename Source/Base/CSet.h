//----------------------------------------------------------------------------------------------------------------------
//	CSet.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CHashing.h"
#include "CIterator.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSet

class CSetInternals;
class CSet {
	// Types
	public:
		typedef	UInt32	ItemCount;

	// Procs
	public:
		typedef	void	(*ApplyProc)(CHashable& hashable, void* userData);

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
										CSet(bool ownsItems = false);
										CSet(const CSet& other);

										// Instance methods
				CSet&					add(const CHashable* hashable);

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

template <typename T> class TSet : public CSet {
	// Methods
	public:
						// Lifecycle methods
						TSet() : CSet(true) {}
						TSet(const TSet<T>& other) : CSet(other) {}

						// Instance methods
		TSet<T>&		add(const T& item)
							{ CSet::add(new T(item)); return *this; }
		TSet<T>&		add(const T* item)
							{ CSet::add(new T(*item)); return *this; }
		TSet<T>&		addFrom(const TSet<T>& other)
							{
								// Iterate all
								for (TIteratorS<T> iterator = other.getIterator(); iterator.hasValue();
										iterator.advance())
									// Add
									CSet::add(new T(iterator.getValue()));

								return *this;
							}

		TSet<T>&		remove(const T item)
							{ CSet::remove(item); return *this; }
		TSet<T>&		removeFrom(const TSet<T>& other)
							{
								// Iterate all
								for (TIteratorS<T> iterator = other.getIterator(); iterator.hasValue();
										iterator.advance())
									// Add
									CSet::remove(T(iterator.getValue()));

								return *this;
							}
		TSet<T>&		removeAll()
							{ CSet::removeAll(); return *this; }

		TSet<T>&		apply(void (proc)(const T item, void* userData), void* userData = nil)
							{ CSet::apply((ApplyProc) proc, userData); return *this; }

		TIteratorS<T>	getIterator() const
							{ TIteratorS<CHashable> iterator = CSet::getIterator();
									return TIteratorS<T>((TIteratorS<T>*) &iterator); }

		TSet<T>&		operator=(const TSet<T>& other)
							{ CSet::operator=(other); return *this; }
		TSet<T>&		operator+=(const T* item)
							{ return add(item); }
		TSet<T>&		operator+=(const TSet<T>& other)
							{ return addFrom(other); }
		TSet<T>&		operator-=(const T item)
							{ return remove(item); }
		TSet<T>&		operator-=(const TSet<T>& other)
							{ return removeFrom(other); }
};
