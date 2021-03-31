//----------------------------------------------------------------------------------------------------------------------
//	TLockingDictionary.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDictionary.h"
#include "ConcurrencyPrimitives.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: TNLockingDictionary

template <typename T> class TNLockingDictionary : public CDictionary {
	// Procs
	public:
		typedef	OI<T>	(*UpdateProc)(const OR<T>& currentValue, void* userData);
	
	// Methods
	public:
								// Lifecycle methods
								TNLockingDictionary(CDictionary::ItemEqualsProc itemEqualsProc = nil) :
									CDictionary((CDictionary::ItemCopyProc) copy, dispose, itemEqualsProc)
									{}

								// Instance methods
				const	OR<T>	get(const CString& key) const
									{
										// Get
										mLock.lockForReading();
										OV<CDictionary::ItemRef>	itemRef = CDictionary::getItemRef(key);
										mLock.unlockForReading();

										return itemRef.hasValue() ? OR<T>(*((T*) itemRef.getValue())) : OR<T>();
									}
						void	set(const CString& key, const T& item)
									{
										// Store
										mLock.lockForWriting();
										CDictionary::set(key, new T(item));
										mLock.unlockForWriting();
									}
						void	update(const CString& key, UpdateProc updateProc, void* userData)
									{
										// Update
										mLock.lockForWriting();
										OV<CDictionary::ItemRef>	itemRef = CDictionary::getItemRef(key);
										OI<T>	updatedValue =
														updateProc(
																itemRef.hasValue() ?
																		OR<T>(*((T*) itemRef.getValue())) : OR<T>(),
																userData);
										if (updatedValue.hasInstance())
											// Store
											CDictionary::set(key, new T(*updatedValue));
										else
											// Remove
											CDictionary::remove(key);
										mLock.unlockForWriting();
									}
						void	remove(const CString& key)
									{
										// Remove
										mLock.lockForWriting();
										CDictionary::remove(key);
										mLock.unlockForWriting();
									}

				const	OR<T>	operator[](const CString& key) const
									{ return get(key); }

	private:
								// Class methods
		static			T*		copy(CDictionary::ItemRef itemRef)
									{ return new T(*((T*) itemRef)); }
		static			void	dispose(CDictionary::ItemRef itemRef)
									{ T* t = (T*) itemRef; Delete(t); }

	// Properties
	private:
		CReadPreferringLock	mLock;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TNLockingArrayDictionary

template <typename T> class TNLockingArrayDictionary : public CDictionary {
	// Methods
	public:
										// Lifecycle methods
										TNLockingArrayDictionary() :
											CDictionary((CDictionary::ItemCopyProc) copy, dispose, nil)
											{}

										// Instance methods
				const	OR<TArray<T> >	get(const CString& key) const
											{
												// Get itemRef
												mLock.lockForReading();
												OV<CDictionary::ItemRef>	itemRef = CDictionary::getItemRef(key);
												mLock.unlockForReading();

												return itemRef.hasValue() ?
														OR<TArray<T> >(*((TArray<T>*) itemRef.getValue())) :
														OR<TArray<T> >();
											}
						void			add(const CString& key, const T& item)
											{
												// Setup
												mLock.lockForWriting();

												// Retrieve current value
												OV<CDictionary::ItemRef>	itemRef = CDictionary::getItemRef(key);
												TNArray<T>	array(
																	itemRef.hasValue() ?
																			*(TNArray<T>*) itemRef.getValue() :
																			TNArray<T>());

												// Add
												array.add(item);

												// Set
												set(key, &array);

												// All done
												mLock.unlockForWriting();
											}

	private:
										// Class methods
		static			TArray<T>*		copy(CDictionary::ItemRef itemRef)
											{ return new TNArray<T>(*((TArray<T>*) itemRef)); }
		static			void			dispose(CDictionary::ItemRef itemRef)
											{ TArray<T>* t = (TArray<T>*) itemRef; Delete(t); }

	// Properties
	private:
		CReadPreferringLock	mLock;
};
