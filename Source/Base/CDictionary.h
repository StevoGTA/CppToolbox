//----------------------------------------------------------------------------------------------------------------------
//	CDictionary.h			©2007 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CSet.h"
#include "SValue.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CDictionary

class CDictionaryInternals;
class CDictionary : public CEquatable {
	// Types
	public:
		typedef	UInt32	KeyCount;

	// Structs
	public:
		struct Item {
			// Procs
			typedef bool	(*IncludeProc)(const Item& item, void* userData);

			// Lifecycle methods
			Item(const CString& key, const SValue& value) : mKey(key), mValue(value) {}
			Item(const CString& key, const SValue& value, SValue::OpaqueCopyProc opaqueCopyProc) :
				mKey(key), mValue(value, opaqueCopyProc)
				{}
			Item(const Item& other, SValue::OpaqueCopyProc opaqueCopyProc) :
				mKey(other.mKey), mValue(other.mValue, opaqueCopyProc)
				{}

			// Properties
			CString	mKey;
			SValue	mValue;
		};

		struct Procs {
			// Procs
			typedef	KeyCount	(*GetKeyCountProc)(void* userData);
			typedef	CString		(*GetKeyAtIndexProc)(UInt32 index, void* userData);
			typedef	OR<SValue>	(*GetValueProc)(const CString& key, void* userData);
			typedef	void		(*SetProc)(const CString& key, const SValue& value, void* userData);
			typedef	void		(*RemoveKeysProc)(const TSet<CString>& keys, void* userData);
			typedef	void		(*RemoveAllProc)(void* userData);
			typedef	void		(*DisposeUserDataProc)(void* userData);

						// Lifecycle methods
						Procs(GetKeyCountProc getKeyCountProc, GetKeyAtIndexProc getKeyAtIndexProc,
								GetValueProc getValueProc, SetProc setProc, RemoveKeysProc removeKeysProc,
								RemoveAllProc removeAllProc, DisposeUserDataProc disposeUserDataProc, void* userData) :
							mGetKeyCountProc(getKeyCountProc), mGetKeyAtIndexProc(getKeyAtIndexProc),
									mGetValueProc(getValueProc), mSetProc(setProc), mRemoveKeysProc(removeKeysProc),
									mRemoveAllProc(removeAllProc), mDisposeUserDataProc(disposeUserDataProc),
									mUserData(userData)
							{}

						// Instance methods
			KeyCount	getKeyCount() const
							{ return mGetKeyCountProc(mUserData); }
			CString		getKeyAtIndex(UInt32 index) const
							{ return mGetKeyAtIndexProc(index, mUserData); }
			OR<SValue>	getValue(const CString& key) const
							{ return mGetValueProc(key, mUserData); }
			void		set(const CString& key, const SValue& value)
							{ mSetProc(key, value, mUserData); }
			void		removeKeys(const TSet<CString>& keys)
							{ mRemoveKeysProc(keys, mUserData); }
			void		removeAll()
							{ mRemoveAllProc(mUserData); }
			void		disposeUserData() const
							{ return mDisposeUserDataProc(mUserData); }

			// Properties
			private:
				GetKeyCountProc		mGetKeyCountProc;
				GetKeyAtIndexProc	mGetKeyAtIndexProc;
				GetValueProc		mGetValueProc;
				SetProc				mSetProc;
				RemoveKeysProc		mRemoveKeysProc;
				RemoveAllProc		mRemoveAllProc;
				DisposeUserDataProc	mDisposeUserDataProc;
				void*				mUserData;
		};

	// Methods
	public:
												// Lifecycle methods
												CDictionary(SValue::OpaqueCopyProc opaqueCopyProc = nil,
														SValue::OpaqueDisposeProc opaqueDisposeProc = nil,
														SValue::OpaqueEqualsProc opaqueEqualsProc = nil);
												CDictionary(const Procs& procs);
												CDictionary(const CDictionary& other);
		virtual									~CDictionary();

												// CEquatable methods
						bool					operator==(const CEquatable& other) const
													{ return equals((const CDictionary&) other); }

												// Instance methods
						KeyCount				getKeyCount() const;
						TSet<CString>			getKeys() const;
						bool					isEmpty() const
													{ return getKeyCount() == 0; }

						bool					contains(const CString& key) const;

