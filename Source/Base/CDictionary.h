//----------------------------------------------------------------------------------------------------------------------
//	CDictionary.h			©2007 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CData.h"
#include "CSet.h"
#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Types

typedef			UInt32	CDictionaryKeyCount;
typedef	const	void*	CDictionaryItemRef;

enum EDictionaryValueType {
	kDictionaryValueTypeBool,
	kDictionaryValueTypeArrayOfDictionaries,
	kDictionaryValueTypeArrayOfStrings,
	kDictionaryValueTypeData,
	kDictionaryValueTypeDictionary,
	kDictionaryValueTypeString,
	kDictionaryValueTypeFloat32,
	kDictionaryValueTypeFloat64,
	kDictionaryValueTypeSInt8,
	kDictionaryValueTypeSInt16,
	kDictionaryValueTypeSInt32,
	kDictionaryValueTypeSInt64,
	kDictionaryValueTypeUInt8,
	kDictionaryValueTypeUInt16,
	kDictionaryValueTypeUInt32,
	kDictionaryValueTypeUInt64,
	kDictionaryValueItemRef,
};

class CDictionary;
struct SDictionaryValue {
	EDictionaryValueType	mValueType;
	union {
		bool					mBool;
		TArrayT<CDictionary>*	mArrayOfDictionaries;
		TArrayT<CString>*		mArrayOfStrings;
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
		CDictionaryItemRef		mItemRef;
	} 						mValue;
};

struct SDictionaryItem {
	// Lifecycle methods
	SDictionaryItem(const CString& key, const SDictionaryValue& value) : mKey(key), mValue(value) {}

	// Properties
	CString				mKey;
	SDictionaryValue	mValue;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDictionary

class CDictionaryInternals;
class CDictionary {
	// Methods
	public:
											// Lifecycle methods
											CDictionary();
											CDictionary(const CDictionary& other);
		virtual								~CDictionary();

											// Instance methods
				CDictionaryKeyCount			getKeyCount() const;
				TSet<CString>				getKeys() const;
				bool						isEmpty() const
												{ return getKeyCount() == 0; }

				bool						contains(const CString& key) const;

				bool						getBool(const CString& key, bool notFoundValue = false) const;
				TArrayT<CDictionary>		getArrayOfDictionaries(const CString& key,
													const TArrayT<CDictionary>& notFoundValue = TArrayT<CDictionary>())
													const;
				TArrayT<CString>			getArrayOfStrings(const CString& key,
													const TArrayT<CString>& notFoundValue = TArrayT<CString>()) const;
				CData						getData(const CString& key, const CData& notFoundValue = CData::mEmpty)
													const;
				CDictionary					getDictionary(const CString& key,
													const CDictionary& notFoundValue = CDictionary::mEmpty) const;
				CString						getString(const CString& key,
													const CString& notFoundValue = CString::mEmpty) const;
				Float32						getFloat32(const CString& key, Float32 notFoundValue = 0.0) const;
				Float64						getFloat64(const CString& key, Float64 notFoundValue = 0.0) const;
				SInt8						getSInt8(const CString& key, SInt8 notFoundValue = 0) const;
				SInt16						getSInt16(const CString& key, SInt16 notFoundValue = 0) const;
				SInt32						getSInt32(const CString& key, SInt32 notFoundValue = 0) const;
				SInt64						getSInt64(const CString& key, SInt64 notFoundValue = 0) const;
				UInt8						getUInt8(const CString& key, UInt8 notFoundValue = 0) const;
				UInt16						getUInt16(const CString& key, UInt16 notFoundValue = 0) const;
				UInt32						getUInt32(const CString& key, UInt32 notFoundValue = 0) const;
				UInt64						getUInt64(const CString& key, UInt64 notFoundValue = 0) const;
				OSType						getOSType(const CString& key, OSType notFoundValue = 0) const
												{ return getUInt32(key, notFoundValue); }
				CDictionaryItemRef			getItemRef(const CString& key) const;
				void						getValue(const CString& key, Float32& outValue, Float32 notFoundValue = 0.0)
													const
												{ outValue = getFloat32(key, notFoundValue); }
				void						getValue(const CString& key, Float64& outValue, Float64 notFoundValue = 0.0)
													const
												{ outValue = getFloat64(key, notFoundValue); }
				void						getValue(const CString& key, SInt8& outValue, SInt8 notFoundValue = 0) const
												{ outValue = getSInt8(key, notFoundValue); }
				void						getValue(const CString& key, SInt16& outValue, SInt16 notFoundValue = 0)
													const
												{ outValue = getSInt16(key, notFoundValue); }
				void						getValue(const CString& key, SInt32& outValue, SInt32 notFoundValue = 0)
													const
												{ outValue = getSInt32(key, notFoundValue); }
				void						getValue(const CString& key, SInt64& outValue, SInt64 notFoundValue = 0)
													const
												{ outValue = getSInt64(key, notFoundValue); }
				void						getValue(const CString& key, UInt8& outValue, UInt8 notFoundValue = 0) const
												{ outValue = getUInt8(key, notFoundValue); }
				void						getValue(const CString& key, UInt16& outValue, UInt16 notFoundValue = 0)
													const
												{ outValue = getUInt16(key, notFoundValue); }
				void						getValue(const CString& key, UInt32& outValue, UInt32 notFoundValue = 0)
													const
												{ outValue = getUInt32(key, notFoundValue); }
				void						getValue(const CString& key, UInt64& outValue, UInt64 notFoundValue = 0)
													const
												{ outValue = getUInt64(key, notFoundValue); }

