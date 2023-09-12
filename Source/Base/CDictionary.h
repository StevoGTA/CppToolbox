//----------------------------------------------------------------------------------------------------------------------
//	CDictionary.h			©2007 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CSet.h"
#include "CString.h"
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
														SValue::OpaqueEqualsProc opaqueEqualsProc = nil,
														SValue::OpaqueDisposeProc opaqueDisposeProc = nil);
												CDictionary(const Procs& procs);
												CDictionary(const CDictionary& other);
		virtual									~CDictionary();

												// CEquatable methods
						bool					operator==(const CEquatable& other) const
													{ return equals((const CDictionary&) other); }

												// Instance methods
		virtual			KeyCount				getKeyCount() const;
		virtual			TSet<CString>			getKeys() const;
						bool					isEmpty() const
													{ return getKeyCount() == 0; }

		virtual			bool					contains(const CString& key) const;

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
						void					getValue(const CString& key, bool& outValue,
														bool defaultValue = false) const
													{ outValue = getBool(key, defaultValue); }
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
						void					set(const CString& key, const OV<SValue>& value);
						void					set(const CString& key, const OR<SValue>& value);

						void					remove(const CString& key);
						void					remove(const TArray<CString>& keys);
						void					remove(const TSet<CString>& keys);
						CDictionary				removing(const TArray<CString>& keys);
						CDictionary				removing(const TSet<CString>& keys);
						void					removeAll();

						bool					equals(const CDictionary& other, void* itemCompareProcUserData = nil)
														const;

						TIteratorS<Item>		getIterator() const;

				const	OR<SValue>				operator[](const CString& key) const;
						CDictionary&			operator=(const CDictionary& other);
						CDictionary				operator+(const CDictionary& other) const;
						CDictionary&			operator+=(const CDictionary& other);

	// Properties
	public:
		static	const	CDictionary				mEmpty;

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
						TDictionary(SValue::OpaqueCopyProc opaqueCopyProc, SValue::OpaqueEqualsProc opaqueEqualsProc,
								SValue::OpaqueDisposeProc opaqueDisposeProc) :
							CDictionary(opaqueCopyProc, opaqueEqualsProc, opaqueDisposeProc)
							{}
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TMDictionary (TDictionary which can be modified)