				const	SValue&					getValue(const CString& key) const;
						bool					getBool(const CString& key, bool defaultValue = false) const;
				const	TArray<CDictionary>&	getArrayOfDictionaries(const CString& key,
														const TArray<CDictionary>& defaultValue =
																TNArray<CDictionary>()) const;
				const	TArray<CString>&		getArrayOfStrings(const CString& key,
														const TArray<CString>& defaultValue = TNArray<CString>()) const;
				const	CData&					getData(const CString& key, const CData& defaultValue = CData::mEmpty)
														const;
				const	CDictionary&			getDictionary(const CString& key,
														const CDictionary& defaultValue = mEmpty) const;
				const	CString&				getString(const CString& key,
														const CString& defaultValue = CString::mEmpty) const;
						Float32					getFloat32(const CString& key, Float32 defaultValue = 0.0) const;
						Float64					getFloat64(const CString& key, Float64 defaultValue = 0.0) const;
						SInt8					getSInt8(const CString& key, SInt8 defaultValue = 0) const;
						SInt16					getSInt16(const CString& key, SInt16 defaultValue = 0) const;
						SInt32					getSInt32(const CString& key, SInt32 defaultValue = 0) const;
						SInt64					getSInt64(const CString& key, SInt64 defaultValue = 0) const;
						UInt8					getUInt8(const CString& key, UInt8 defaultValue = 0) const;
						UInt16					getUInt16(const CString& key, UInt16 defaultValue = 0) const;
						UInt32					getUInt32(const CString& key, UInt32 defaultValue = 0) const;
						UInt64					getUInt64(const CString& key, UInt64 defaultValue = 0) const;
						OV<SValue::Opaque>		getOpaque(const CString& key) const;
						OSType					getOSType(const CString& key, OSType defaultValue = 0) const
													{ return getUInt32(key, defaultValue); }
						void					getValue(const CString& key, Float32& outValue,
														Float32 defaultValue = 0.0) const
													{ outValue = getFloat32(key, defaultValue); }
						void					getValue(const CString& key, Float64& outValue,
														Float64 defaultValue = 0.0) const
													{ outValue = getFloat64(key, defaultValue); }
						void					getValue(const CString& key, SInt8& outValue, SInt8 defaultValue = 0)
														const
													{ outValue = getSInt8(key, defaultValue); }
						void					getValue(const CString& key, SInt16& outValue, SInt16 defaultValue = 0)
														const
													{ outValue = getSInt16(key, defaultValue); }
						void					getValue(const CString& key, SInt32& outValue, SInt32 defaultValue = 0)
														const
													{ outValue = getSInt32(key, defaultValue); }
						void					getValue(const CString& key, SInt64& outValue, SInt64 defaultValue = 0)
														const
													{ outValue = getSInt64(key, defaultValue); }
						void					getValue(const CString& key, UInt8& outValue, UInt8 defaultValue = 0)
														const
													{ outValue = getUInt8(key, defaultValue); }
						void					getValue(const CString& key, UInt16& outValue, UInt16 defaultValue = 0)
														const
													{ outValue = getUInt16(key, defaultValue); }
						void					getValue(const CString& key, UInt32& outValue, UInt32 defaultValue = 0)
														const
													{ outValue = getUInt32(key, defaultValue); }
						void					getValue(const CString& key, UInt64& outValue, UInt64 defaultValue = 0)
														const
													{ outValue = getUInt64(key, defaultValue); }

						void					set(const CString& key, bool value);
						void					set(const CString& key, const TArray<CDictionary>& value);
						void					set(const CString& key, const TArray<CString>& value);
						void					set(const CString& key, const CData& value);
						void					set(const CString& key, const CDictionary& value);
						void					set(const CString& key, const CString& value);
						void					set(const CString& key, Float32 value);
						void					set(const CString& key, Float64 value);
						void					set(const CString& key, SInt8 value);
						void					set(const CString& key, SInt16 value);
						void					set(const CString& key, SInt32 value);
						void					set(const CString& key, SInt64 value);
						void					set(const CString& key, UInt8 value);
						void					set(const CString& key, UInt16 value);
						void					set(const CString& key, UInt32 value);
						void					set(const CString& key, UInt64 value);
						void					set(const CString& key, SValue::Opaque value);
						void					set(const CString& key, const SValue& value);
						void					set(const CString& key, const OI<SValue>& value);
						void					set(const CString& key, const OR<SValue>& value);

						void					remove(const CString& key);
						void					remove(const TSet<CString>& keys);
						void					removeAll();

						bool					equals(const CDictionary& other, void* itemCompareProcUserData = nil)
														const;

						TIteratorS<Item>		getIterator() const;

				const	OR<SValue>				operator[](const CString& key) const;
						CDictionary&			operator=(const CDictionary& other);
						CDictionary&			operator+=(const CDictionary& other);

	// Properties
	public:
		static	CDictionary				mEmpty;

