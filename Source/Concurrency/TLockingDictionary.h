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
		typedef	OV<T>	(*UpdateProc)(const OR<T>& currentValue, void* userData);
	
	// Methods
	public:
								// Lifecycle methods
								TNLockingDictionary(SValue::OpaqueEqualsProc opaqueEqualsProc = nil) :
									CDictionary((SValue::OpaqueCopyProc) copy, dispose, opaqueEqualsProc)
									{}

								// Instance methods
				const	OR<T>	get(const CString& key) const
									{
										// Get
										mLock.lockForReading();
										OV<SValue::Opaque>	opaque = CDictionary::getOpaque(key);
										mLock.unlockForReading();

										return opaque.hasValue() ? OR<T>(*((T*) opaque.getValue())) : OR<T>();
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
										OV<SValue::Opaque>	opaque = CDictionary::getOpaque(key);
										OV<T>	updatedValue =
														updateProc(
																opaque.hasValue() ?
																		OR<T>(*((T*) opaque.getValue())) : OR<T>(),
																userData);
										if (updatedValue.hasValue())
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
		static			T*		copy(SValue::Opaque opaque)
									{ return new T(*((T*) opaque)); }
		static			void	dispose(SValue::Opaque opaque)
									{ T* t = (T*) opaque; Delete(t); }

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
											CDictionary((SValue::OpaqueCopyProc) copy, dispose, nil)
											{}

										// Instance methods
				const	OR<TArray<T> >	get(const CString& key) const
											{
												// Get opaque
												mLock.lockForReading();
												OV<SValue::Opaque>	opaque = CDictionary::getOpaque(key);
												mLock.unlockForReading();

												return opaque.hasValue() ?
														OR<TArray<T> >(*((TArray<T>*) opaque.getValue())) :
														OR<TArray<T> >();
											}
						void			add(const CString& key, const T& item)
											{
												// Setup
												mLock.lockForWriting();

												// Retrieve current value
												OV<SValue::Opaque>	opaque = CDictionary::getOpaque(key);
												TNArray<T>	array(
																	opaque.hasValue() ?
																			*(TNArray<T>*) opaque.getValue() :
																			TNArray<T>());

												// Add
												array.add(item);

												// Set
												set(key, new TNArray<T>(array));

												// All done
												mLock.unlockForWriting();
											}

	private:
										// Class methods
		static			TArray<T>*		copy(SValue::Opaque opaque)
											{ return new TNArray<T>(*((TArray<T>*) opaque)); }
		static			void			dispose(SValue::Opaque opaque)
											{ TArray<T>* t = (TArray<T>*) opaque; Delete(t); }

	// Properties
	private:
		CReadPreferringLock	mLock;
};