template <typename T> class TMDictionary : public TDictionary<T> {
	// Methods
	public:
				// Instance methods
		void	set(const CString& key, const T& item)
					{ CDictionary::set(key, new T(item)); }
		void	set(const CString& key, const OV<T>& item)
					{
						// Check for instance
						if (item.hasValue())
							// Set
							CDictionary::set(key, new T(*item));
						else
							// Remove
							CDictionary::remove(key);
					}

	protected:
				// Lifecycle methods
				TMDictionary(SValue::OpaqueCopyProc opaqueCopyProc, SValue::OpaqueEqualsProc opaqueEqualsProc,
						SValue::OpaqueDisposeProc opaqueDisposeProc) :
					TDictionary<T>(opaqueCopyProc, opaqueEqualsProc, opaqueDisposeProc)
					{}
				TMDictionary(SValue::OpaqueCopyProc opaqueCopyProc, SValue::OpaqueEqualsProc opaqueEqualsProc,
						SValue::OpaqueDisposeProc opaqueDisposeProc, const TDictionary<T>& other,
						CDictionary::Item::IncludeProc itemIncludeProc, void* userData) :
					TDictionary<T>(opaqueCopyProc, opaqueEqualsProc, opaqueDisposeProc)
					{
						for (TIteratorS<CDictionary::Item> iterator = other.getIterator(); iterator.hasValue();
								iterator.advance()) {
							// Call proc
							if (itemIncludeProc(*iterator, userData))
								// Add item
								CDictionary::set(iterator->mKey, iterator->mValue);
						}
					}
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TNDictionary (TMDictionary where copy happens through new T())

template <typename T> class TNDictionary : public TMDictionary<T> {
	// Methods
	public:
						// Lifecycle methods
						TNDictionary(SValue::OpaqueEqualsProc opaqueEqualsProc = nil) :
							TMDictionary<T>((SValue::OpaqueCopyProc) copy, opaqueEqualsProc,
									(SValue::OpaqueDisposeProc) dispose)
							{}
						TNDictionary(const TDictionary<T>& other, CDictionary::Item::IncludeProc itemIncludeProc,
								void* userData) :
							TMDictionary<T>((SValue::OpaqueCopyProc) copy, nil, (SValue::OpaqueDisposeProc) dispose,
									other, itemIncludeProc, userData)
							{}
						TNDictionary(const TDictionary<T>& other) :
							TMDictionary<T>((SValue::OpaqueCopyProc) copy, nil, (SValue::OpaqueDisposeProc) dispose,
									other)
							{}

	private:
						// Class methods
		static	T*		copy(SValue::Opaque opaque)
							{ return new T(*((T*) opaque)); }
		static	void	dispose(SValue::Opaque opaque)
							{ T* t = (T*) opaque; Delete(t); }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TNArrayDictionary (TNDictionary where values are arrays)

template <typename T> class TNArrayDictionary : public TNDictionary<TNArray<T> > {
	// Methods
	public:
				// Instance methods
		void	add(const CString& key, const T& item)
					{
						// Update
						const	OR<TNArray<T> >	array = TNDictionary<TNArray<T> >::get(key);
						if (array.hasReference())
							// Already have array
							*array += item;
						else
							// First one
							TNDictionary<TNArray<T> >::set(key, TNArray<T>(item));
					}
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TCDictionary (TDictionary where copy happens through opaque->copy())

template <typename T> class TCDictionary : public TMDictionary<T> {
	// Methods
	public:
						// Lifecycle methods
						TCDictionary(SValue::OpaqueEqualsProc opaqueEqualsProc = nil) :
							TMDictionary<T>((SValue::OpaqueCopyProc) copy, opaqueEqualsProc,
									(SValue::OpaqueDisposeProc) dispose)
							{}

						// Instance methods
				void	set(const CString& key, const T& item)
							{ CDictionary::set(key, item.copy()); }

	private:
						// Class methods
		static	T*		copy(SValue::Opaque opaque)
							{ return ((T*) opaque)->copy(); }
		static	void	dispose(SValue::Opaque opaque)
							{ T* t = (T*) opaque; Delete(t); }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TReferenceDictionary

template <typename T> class TReferenceDictionary : public CDictionary {
	// Methods
	public:
						// Lifecycle methods
						TReferenceDictionary() : CDictionary() {}
						TReferenceDictionary(const TReferenceDictionary& dictionary) : CDictionary(dictionary) {}

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
// MARK: - TNKeyConvertibleDictionary

template <typename K, typename T> class TNKeyConvertibleDictionary : public TNDictionary<T> {
	// Methods
	public:
						// Lifecycle methods
						TNKeyConvertibleDictionary(SValue::OpaqueEqualsProc opaqueEqualsProc = nil) :
							TNDictionary<T>(opaqueEqualsProc)
							{}

						// Instance methods
		const	OR<T>	get(K key) const
							{ return TNDictionary<T>::get(CString(key)); }
				void	set(K key, const T& item)
							{ TNDictionary<T>::set(CString(key), item); }

				void	remove(K key)
							{ TNDictionary<T>::remove(CString(key)); }

		const	OR<T>	operator[](K key) const
							{ return get(key); }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TNArrayKeyConvertibleDictionary

template <typename K, typename T> class TNArrayKeyConvertibleDictionary :
		public TNKeyConvertibleDictionary<K, TNArray<T> > {
	// Methods
	public:
				// Instance methods
		void	add(K key, const T& item)
					{
						// Setup
						CString	keyString(key);

						// Update
						const	OR<TNArray<T> >	array = TNKeyConvertibleDictionary<K, TNArray<T> >::get(key);
						if (array.hasReference())
							// Already have array
							*array += item;
						else
							// First one
							TNKeyConvertibleDictionary<K, TNArray<T> >::set(key, TNArray<T>(item));
					}
};
