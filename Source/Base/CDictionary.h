//----------------------------------------------------------------------------------------------------------------------
//	CDictionary.h			©2007 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CData.h"
#include "CSet.h"
#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CDictionary

class CDictionaryInternals;
class CDictionary : public CEquatable {
	// Types
	public:
		typedef			UInt32	KeyCount;
		typedef	const	void*	ItemRef;

	// Procs
	public:
		typedef	ItemRef	(*ItemCopyProc)(ItemRef itemRef);
		typedef	void	(*ItemDisposeProc)(ItemRef itemRef);
		typedef	bool	(*ItemEqualsProc)(ItemRef itemRef1, ItemRef itemRef2);

	// Structs
	public:
		struct Value {
			// Enums
			public:
				enum Type {
					kArrayOfDictionaries,
					kArrayOfStrings,
					kBool,
					kData,
					kDictionary,
					kString,
					kFloat32,
					kFloat64,
					kSInt8,
					kSInt16,
					kSInt32,
					kSInt64,
					kUInt8,
					kUInt16,
					kUInt32,
					kUInt64,
					kItemRef,
				};

			// Procs
			public:
				typedef	const OR<Value>	(*Proc)(const CString& key, void* userData);

			// Methods
			public:
												// Lifecycle methods
												Value(const TArray<CDictionary>& value);
												Value(const TArray<CString>& value);
												Value(bool value);
												Value(const CData& value);
												Value(const CDictionary& value);
												Value(const CString& value);
												Value(Float32 value);
												Value(Float64 value);
												Value(SInt8 value);
												Value(SInt16 value);
												Value(SInt32 value);
												Value(SInt64 value);
												Value(UInt8 value);
												Value(UInt16 value);
												Value(UInt32 value);
												Value(UInt64 value);
												Value(ItemRef value);
												Value(const Value& other, ItemCopyProc itemCopyProc = nil);

												// Instance methods
						Type					getType() const { return mType; }

				const	TArray<CDictionary>&	getArrayOfDictionaries(
														const TArray<CDictionary>& defaultValue =
																TNArray<CDictionary>()) const;
				const	TArray<CString>&		getArrayOfStrings(
														const TArray<CString>& defaultValue = TNArray<CString>()) const;
						bool					getBool(bool defaultValue = false) const;
				const	CData&					getData(const CData& defaultValue = CData::mEmpty) const;
				const	CDictionary&			getDictionary(const CDictionary& defaultValue = mEmpty) const;
				const	CString&				getString(const CString& defaultValue = CString::mEmpty) const;
						Float32					getFloat32(Float32 defaultValue = 0.0) const;
						Float64					getFloat64(Float64 defaultValue = 0.0) const;
						SInt8					getSInt8(SInt8 defaultValue = 0) const;
						SInt16					getSInt16(SInt16 defaultValue = 0) const;
						SInt32					getSInt32(SInt32 defaultValue = 0) const;
						SInt64					getSInt64(SInt64 defaultValue = 0) const;
						UInt8					getUInt8(UInt8 defaultValue = 0) const;
						UInt16					getUInt16(UInt16 defaultValue = 0) const;
						UInt32					getUInt32(UInt32 defaultValue = 0) const;
						UInt64					getUInt64(UInt64 defaultValue = 0) const;
						CDictionary::ItemRef	getItemRef() const;

						bool					equals(const Value& other, ItemEqualsProc itemEqualsProc) const;

						void					dispose(ItemDisposeProc itemDisposeProc);

			// Properties
			private:
				Type	mType;
				union ValueValue {
					// Lifecycle methods
					ValueValue(TArray<CDictionary>* value) : mArrayOfDictionaries(value) {}
					ValueValue(TArray<CString>* value) : mArrayOfStrings(value) {}
					ValueValue(bool value) : mBool(value) {}
					ValueValue(CData* value) : mData(value) {}
					ValueValue(CDictionary* value) : mDictionary(value) {}
					ValueValue(CString* value) : mString(value) {}
					ValueValue(Float32 value) : mFloat32(value) {}
					ValueValue(Float64 value) : mFloat64(value) {}
					ValueValue(SInt8 value) : mSInt8(value) {}
					ValueValue(SInt16 value) : mSInt16(value) {}
					ValueValue(SInt32 value) : mSInt32(value) {}
					ValueValue(SInt64 value) : mSInt64(value) {}
					ValueValue(UInt8 value) : mUInt8(value) {}
					ValueValue(UInt16 value) : mUInt16(value) {}
					ValueValue(UInt32 value) : mUInt32(value) {}
					ValueValue(UInt64 value) : mUInt64(value) {}
					ValueValue(ItemRef value) : mItemRef(value) {}

					// Properties
					TArray<CDictionary>*	mArrayOfDictionaries;
					TArray<CString>*		mArrayOfStrings;
					bool					mBool;
					CData*					mData;
					CDictionary*			mDictionary;
					CString*				mString;
					Float32					mFloat32;
					Float64					mFloat64;
					SInt8					mSInt8;
					SInt16					mSInt16;
					SInt32					mSInt32;
					SInt64					mSInt64;
					UInt8					mUInt8;
					UInt16					mUInt16;
					UInt32					mUInt32;
					UInt64					mUInt64;
					ItemRef					mItemRef;
				}	mValue;
		};

		struct Item {
			// Procs
			typedef bool	(*IncludeProc)(const Item& item, void* userData);

