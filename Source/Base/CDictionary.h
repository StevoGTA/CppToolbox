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

struct SDictionaryValue;
struct SDictionaryItem;

//----------------------------------------------------------------------------------------------------------------------
// MARK: - Procs

typedef	CDictionaryItemRef		(*CDictionaryItemCopyProc)(CDictionaryItemRef itemRef);
typedef	void					(*CDictionaryItemDisposeProc)(CDictionaryItemRef itemRef);
typedef	bool					(*CDictionaryItemEqualsProc)(CDictionaryItemRef itemRef1, CDictionaryItemRef itemRef2);

typedef	CDictionaryKeyCount		(*CDictionaryGetKeyCountProc)(void* userData);
typedef	OR<SDictionaryValue>	(*CDictionaryGetValueProc)(const CString& key, void* userData);
typedef	void					(*CDictionaryDisposeUserDataProc)(void* userData);

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SDictionaryProcsInfo

struct SDictionaryProcsInfo {
							// Lifecycle methods
							SDictionaryProcsInfo(CDictionaryGetKeyCountProc getKeyCountProc,
									CDictionaryGetValueProc getValueProc,
									CDictionaryDisposeUserDataProc disposeUserDataProc, void* userData) :
								mGetKeyCountProc(getKeyCountProc), mGetValueProc(getValueProc),
										mDisposeUserDataProc(disposeUserDataProc), mUserData(userData)
								{}

							// Instance methods
	CDictionaryKeyCount		getKeyCount() const
								{ return mGetKeyCountProc(mUserData); }
	OR<SDictionaryValue>	getValue(const CString& key) const
								{ return mGetValueProc(key, mUserData); }
	void					disposeUserData() const
								{ return mDisposeUserDataProc(mUserData); }

	// Properties
	private:
		CDictionaryGetKeyCountProc		mGetKeyCountProc;
		CDictionaryGetValueProc			mGetValueProc;
		CDictionaryDisposeUserDataProc	mDisposeUserDataProc;
		void*							mUserData;
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
													CDictionary(const SDictionaryProcsInfo& procsInfo);
													CDictionary(const CDictionary& other);
		virtual										~CDictionary();

													// CEquatable methods
						bool						operator==(const CEquatable& other) const
														{ return equals((const CDictionary&) other); }

													// Instance methods
						CDictionaryKeyCount			getKeyCount() const;
						TSet<CString>				getKeys() const;
						bool						isEmpty() const
														{ return getKeyCount() == 0; }

						bool						contains(const CString& key) const;

				const	SDictionaryValue&			getValue(const CString& key) const;
						bool						getBool(const CString& key, bool defaultValue = false) const;
				const	TNArray<CDictionary>&		getArrayOfDictionaries(const CString& key,
															const TNArray<CDictionary>& defaultValue =
																	TNArray<CDictionary>()) const;
				const	TNArray<CString>&			getArrayOfStrings(const CString& key,
															const TNArray<CString>& defaultValue = TNArray<CString>())
															const;
				const	CData&						getData(const CString& key,
															const CData& defaultValue = CData::mEmpty) const;
				const	CDictionary&				getDictionary(const CString& key,
															const CDictionary& defaultValue = CDictionary::mEmpty)
															const;
				const	CString&					getString(const CString& key,
															const CString& defaultValue = CString::mEmpty) const;
						Float32						getFloat32(const CString& key, Float32 defaultValue = 0.0) const;
						Float64						getFloat64(const CString& key, Float64 defaultValue = 0.0) const;
						SInt8						getSInt8(const CString& key, SInt8 defaultValue = 0) const;
						SInt16						getSInt16(const CString& key, SInt16 defaultValue = 0) const;
						SInt32						getSInt32(const CString& key, SInt32 defaultValue = 0) const;
						SInt64						getSInt64(const CString& key, SInt64 defaultValue = 0) const;
						UInt8						getUInt8(const CString& key, UInt8 defaultValue = 0) const;
						UInt16						getUInt16(const CString& key, UInt16 defaultValue = 0) const;
						UInt32						getUInt32(const CString& key, UInt32 defaultValue = 0) const;
						UInt64						getUInt64(const CString& key, UInt64 defaultValue = 0) const;
						OV<CDictionaryItemRef>		getItemRef(const CString& key) const;
						OSType						getOSType(const CString& key, OSType defaultValue = 0) const
														{ return getUInt32(key, defaultValue); }
						void						getValue(const CString& key, Float32& outValue,
															Float32 defaultValue = 0.0) const
														{ outValue = getFloat32(key, defaultValue); }
						void						getValue(const CString& key, Float64& outValue,
															Float64 defaultValue = 0.0) const
														{ outValue = getFloat64(key, defaultValue); }
						void						getValue(const CString& key, SInt8& outValue,
															SInt8 defaultValue = 0) const
														{ outValue = getSInt8(key, defaultValue); }
						void						getValue(const CString& key, SInt16& outValue,
															SInt16 defaultValue = 0) const
														{ outValue = getSInt16(key, defaultValue); }
						void						getValue(const CString& key, SInt32& outValue,
															SInt32 defaultValue = 0) const
														{ outValue = getSInt32(key, defaultValue); }
						void						getValue(const CString& key, SInt64& outValue,
															SInt64 defaultValue = 0) const
														{ outValue = getSInt64(key, defaultValue); }
						void						getValue(const CString& key, UInt8& outValue,
															UInt8 defaultValue = 0) const
														{ outValue = getUInt8(key, defaultValue); }
						void						getValue(const CString& key, UInt16& outValue,
															UInt16 defaultValue = 0) const
														{ outValue = getUInt16(key, defaultValue); }
						void						getValue(const CString& key, UInt32& outValue,
															UInt32 defaultValue = 0) const
														{ outValue = getUInt32(key, defaultValue); }
						void						getValue(const CString& key, UInt64& outValue,
															UInt64 defaultValue = 0) const
														{ outValue = getUInt64(key, defaultValue); }

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
						void						set(const CString& key, const SDictionaryValue& value);

