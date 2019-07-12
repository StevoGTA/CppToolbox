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

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Procs

typedef	CDictionaryItemRef	(*CDictionaryItemCopyProc)(CDictionaryItemRef itemRef);
typedef	void				(*CDictionaryItemDisposeProc)(CDictionaryItemRef itemRef);
typedef	bool				(*CDictionaryItemEqualsProc)(CDictionaryItemRef itemRef1, CDictionaryItemRef itemRef2);

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Values

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
	kDictionaryValueTypeItemRef,
};

class CDictionary;
struct SDictionaryValue {
	EDictionaryValueType	mValueType;
	union {
		bool					mBool;
		TArray<CDictionary>*	mArrayOfDictionaries;
		TArray<CString>*		mArrayOfStrings;
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
// MARK: - CDictionary

class CDictionaryInternals;
class CDictionary : public CEquatable {
	// Methods
	public:
											// Lifecycle methods
											CDictionary(CDictionaryItemCopyProc itemCopyProc = nil,
													CDictionaryItemDisposeProc itemDisposeProc = nil,
													CDictionaryItemEqualsProc itemEqualsProc = nil);
											CDictionary(const CDictionary& other);
		virtual								~CDictionary();

											// CEquatable methods
				bool						operator==(const CEquatable& other) const
												{ return equals((const CDictionary&) other); }

											// Instance methods
				CDictionaryKeyCount			getKeyCount() const;
				TSet<CString>				getKeys() const;
				bool						isEmpty() const
												{ return getKeyCount() == 0; }

				bool						contains(const CString& key) const;

				bool						getBool(const CString& key, bool notFoundValue = false) const;
				TArray<CDictionary>			getArrayOfDictionaries(const CString& key,
													const TArray<CDictionary>& notFoundValue = TArray<CDictionary>())
													const;
				TArray<CString>				getArrayOfStrings(const CString& key,
													const TArray<CString>& notFoundValue = TArray<CString>()) const;
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
				void						set(const CString& key, const TArray<CDictionary>& value);
				void						set(const CString& key, const TArray<CString>& value);
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

				bool						equals(const CDictionary& other, void* itemCompareProcUserData = nil) const;

				TIteratorS<SDictionaryItem>	getIterator() const;

				CDictionary&				operator=(const CDictionary& other);

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
								TDictionary(CDictionaryItemEqualsProc itemEqualsProc = nil) :
									CDictionary(nil, nil, itemEqualsProc)
									{}
								TDictionary(const TDictionary& dictionary) : CDictionary(dictionary) {}
								~TDictionary() {}

								// Instance methods
		const	T				get(const CString& key) const
									{ return (T) CDictionary::getItemRef(key); }

		const	TPtrArray<T>	getValues() const
									{
										// Get values
										TPtrArray<T>	values;
										for (TIteratorS<SDictionaryItem> iterator = getIterator();
												iterator.hasValue(); iterator.advance())
											// Insert value
											values.add((T) iterator.getValue().mValue.mValue.mItemRef);

										return values;
									}

				void			set(const CString& key, const T item)
									{ CDictionary::set(key, item); }


		const	T				operator[](const CString& key) const
									{ return (T) CDictionary::getItemRef(key); }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TOwningDictionary

template <typename T> class TOwningDictionary : public CDictionary {
	// Methods
	public:
								// Lifecycle methods
								TOwningDictionary(CDictionaryItemEqualsProc itemEqualsProc = nil) :
									CDictionary((CDictionaryItemCopyProc) copy, dispose, itemEqualsProc)
									{}
								TOwningDictionary(const TOwningDictionary& dictionary) : CDictionary(dictionary) {}
								~TOwningDictionary() {}

								// Instance methods
				T*				get(const CString& key) const
									{ return (T*) CDictionary::getItemRef(key); }

		const	TPtrArray<T*>	getValues() const
									{
										// Get values
										TPtrArray<T*>	values;
										for (TIteratorS<SDictionaryItem> iterator = getIterator();
												iterator.hasValue(); iterator.advance())
											// Insert value
											values.add((T*) iterator.getValue().mValue.mValue.mItemRef);

										return values;
									}

				void			set(const CString& key, const T& item)
									{ CDictionary::set(key, new T(item)); }

				T*				operator[](const CString& key) const
									{ return (T*) CDictionary::getItemRef(key); }

	private:
								// Class methods
		static	T*				copy(CDictionaryItemRef itemRef)
									{ return new T(*((T*) itemRef)); }
		static	void			dispose(CDictionaryItemRef itemRef)
									{ T* t = (T*) itemRef; DisposeOf(t); }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TKeyConvertibleDictionary

template <typename K, typename T> class TKeyConvertibleDictionary : public CDictionary {
	// Methods
	public:
								// Lifecycle methods
								TKeyConvertibleDictionary(CDictionaryItemEqualsProc itemEqualsProc = nil) :
									CDictionary(nil, nil, itemEqualsProc)
									{}
								TKeyConvertibleDictionary(const TKeyConvertibleDictionary& dictionary) :
									CDictionary(dictionary)
									{}
								~TKeyConvertibleDictionary() {}

								// Instance methods
		const	T				get(K key) const
									{ return (T) CDictionary::getItemRef(CString(key)); }

		const	TPtrArray<T>	getValues() const
									{
										// Get values
										TPtrArray<T>	values;
										for (TIteratorS<SDictionaryItem> iterator = getIterator();
												iterator.hasValue(); iterator.advance())
											// Insert value
											values.add((T) iterator.getValue().mValue.mValue.mItemRef);

										return values;
									}

				void			set(K key, const T item)
									{ CDictionary::set(CString(key), item); }

				void			remove(K key)
									{ CDictionary::remove(CString(key)); }
				void			removeAll()
									{ CDictionary::removeAll(); }

		const	T				operator[](K key) const
									{ return (T) CDictionary::getItemRef(CString(key)); }
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - TOwningKeyConvertibleDictionary

template <typename K, typename T> class TOwningKeyConvertibleDictionary : public CDictionary {
	// Methods
	public:
								// Lifecycle methods
								TOwningKeyConvertibleDictionary(CDictionaryItemEqualsProc itemEqualsProc = nil) :
									CDictionary((CDictionaryItemCopyProc) copy, dispose, itemEqualsProc)
									{}
								TOwningKeyConvertibleDictionary(const TOwningKeyConvertibleDictionary& dictionary) :
									CDictionary(dictionary)
									{}
								~TOwningKeyConvertibleDictionary() {}

								// Instance methods
				T*				get(K key) const
									{ return (T*) CDictionary::getItemRef(CString(key)); }

		const	TPtrArray<T*>	getValues() const
									{
										// Get values
										TPtrArray<T*>	values;
										for (TIteratorS<SDictionaryItem> iterator = getIterator();
												iterator.hasValue(); iterator.advance())
											// Insert value
											values.add((T*) iterator.getValue().mValue.mValue.mItemRef);

										return values;
									}

				void			set(K key, const T& item)
									{ CDictionary::set(CString(key), new T(item)); }

				void			remove(K key)
									{ CDictionary::remove(CString(key)); }

				T*				operator[](K key) const
									{ return (T*) CDictionary::getItemRef(CString(key)); }

	private:
								// Class methods
		static	T*				copy(CDictionaryItemRef itemRef)
									{ return new T(*((T*) itemRef)); }
		static	void			dispose(CDictionaryItemRef itemRef)
									{ T* t = (T*) itemRef; DisposeOf(t); }
};
