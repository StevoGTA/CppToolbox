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
		kInt64,
		kFloat64,
		kString,
		kData,
		kLastInsertRowID,
		kNull,
	};

	enum NonValueTypeKind {
		kLastInsertRowIDKind,
		kNullKind,
	};

											// Lifecycle methods
											SSQLiteValue(SInt64 sInt64) : mType(kInt64), mValue(sInt64) {}
											SSQLiteValue(Float64 float64) : mType(kFloat64), mValue(float64) {}
											SSQLiteValue(const CString& string) : mType(kString), mValue(string) {}
											SSQLiteValue(const CData& data) : mType(kData), mValue(data) {}
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
			CString							getString()
												{
													// Check type
													switch (mType) {
														case kInt64:	return CString(mValue.mInt64);
														case kFloat64:	return CString(mValue.mFloat64);
														case kString:	return *mValue.mString;
														case kNull:		return CString(OSSTR("NULL"));
														default:		AssertFail();	return CString::mEmpty;
													}
												}

											// Class methods
	static	TArray<TArray<SSQLiteValue> >	chunked(const TArray<SSQLiteValue>& values, UInt32 chunkSize)
												{
													// Setup
													TNArray<TArray<SSQLiteValue> >	arrays;
													CArray::ItemCount				count = values.getCount();

													// Iterate all values
													TNArray<SSQLiteValue>	array;
													for (CArray::ItemIndex i = 0; i < count; i++) {
														// Add value to chunk array
														array += values[i];

														// Check chunk array size
														if (array.getCount() == chunkSize) {
															// At max, add to arrays and reset
															arrays += array;
															array = TNArray<SSQLiteValue>();
														}
													}

													// Check if have any remaining
													if (!array.isEmpty())
														// Add to arrays
														arrays += array;

													return arrays;
												}

	// Properties
	Type	mType;
	union ValueValue {
		ValueValue(SInt64 sInt64) : mInt64(sInt64) {}
		ValueValue(Float64 float64) : mFloat64(float64) {}
		ValueValue(const CString& string) : mString(new CString(string)) {}
		ValueValue(const CData& data) : mData(new CData(data)) {}

		SInt64		mInt64;
		Float64		mFloat64;
		CString*	mString;
		CData*		mData;
	}		mValue;
};