	virtual				void						remove(const CString& key);
	virtual				void						removeAll();

						bool						equals(const CDictionary& other,
															void* itemCompareProcUserData = nil) const;

						TIteratorS<SDictionaryItem>	getIterator() const;

						CDictionary&				operator=(const CDictionary& other);
						CDictionary&				operator+=(const CDictionary& other);

	// Properties
	public:
		static	CDictionary				mEmpty;

	private:
				CDictionaryInternals*	mInternals;
};

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

struct SDictionaryValue {
	// Methods
	public:
										// Lifecycle methods
										SDictionaryValue(bool value);
										SDictionaryValue(const TArray<CDictionary>& value);
										SDictionaryValue(const TArray<CString>& value);
										SDictionaryValue(const CData& value);
										SDictionaryValue(const CDictionary& value);
										SDictionaryValue(const CString& value);
										SDictionaryValue(Float32 value);
										SDictionaryValue(Float64 value);
										SDictionaryValue(SInt8 value);
										SDictionaryValue(SInt16 value);
										SDictionaryValue(SInt32 value);
										SDictionaryValue(SInt64 value) ;
										SDictionaryValue(UInt8 value);
										SDictionaryValue(UInt16 value);
										SDictionaryValue(UInt32 value);
										SDictionaryValue(UInt64 value);
										SDictionaryValue(CDictionaryItemRef value);
										SDictionaryValue(const SDictionaryValue& other);	// No copy
										SDictionaryValue(const SDictionaryValue& other,
												CDictionaryItemCopyProc itemCopyProc);	// Copy

										// Instance methods
				EDictionaryValueType	getType() const { return mValueType; }

				bool					getBool(bool defaultValue = false) const;
		const	TNArray<CDictionary>&	getArrayOfDictionaries(
												const TNArray<CDictionary>& defaultValue = TNArray<CDictionary>())
												const;
		const	TNArray<CString>&		getArrayOfStrings(const TNArray<CString>& defaultValue = TNArray<CString>())
												const;
		const	CData&					getData(const CData& defaultValue = CData::mEmpty) const;
		const	CDictionary&			getDictionary(const CDictionary& defaultValue = CDictionary::mEmpty) const;
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
				CDictionaryItemRef		getItemRef() const;

				bool					equals(const SDictionaryValue& other,
												CDictionaryItemEqualsProc itemEqualsProc) const;

				void					dispose(CDictionaryItemDisposeProc itemDisposeProc);