				void						set(const CString& key, bool value);
				void						set(const CString& key, const TArrayT<CDictionary>& value);
				void						set(const CString& key, const TArrayT<CString>& value);
				void						set(const CString& key, const CData& value);
				void						set(const CString& key, const CDictionary& value);
				void						set(const CString& key, const CString& value);
				void						set(const CString& key, Float32 value);
				void						set(const CString& key, Float64 value);
				void						set(const CString& key, SInt8 value);
				void						set(const CString& key, SInt16 value);
				void						set(const CString& key, SInt32 value);
				void						set(const CString& key, SInt64 value);
				void						set(const CString& key, UInt8 value);
				void						set(const CString& key, UInt16 value);
				void						set(const CString& key, UInt32 value);
				void						set(const CString& key, UInt64 value);
				void						set(const CString& key, CDictionaryItemRef value);

	virtual		void						remove(const CString& key);
	virtual		void						removeAll();

				TIterator<SDictionaryItem>	getIterator() const;

				CDictionary&				operator=(const CDictionary& other);
//				bool						operator==(const CDictionary& other) const;
//				bool						operator!=(const CDictionary& other) const
//												{ return !operator==(other); }

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
							// Lifecycle methods
							TDictionary() : CDictionary() {}
							TDictionary(const TDictionary& dictionary) : CDictionary(dictionary) {}
							~TDictionary() {}

							// Instance methods
		const	T			get(const CString& key) const
								{ return (T) CDictionary::getItemRef(key); }

		const	TArray<T>	getValues() const
								{
									// Get values
									TArray<T>	values;
									for (TIterator<SDictionaryItem> iterator = getIterator();
											iterator.hasValue(); iterator.advance())
										// Insert value
										values.add((T) iterator.getValue().mValue.mValue.mItemRef);

									return values;
								}

				void		set(const CString& key, const T item)
								{
									// Get existing value
									CDictionaryItemRef	itemRef = getItemRef(key);
									if (itemRef != nil) {
										// Dispose of existing value
										T	t = (T) itemRef;
										DisposeOf(t);
									}

									// Update to new value
									CDictionary::set(key, item);
								}

				void		remove(const CString& key)
								{
									// Get
									T	t = (T) getItemRef(key);
									if (t != nil) {
										// Remove
										CDictionary::remove(key);

										// Dispose
										DisposeOf(t);
									}
								}
				void		removeAll()
								{
									// Dispose all values
									for (TIterator<SDictionaryItem> iterator = getIterator();
											iterator.hasValue(); iterator.advance()) {
										// Dispose of existing value
										T	t = (T) iterator.getValue().mValue.mValue.mItemRef;
										DisposeOf(t);
									}

									// Remove all
									CDictionary::removeAll();
								}

		const	T			operator[](const CString& key) const
								{ return (T) CDictionary::getItemRef(key); }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TKeyConvertibleDictionary

template <typename K, typename T> class TKeyConvertibleDictionary : public CDictionary {
	// Methods
	public:
							// Lifecycle methods
							TKeyConvertibleDictionary() : CDictionary() {}
							TKeyConvertibleDictionary(const TKeyConvertibleDictionary& dictionary) :
								CDictionary(dictionary)
								{}
							~TKeyConvertibleDictionary() {}

							// Instance methods
		const	T			get(K key) const
								{ return (T) CDictionary::getItemRef(CString(key)); }

		const	TArray<T>	getValues() const
								{
									// Get values
									TArray<T>	values;
									for (TIterator<SDictionaryItem> iterator = getIterator();
											iterator.hasValue(); iterator.advance())
										// Insert value
										values.add((T) iterator.getValue().mValue.mValue.mItemRef);

									return values;
								}

				void		set(K key, const T item)
								{
									// Get existing value
									CDictionaryItemRef	itemRef = getItemRef(CString(key));
									if (itemRef != nil) {
										// Dispose of existing value
										T	t = (T) itemRef;
										DisposeOf(t);
									}

									// Update to new value
									CDictionary::set(CString(key), item);
								}

				void		remove(K key)
								{
									// Get
									T	t = (T) getItemRef(CString(key));
									if (t != nil) {
										// Remove
										CDictionary::remove(CString(key));

										// Dispose
										DisposeOf(t);
									}
								}
				void		removeAll()
								{
									// Dispose all values
									for (TIterator<SDictionaryItem> iterator = getIterator();
											iterator.hasValue(); iterator.advance()) {
										// Dispose of existing value
										T	t = (T) iterator.getValue().mValue.mValue.mItemRef;
										DisposeOf(t);
									}

									// Remove all
									CDictionary::removeAll();
								}

		const	T			operator[](K key) const
								{ return (T) CDictionary::getItemRef(CString(key)); }
};
