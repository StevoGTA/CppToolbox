//----------------------------------------------------------------------------------------------------------------------
//	SSQLiteValue.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CData.h"
#include "CString.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SSQLiteValue

struct SSQLiteValue {
	// Type
	enum Type {
		kData,
		kFloat64,
		kInt64,
		kString,

		kLastInsertRowID,
		kNull,
	};

	enum NonValueTypeKind {
		kLastInsertRowIDKind,
		kNullKind,
	};

									// Lifecycle methods
									SSQLiteValue(const CData& data) : mType(kData), mValue(data) {}
									SSQLiteValue(Float64 float64) : mType(kFloat64), mValue(float64) {}
									SSQLiteValue(SInt64 sInt64) : mType(kInt64), mValue(sInt64) {}
									SSQLiteValue(UInt32 uInt32) : mType(kInt64), mValue((SInt64) uInt32) {}
									SSQLiteValue(const CString& string) : mType(kString), mValue(string) {}
									SSQLiteValue(NonValueTypeKind nonValueTypeKind) : mValue((SInt64) 0)
										{
											// Check kind
											switch (nonValueTypeKind) {
												case kLastInsertRowIDKind:	mType = kLastInsertRowID;	break;
												case kNullKind:				mType = kNull;				break;
											}
										}
									SSQLiteValue(const SSQLiteValue& other) :
										mType(other.mType), mValue(other.mValue)
										{
											if (mType == kString)
												// String
												mValue.mString = new CString(*mValue.mString);
											else if (mType == kData)
												// Data
												mValue.mData = new CData(*mValue.mData);
										}
									~SSQLiteValue()
										{
											// Check type
											if (mType == kString) {
												// String
												Delete(mValue.mString);
											} else if (mType == kData) {
												// Data
												Delete(mValue.mData);
											}
										}

									// Instance methods
			CString					getString()
										{
											// Check type
											switch (mType) {
												case kFloat64:	return CString(mValue.mFloat64);
												case kInt64:	return CString(mValue.mInt64);
												case kString:	return *mValue.mString;
												case kNull:		return CString(OSSTR("NULL"));
												default:		AssertFail();	return CString::mEmpty;
											}
										}

									// Class methods
	static	TMArray<SSQLiteValue>	valuesFrom(const TArray<CString>& values)
										{
											// Convert array
											TNArray<SSQLiteValue>	sqliteValues;
											CArray::ItemCount		count = values.getCount();
											for (CArray::ItemIndex i = 0; i < count; i++)
												// Add value
												sqliteValues += SSQLiteValue(values[i]);

											return sqliteValues;
										}
	static	TMArray<SSQLiteValue>	valuesFrom(const TNumberArray<SInt64>& values)
										{
											// Convert array
											TNArray<SSQLiteValue>	sqliteValues;
											CArray::ItemCount		count = values.getCount();
											for (CArray::ItemIndex i = 0; i < count; i++)
												// Add value
												sqliteValues += SSQLiteValue(values[i]);

											return sqliteValues;
										}

	// Properties
	Type	mType;
	union ValueValue {
		ValueValue(const CData& data) : mData(new CData(data)) {}
		ValueValue(Float64 float64) : mFloat64(float64) {}
		ValueValue(SInt64 sInt64) : mInt64(sInt64) {}
		ValueValue(const CString& string) : mString(new CString(string)) {}

		SInt64		mInt64;
		Float64		mFloat64;
		CString*	mString;
		CData*		mData;
	}		mValue;
};
