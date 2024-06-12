//----------------------------------------------------------------------------------------------------------------------
//	TLockingDictionary.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDictionary.h"
#include "ConcurrencyPrimitives.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: TLockingDictionary
template <typename T> class TLockingDictionary : public CDictionary {
	// Methods
	public:
								// Lifecycle methods
								TLockingDictionary(SValue::OpaqueCopyProc opaqueCopyProc = nil,
										SValue::OpaqueEqualsProc opaqueEqualsProc = nil,
										SValue::OpaqueDisposeProc opaqueDisposeProc = nil) :
									CDictionary(opaqueCopyProc, opaqueEqualsProc, opaqueDisposeProc)
									{}
								TLockingDictionary(const Procs& procs) : CDictionary(procs) {}
								TLockingDictionary(const TLockingDictionary<T>& other) : CDictionary(other) {}

								// CDictionary methods
				KeyCount		getKeyCount() const
									{
										// Get
										mLock.lockForReading();
										KeyCount	keyCount = CDictionary::getKeyCount();
										mLock.unlockForReading();

										return keyCount;
									}
				TSet<CString>	getKeys() const
									{
										// Get
										mLock.lockForReading();
										TSet<CString>	keys = CDictionary::getKeys();
										mLock.unlockForReading();

										return keys;
									}

				bool			contains(const CString& key) const
									{
										// Get
										mLock.lockForReading();
										bool	contains = CDictionary::contains(key);
										mLock.unlockForReading();

										return contains;
									}

		const	OR<T>			get(const CString& key) const
									{
										// Get
										mLock.lockForReading();
										OV<SValue::Opaque>	opaque = CDictionary::getOpaque(key);
										mLock.unlockForReading();

										return opaque.hasValue() ? OR<T>(*((T*) *opaque)) : OR<T>();
									}

		const	OR<T>			operator[](const CString& key) const
									{ return get(key); }

	// Properties
	protected:
		CReadPreferringLock	mLock;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TNLockingDictionary

template <typename T> class TNLockingDictionary : public TLockingDictionary<T> {
	// Procs
	public:
		typedef	OV<T>	(*UpdateProc)(const OR<T>& currentValue, void* userData);
	
	// Methods
	public:
										// Lifecycle methods
										TNLockingDictionary(SValue::OpaqueEqualsProc opaqueEqualsProc = nil) :
											TLockingDictionary<T>((SValue::OpaqueCopyProc) copy, opaqueEqualsProc,
													dispose)
											{}

										// Instance methods
				void					set(const CString& key, const T& item)
											{
												// Store
												TLockingDictionary<T>::mLock.lockForWriting();
												CDictionary::set(key, new T(item));
												TLockingDictionary<T>::mLock.unlockForWriting();
											}

				void					update(const CString& key, UpdateProc updateProc, void* userData)
											{
												// Update
												TLockingDictionary<T>::mLock.lockForWriting();
												OV<SValue::Opaque>	opaque = CDictionary::getOpaque(key);
												OV<T>				updatedValue =
																			updateProc(
																					opaque.hasValue() ?
																							OR<T>(*((T*) *opaque)) :
																							OR<T>(),
																					userData);
												if (updatedValue.hasValue())
													// Store
													CDictionary::set(key, new T(*updatedValue));
												else
													// Remove
													CDictionary::remove(key);
												TLockingDictionary<T>::mLock.unlockForWriting();
											}

				void					remove(const CString& key)
											{
												// Remove
												TLockingDictionary<T>::mLock.lockForWriting();
												CDictionary::remove(key);
												TLockingDictionary<T>::mLock.unlockForWriting();
											}
				void					remove(const TArray<CString>& keys)
											{
												// Remove
												TLockingDictionary<T>::mLock.lockForWriting();
												CDictionary::remove(keys);
												TLockingDictionary<T>::mLock.unlockForWriting();
											}
				void					remove(const TSet<CString>& keys)
											{
												// Remove
												TLockingDictionary<T>::mLock.lockForWriting();
												CDictionary::remove(keys);
												TLockingDictionary<T>::mLock.unlockForWriting();
											}

				TNLockingDictionary<T>&	operator+=(const TDictionary<T>& other)
											{
												// Setup
												TSet<CString>	keys = other.getKeys();

												// Update
												TLockingDictionary<T>::mLock.lockForWriting();
												for (TIteratorS<CString> iterator = keys.getIterator();
														iterator.hasValue(); iterator.advance())
													// Update
													CDictionary::set(*iterator, new T(*other[*iterator]));
												TLockingDictionary<T>::mLock.unlockForWriting();

												return *this;
											}

	private:
										// Class methods
		static	T*						copy(SValue::Opaque opaque)
											{ return new T(*((T*) opaque)); }
		static	void					dispose(SValue::Opaque opaque)
											{ T* t = (T*) opaque; Delete(t); }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TNLockingArrayDictionary

template <typename T> class TNLockingArrayDictionary : public TNLockingDictionary<TNArray<T> > {
	// Methods
	public:
				// Instance methods
		void	add(const CString& key, const T& item)
					{
						// Lock
						TLockingDictionary<TNArray<T> >::mLock.lockForWriting();

						// Update
						OV<SValue::Opaque>	opaque = CDictionary::getOpaque(key);
						if (opaque.hasValue())
							// Already have array
							*((TNArray<T>*) *opaque) += item;
						else
							// First one
							CDictionary::set(key, new TNArray<T>(item));

						// Unlock
						TLockingDictionary<TNArray<T> >::mLock.unlockForWriting();
					}
		void	remove(const CString& key, const T& item)
					{
						// Lock
						TLockingDictionary<TNArray<T> >::mLock.lockForWriting();

						// Update
						OV<SValue::Opaque>	opaque = CDictionary::getOpaque(key);
						if (opaque.hasValue()) {
							// Already have array
							TNArray<T>&	array = *((TNArray<T>*) *opaque);
							array -= item;

							// Check if empty
							if (array.isEmpty())
								// Array is now empty!
								CDictionary::remove(key);
						}

						// Unlock
						TLockingDictionary<TNArray<T> >::mLock.unlockForWriting();
					}
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TNLockingKeyConvertibleDictionary

template <typename K, typename T> class TNLockingKeyConvertibleDictionary : public TNLockingDictionary<T> {
	// Methods
	public:
						// Instance methods
		const	OR<T>	get(K key) const
							{ return TNLockingDictionary<T>::get(CString(key)); }
				void	set(K key, const T& item)
							{ TNLockingDictionary<T>::set(CString(key), item); }

				void	remove(K key)
							{ TNLockingDictionary<T>::remove(CString(key)); }

		const	OR<T>	operator[](K key) const
							{ return get(key); }
};
