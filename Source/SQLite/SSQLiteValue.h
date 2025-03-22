//----------------------------------------------------------------------------------------------------------------------
//	SSQLiteValue.h			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CSet.h"
#include "SValue.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SSQLiteValue

struct SSQLiteValue {
	// Type
	public:
		enum Type {
			kTypeData,
			kTypeFloat64,
			kTypeSInt64,
			kTypeString,

			kTypeLastInsertRowID,
			kTypeNull,
		};

	// NonValueTypeKind
	public:
		enum NonValueTypeKind {
			kNonValueTypeKindLastInsertRowID,
			kNonValueTypeKindNull,
		};

	// Methods
	public:
												// Lifecycle methods
												SSQLiteValue(const CData& data) : mType(kTypeData), mValue(data) {}
												SSQLiteValue(Float64 float64) : mType(kTypeFloat64), mValue(float64) {}
												SSQLiteValue(SInt64 sInt64) : mType(kTypeSInt64), mValue(sInt64) {}
												SSQLiteValue(UInt32 uInt32) :
													mType(kTypeSInt64), mValue((SInt64) uInt32)
													{}
												SSQLiteValue(const CString& string) :
													mType(kTypeString), mValue(string)
													{}
												SSQLiteValue(const SValue& value) : mValue((SInt64) 0)
													{
														// Check value type
														if (value.getType() == SValue::kTypeData) {
															// Data
															mType = kTypeData;
															mValue.mData = new CData(value.getData());
														} else if (value.getType() == SValue::kTypeString) {
															// String
															mType = kTypeString;
															mValue.mString = new CString(value.getString());
														} else if ((value.getType() == SValue::kTypeFloat32) ||
																(value.getType() == SValue::kTypeFloat64)) {
															// Float64
															mType = kTypeFloat64;
															mValue.mFloat64 = value.getFloat64();
														} else if (value.canCoerceToType(SValue::kTypeSInt64)) {
															// SInt64
															mType = kTypeSInt64;
															mValue.mSInt64 = value.getSInt64();
														} else
															// Null
															mType = kTypeNull;
													}
												SSQLiteValue(NonValueTypeKind nonValueTypeKind) : mValue((SInt64) 0)
													{
														// Check kind
														switch (nonValueTypeKind) {
															case kNonValueTypeKindLastInsertRowID:
																// Last Insert Row ID
																mType = kTypeLastInsertRowID;
																break;

															case kNonValueTypeKindNull:
																// Null
																mType = kTypeNull;
																break;
														}
													}
												SSQLiteValue(const SSQLiteValue& other) :
													mType(other.mType), mValue(other.mValue)
													{
														if (mType == kTypeString)
															// String
															mValue.mString = new CString(*mValue.mString);
														else if (mType == kTypeData)
															// Data
															mValue.mData = new CData(*mValue.mData);
													}
												~SSQLiteValue()
													{
														// Check type
														if (mType == kTypeString) {
															// String
															Delete(mValue.mString);
														} else if (mType == kTypeData) {
															// Data
															Delete(mValue.mData);
														}
													}

												// Instance methods
						Type					getType() const
													{ return mType; }
				const	CData&					getData() const
													{ return *mValue.mData; }
						Float64					getFloat64() const
													{ return mValue.mFloat64; }
						SInt64					getSInt64() const
													{ return mValue.mSInt64; }
				const	CString&				getString() const
													{ return *mValue.mString; }

						CString					getString()
													{
														// Check type
														switch (mType) {
															case kTypeFloat64:	return CString(mValue.mFloat64);
															case kTypeSInt64:	return CString(mValue.mSInt64);
															case kTypeString:	return *mValue.mString;
															case kTypeNull:		return CString(OSSTR("NULL"));
															default:			AssertFail();	return CString::mEmpty;
														}
													}

												// Class methods
		static			TArray<SSQLiteValue>	valuesFrom(const TArray<CString>& values)
													{
														// Convert array
														TNArray<SSQLiteValue>	sqliteValues;
														CArray::ItemCount		count = values.getCount();
														for (CArray::ItemIndex i = 0; i < count; i++)
															// Add value
															sqliteValues += SSQLiteValue(values[i]);

														return sqliteValues;
													}
		static			TArray<SSQLiteValue>	valuesFrom(const TNumberArray<SInt64>& values)
													{
														// Convert array
														TNArray<SSQLiteValue>	sqliteValues;
														CArray::ItemCount		count = values.getCount();
														for (CArray::ItemIndex i = 0; i < count; i++)
															// Add value
															sqliteValues += SSQLiteValue(values[i]);

														return sqliteValues;
													}
		static			TArray<SSQLiteValue>	valuesFrom(const TSet<CString>& values)
													{
														// Convert array
														TNArray<SSQLiteValue>	sqliteValues;
														for (TIteratorS<CString> iterator = values.getIterator();
																iterator.hasValue(); iterator.advance())
															// Add value
															sqliteValues += SSQLiteValue(*iterator);

														return sqliteValues;
													}

	// Properties
	private:
		Type	mType;
		union ValueValue {
			ValueValue(const CData& data) : mData(new CData(data)) {}
			ValueValue(Float64 float64) : mFloat64(float64) {}
			ValueValue(SInt64 sInt64) : mSInt64(sInt64) {}
			ValueValue(const CString& string) : mString(new CString(string)) {}

			CData*		mData;
			Float64		mFloat64;
			SInt64		mSInt64;
			CString*	mString;
		} mValue;
};