			// Lifecycle methods
			Item(const CString& key, const Value& value) : mKey(key), mValue(value) {}
			Item(const CString& key, const Value& value, ItemCopyProc itemCopyProc) :
				mKey(key), mValue(value, itemCopyProc)
				{}
			Item(const Item& other, ItemCopyProc itemCopyProc) : mKey(other.mKey), mValue(other.mValue, itemCopyProc) {}

			// Properties
			CString	mKey;
			Value	mValue;
		};

		struct Procs {
			// Procs
			typedef	KeyCount	(*GetKeyCountProc)(void* userData);
			typedef	CString		(*GetKeyAtIndexProc)(UInt32 index, void* userData);
			typedef	OR<Value>	(*GetValueProc)(const CString& key, void* userData);
			typedef	void		(*SetProc)(const CString& key, const CDictionary::Value& value, void* userData);
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
			OR<Value>	getValue(const CString& key) const
							{ return mGetValueProc(key, mUserData); }
			void		set(const CString& key, const CDictionary::Value& value)
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
												CDictionary(ItemCopyProc itemCopyProc = nil,
														ItemDisposeProc itemDisposeProc = nil,
														ItemEqualsProc itemEqualsProc = nil);
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

				const	Value&					getValue(const CString& key) const;
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
						OV<ItemRef>				getItemRef(const CString& key) const;
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
						void					set(const CString& key, ItemRef value);
						void					set(const CString& key, const Value& value);
						void					set(const CString& key, const OI<CDictionary::Value>& value);
						void					set(const CString& key, const OR<CDictionary::Value>& value);

						void					remove(const CString& key);
						void					remove(const TSet<CString>& keys);
						void					removeAll();

						bool					equals(const CDictionary& other, void* itemCompareProcUserData = nil)
														const;

						TIteratorS<Item>		getIterator() const;

				const	OR<Value>				operator[](const CString& key) const;
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
								// Get itemRef
								OV<CDictionary::ItemRef>	itemRef = CDictionary::getItemRef(key);

								return itemRef.hasValue() ? OR<T>(*((T*) itemRef.getValue())) : OR<T>();
							}

		const	OR<T>	operator[](const CString& key) const
							{ return get(key); }

	protected:
						// Lifecycle methods
						TDictionary(CDictionary::ItemCopyProc copyProc, CDictionary::ItemEqualsProc itemEqualsProc) :
							CDictionary(copyProc, dispose, itemEqualsProc)
							{}

	private:
		static	void	dispose(CDictionary::ItemRef itemRef)
							{ T* t = (T*) itemRef; Delete(t); }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TNDictionary (TDictionary where copy happens through new T())

template <typename T> class TNDictionary : public TDictionary<T> {
	// Methods
	public:
						// Lifecycle methods
						TNDictionary(CDictionary::ItemEqualsProc itemEqualsProc = nil) :
							TDictionary<T>((CDictionary::ItemCopyProc) copy, itemEqualsProc)
							{}
						TNDictionary(const TDictionary<T>& other, CDictionary::Item::IncludeProc itemIncludeProc,
								void* userData) :
							TDictionary<T>((CDictionary::ItemCopyProc) copy, nil)
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
		static	T*		copy(CDictionary::ItemRef itemRef)
							{ return new T(*((T*) itemRef)); }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TCDictionary (TDictionary where copy happens through itemRef->copy())

template <typename T> class TCDictionary : public TDictionary<T> {
	// Methods
	public:
						// Lifecycle methods
						TCDictionary(CDictionary::ItemEqualsProc itemEqualsProc = nil) :
							TDictionary<T>((CDictionary::ItemCopyProc) copy, itemEqualsProc)
							{}

						// Instance methods
				void	set(const CString& key, const T& item)
							{ CDictionary::set(key, item.copy()); }

	private:
						// Class methods
		static	T*		copy(CDictionary::ItemRef itemRef)
							{ return ((T*) itemRef)->copy(); }
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
								// Get itemRef
								OV<CDictionary::ItemRef>	itemRef = CDictionary::getItemRef(key);

								return itemRef.hasValue() ? OR<T>(*((T*) itemRef.getValue())) : OR<T>();
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
						TKeyConvertibleDictionary(CDictionary::ItemEqualsProc itemEqualsProc = nil) :
							CDictionary((CDictionary::ItemCopyProc) copy, dispose, itemEqualsProc)
							{}
						TKeyConvertibleDictionary(const TKeyConvertibleDictionary& dictionary) :
							CDictionary(dictionary)
							{}
						~TKeyConvertibleDictionary() {}

						// Instance methods
		const	OR<T>	get(K key) const
							{
								// Get itemRef
								OV<CDictionary::ItemRef>	itemRef = CDictionary::getItemRef(CString(key));

								return itemRef.hasValue() ? OR<T>(*((T*) itemRef.getValue())) : OR<T>();
							}
				void	set(K key, const T& item)
							{ CDictionary::set(CString(key), new T(item)); }

				void	remove(K key)
							{ CDictionary::remove(CString(key)); }

		const	OR<T>	operator[](K key) const
							{ return get(key); }

	private:
						// Class methods
		static	T*		copy(CDictionary::ItemRef itemRef)
							{ return new T(*((T*) itemRef)); }
		static	void	dispose(CDictionary::ItemRef itemRef)
							{ T* t = (T*) itemRef; Delete(t); }
};