	// Properties
	private:
		EDictionaryValueType	mValueType;
		union SDictionaryValueValue {
			SDictionaryValueValue(bool value) : mBool(value) {}
			SDictionaryValueValue(TNArray<CDictionary>* value) : mArrayOfDictionaries(value) {}
			SDictionaryValueValue(TNArray<CString>* value) : mArrayOfStrings(value) {}
			SDictionaryValueValue(CData* value) : mData(value) {}
			SDictionaryValueValue(CDictionary* value) : mDictionary(value) {}
			SDictionaryValueValue(CString* value) : mString(value) {}
			SDictionaryValueValue(Float32 value) : mFloat32(value) {}
			SDictionaryValueValue(Float64 value) : mFloat64(value) {}
			SDictionaryValueValue(SInt8 value) : mSInt8(value) {}
			SDictionaryValueValue(SInt16 value) : mSInt16(value) {}
			SDictionaryValueValue(SInt32 value) : mSInt32(value) {}
			SDictionaryValueValue(SInt64 value) : mSInt64(value) {}
			SDictionaryValueValue(UInt8 value) : mUInt8(value) {}
			SDictionaryValueValue(UInt16 value) : mUInt16(value) {}
			SDictionaryValueValue(UInt32 value) : mUInt32(value) {}
			SDictionaryValueValue(UInt64 value) : mUInt64(value) {}
			SDictionaryValueValue(CDictionaryItemRef value) : mItemRef(value) {}

			bool					mBool;
			TNArray<CDictionary>*	mArrayOfDictionaries;
			TNArray<CString>*		mArrayOfStrings;
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
	SDictionaryItem(const CString& key, const SDictionaryValue& value) :
		mKey(key), mValue(value)
		{}
	SDictionaryItem(const CString& key, const SDictionaryValue& value, CDictionaryItemCopyProc itemCopyProc) :
		mKey(key), mValue(value, itemCopyProc)
		{}
	SDictionaryItem(const SDictionaryItem& other, CDictionaryItemCopyProc itemCopyProc) :
		mKey(other.mKey), mValue(other.mValue, itemCopyProc)
		{}

	// Properties
	CString				mKey;
	SDictionaryValue	mValue;
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
		const	OV<T>	get(const CString& key) const
							{
								// Get itemRef
								OV<CDictionaryItemRef>	itemRef = CDictionary::getItemRef(key);

								return itemRef.hasValue() ? OV<T>((T) itemRef.getValue()) : OV<T>();
							}
				void	set(const CString& key, const T item)
							{ CDictionary::set(key, item); }

		const	OV<T>	operator[](const CString& key) const
							{ return get(key); }
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
		const	OR<T>	get(const CString& key) const
							{
								// Get itemRef
								OV<CDictionaryItemRef>	itemRef = CDictionary::getItemRef(key);

								return itemRef.hasValue() ? OR<T>(*((T*) itemRef.getValue())) : OR<T>();
							}
				void	set(const CString& key, const T& item)
							{ CDictionary::set(key, new T(item)); }

		const	OR<T>	operator[](const CString& key) const
							{ return get(key); }

	private:
						// Class methods
		static	T*		copy(CDictionaryItemRef itemRef)
							{ return new T(*((T*) itemRef)); }
		static	void	dispose(CDictionaryItemRef itemRef)
							{ T* t = (T*) itemRef; Delete(t); }
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
		const	T		get(K key) const
								{ return (T) CDictionary::getItemRef(CString(key)); }
				void	set(K key, const T item)
							{ CDictionary::set(CString(key), item); }

				void	remove(K key)
							{ CDictionary::remove(CString(key)); }

		const	T		operator[](K key) const
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
		const	OV<T>	get(const CString& key) const
							{
								// Get itemRef
								OV<CDictionaryItemRef>	itemRef = CDictionary::getItemRef(key);

								return itemRef.hasValue() ? OV<T>((T) itemRef.getValue()) : OV<T>();
							}
				void	set(K key, const T& item)
							{ CDictionary::set(CString(key), new T(item)); }

				void	remove(K key)
							{ CDictionary::remove(CString(key)); }

		const	OV<T>	operator[](const CString& key) const
							{ return get(key); }

	private:
						// Class methods
		static	T*		copy(CDictionaryItemRef itemRef)
							{ return new T(*((T*) itemRef)); }
		static	void	dispose(CDictionaryItemRef itemRef)
							{ T* t = (T*) itemRef; Delete(t); }
};