	private:
				CDictionaryInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TDictionary

template <typename T> class TDictionary : public CDictionary {
	// Methods
	public:
						// Instance methods
		const	OR<T>	get(const CString& key) const
							{
								// Get opaque
								OV<SValue::Opaque>	opaque = CDictionary::getOpaque(key);

								return opaque.hasValue() ? OR<T>(*((T*) opaque.getValue())) : OR<T>();
							}

		const	OR<T>	operator[](const CString& key) const
							{ return get(key); }

	protected:
						// Lifecycle methods
						TDictionary(SValue::OpaqueCopyProc opaqueCopyProc, SValue::OpaqueEqualsProc opaqueEqualsProc) :
							CDictionary(opaqueCopyProc, dispose, opaqueEqualsProc)
							{}

	private:
		static	void	dispose(SValue::Opaque opaque)
							{ T* t = (T*) opaque; Delete(t); }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TNDictionary (TDictionary where copy happens through new T())

template <typename T> class TNDictionary : public TDictionary<T> {
	// Methods
	public:
						// Lifecycle methods
						TNDictionary(SValue::OpaqueEqualsProc opaqueEqualsProc = nil) :
							TDictionary<T>((SValue::OpaqueCopyProc) copy, opaqueEqualsProc)
							{}
						TNDictionary(const TDictionary<T>& other, CDictionary::Item::IncludeProc itemIncludeProc,
								void* userData) :
							TDictionary<T>((SValue::OpaqueCopyProc) copy, nil)
							{
								for (TIteratorS<CDictionary::Item> iterator = other.getIterator(); iterator.hasValue();
										iterator.advance()) {
									// Call proc
									if (itemIncludeProc(*iterator, userData))
										// Add item
										CDictionary::set(iterator->mKey, iterator->mValue);
								}
							}

						// Instance methods
				void	set(const CString& key, const T& item)
							{ CDictionary::set(key, new T(item)); }
				void	set(const CString& key, const OI<T>& item)
							{
								// Check for instance
								if (item.hasInstance())
									// Set
									CDictionary::set(key, new T(*item));
								else
									// Remove
									CDictionary::remove(key);
							}

	private:
						// Class methods
		static	T*		copy(SValue::Opaque opaque)
							{ return new T(*((T*) opaque)); }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TCDictionary (TDictionary where copy happens through opaque->copy())

template <typename T> class TCDictionary : public TDictionary<T> {
	// Methods
	public:
						// Lifecycle methods
						TCDictionary(SValue::OpaqueEqualsProc opaqueEqualsProc = nil) :
							TDictionary<T>((SValue::OpaqueCopyProc) copy, opaqueEqualsProc)
							{}

						// Instance methods
				void	set(const CString& key, const T& item)
							{ CDictionary::set(key, item.copy()); }

	private:
						// Class methods
		static	T*		copy(SValue::Opaque opaque)
							{ return ((T*) opaque)->copy(); }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TReferenceDictionary

template <typename T> class TReferenceDictionary : public CDictionary {
	// Methods
	public:
						// Lifecycle methods
						TReferenceDictionary() : CDictionary() {}
						TReferenceDictionary(const TReferenceDictionary& dictionary) : CDictionary(dictionary) {}
						~TReferenceDictionary() {}

						// Instance methods
		const	OR<T>	get(const CString& key) const
							{
								// Get opaque
								OV<SValue::Opaque>	opaque = CDictionary::getOpaque(key);

								return opaque.hasValue() ? OR<T>(*((T*) opaque.getValue())) : OR<T>();
							}
				void	set(const CString& key, const T& item)
							{ CDictionary::set(key, &item); }

		const	OR<T>	operator[](const CString& key) const
							{ return get(key); }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TKeyConvertibleDictionary

template <typename K, typename T> class TKeyConvertibleDictionary : public CDictionary {
	// Methods
	public:
						// Lifecycle methods
						TKeyConvertibleDictionary(SValue::OpaqueEqualsProc opaqueEqualsProc = nil) :
							CDictionary((SValue::OpaqueCopyProc) copy, dispose, opaqueEqualsProc)
							{}
						TKeyConvertibleDictionary(const TKeyConvertibleDictionary& dictionary) :
							CDictionary(dictionary)
							{}
						~TKeyConvertibleDictionary() {}

						// Instance methods
		const	OR<T>	get(K key) const
							{
								// Get opaque
								OV<SValue::Opaque>	opaque = CDictionary::getOpaque(CString(key));

								return opaque.hasValue() ? OR<T>(*((T*) opaque.getValue())) : OR<T>();
							}
				void	set(K key, const T& item)
							{ CDictionary::set(CString(key), new T(item)); }

				void	remove(K key)
							{ CDictionary::remove(CString(key)); }

		const	OR<T>	operator[](K key) const
							{ return get(key); }

	private:
						// Class methods
		static	T*		copy(SValue::Opaque opaque)
							{ return new T(*((T*) opaque)); }
		static	void	dispose(SValue::Opaque opaque)
							{ T* t = (T*) opaque; Delete(t); }
};
