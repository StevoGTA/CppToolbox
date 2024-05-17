//----------------------------------------------------------------------------------------------------------------------
//	SValue.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "SValue.h"

#include "CDictionary.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SValue

// MARK: Properties

const	SValue	SValue::mEmpty;

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
SValue::SValue() : mType(kTypeEmpty), mValue(false)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SValue::SValue(bool value) : mType(kTypeBool), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SValue::SValue(const TArray<CDictionary>& value) : mType(kTypeArrayOfDictionaries), mValue(new TArray<CDictionary>(value))
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SValue::SValue(const TArray<CString>& value) : mType(kTypeArrayOfStrings), mValue(new TArray<CString>(value))
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SValue::SValue(const CData& value) : mType(kTypeData), mValue(new CData(value))
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SValue::SValue(const CDictionary& value) : mType(kTypeDictionary), mValue(new CDictionary(value))
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SValue::SValue(const CString& value) : mType(kTypeString), mValue(new CString(value))
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SValue::SValue(Float32 value) : mType(kTypeFloat32), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SValue::SValue(Float64 value) : mType(kTypeFloat64), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SValue::SValue(SInt8 value) : mType(kTypeSInt8), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SValue::SValue(SInt16 value) : mType(kTypeSInt16), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SValue::SValue(SInt32 value) : mType(kTypeSInt32), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SValue::SValue(SInt64 value) : mType(kTypeSInt64), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SValue::SValue(UInt8 value) : mType(kTypeUInt8), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SValue::SValue(UInt16 value) : mType(kTypeUInt16), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SValue::SValue(UInt32 value) : mType(kTypeUInt32), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SValue::SValue(UInt64 value) : mType(kTypeUInt64), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SValue::SValue(Opaque value) : mType(kTypeOpaque), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SValue::SValue(const SValue& other, OpaqueCopyProc opaqueCopyProc) : mType(other.mType), mValue(false)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mType) {
		case kTypeArrayOfDictionaries:
			// Array of dictionaries
			mValue.mArrayOfDictionaries = new TArray<CDictionary>(*other.mValue.mArrayOfDictionaries);
			break;

		case kTypeArrayOfStrings:
			// Array of strings
			mValue.mArrayOfStrings = new TArray<CString>(*other.mValue.mArrayOfStrings);
			break;

		case kTypeData:
			// Data
			mValue.mData = new CData(*other.mValue.mData);
			break;

		case kTypeDictionary:
			// Dictionary
			mValue.mDictionary = new CDictionary(*other.mValue.mDictionary);
			break;

		case kTypeString:
			// String
			mValue.mString = new CString(*other.mValue.mString);
			break;

		case kTypeEmpty:
		case kTypeBool:
		case kTypeFloat32:
		case kTypeFloat64:
		case kTypeSInt8:
		case kTypeSInt16:
		case kTypeSInt32:
		case kTypeSInt64:
		case kTypeUInt8:
		case kTypeUInt16:
		case kTypeUInt32:
		case kTypeUInt64:
			// Can copy
			mValue = other.mValue;
			break;

		case kTypeOpaque:
			// Opaque
			mValue = (opaqueCopyProc != nil) ?  opaqueCopyProc(other.mValue.mOpaque) : other.mValue;
			break;
	}
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
bool SValue::canCoerceToType(Type type) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check requsted type
	switch (type) {
		case kTypeFloat32:
		case kTypeFloat64:
			// Check current type
			switch (mType) {
				case kTypeFloat32:
				case kTypeFloat64:
				case kTypeSInt8:
				case kTypeSInt16:
				case kTypeSInt32:
				case kTypeSInt64:
				case kTypeUInt8:
				case kTypeUInt16:
				case kTypeUInt32:
				case kTypeUInt64:
					// Yes
					return true;

				default:
					// No
					return false;
			}

		case kTypeSInt8:
		case kTypeSInt16:
		case kTypeSInt32:
		case kTypeSInt64:
		case kTypeUInt8:
		case kTypeUInt16:
		case kTypeUInt32:
		case kTypeUInt64:
			// Check current type
			switch (mType) {
				case kTypeSInt8:
				case kTypeSInt16:
				case kTypeSInt32:
				case kTypeSInt64:
				case kTypeUInt8:
				case kTypeUInt16:
				case kTypeUInt32:
				case kTypeUInt64:
					// Yes
					return true;

				default:
					// No
					return false;
			}

		default:
			// Cannot coerce value
			return false;
	};
}

//----------------------------------------------------------------------------------------------------------------------
const TArray<CDictionary>& SValue::getArrayOfDictionaries(const TArray<CDictionary>& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Verify value type
	AssertFailIf(mType != kTypeArrayOfDictionaries);

	return (mType == kTypeArrayOfDictionaries) ? *mValue.mArrayOfDictionaries : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
const TArray<CString>& SValue::getArrayOfStrings(const TArray<CString>& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Verify value type
	AssertFailIf(mType != kTypeArrayOfStrings);

	return (mType == kTypeArrayOfStrings) ? *mValue.mArrayOfStrings : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
bool SValue::getBool(bool defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mType) {
		case kTypeBool:		return mValue.mBool;
		case kTypeSInt8:	return mValue.mSInt8 == 1;
		case kTypeSInt16:	return mValue.mSInt16 == 1;
		case kTypeSInt32:	return mValue.mSInt32 == 1;
		case kTypeSInt64:	return mValue.mSInt64 == 1;
		case kTypeUInt8:	return mValue.mUInt8 == 1;
		case kTypeUInt16:	return mValue.mUInt16 == 1;
		case kTypeUInt32:	return mValue.mUInt32 == 1;
		case kTypeUInt64:	return mValue.mUInt64 == 1;
		default:
			// Cannot coerce value
			AssertFail();

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
const CData& SValue::getData(const CData& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Verify value type
	AssertFailIf(mType != kTypeData);

	return (mType == kTypeData) ? *mValue.mData : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
const CDictionary& SValue::getDictionary(const CDictionary& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Verify value type
	AssertFailIf(mType != kTypeDictionary);

	return (mType == kTypeDictionary) ? *mValue.mDictionary : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
const CString& SValue::getString(const CString& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Verify value type
	AssertFailIf(mType != kTypeString);

	return (mType == kTypeString) ? *mValue.mString : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
Float32 SValue::getFloat32(Float32 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mType) {
		case kTypeFloat32:	return mValue.mFloat32;
		case kTypeFloat64:	return (Float32) mValue.mFloat64;
		case kTypeSInt8:	return mValue.mSInt8;
		case kTypeSInt16:	return mValue.mSInt16;
		case kTypeSInt32:	return (Float32) mValue.mSInt32;
		case kTypeSInt64:	return (Float32) mValue.mSInt64;
		case kTypeUInt8:	return mValue.mUInt8;
		case kTypeUInt16:	return mValue.mUInt16;
		case kTypeUInt32:	return (Float32) mValue.mUInt32;
		case kTypeUInt64:	return (Float32) mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFail();

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
Float64 SValue::getFloat64(Float64 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mType) {
		case kTypeFloat32:	return mValue.mFloat32;
		case kTypeFloat64:	return mValue.mFloat64;
		case kTypeSInt8:	return mValue.mSInt8;
		case kTypeSInt16:	return mValue.mSInt16;
		case kTypeSInt32:	return mValue.mSInt32;
		case kTypeSInt64:	return (Float64) mValue.mSInt64;
		case kTypeUInt8:	return mValue.mUInt8;
		case kTypeUInt16:	return mValue.mUInt16;
		case kTypeUInt32:	return mValue.mUInt32;
		case kTypeUInt64:	return (Float64) mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFail();

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
SInt8 SValue::getSInt8(SInt8 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mType) {
		case kTypeSInt8:	return mValue.mSInt8;
		case kTypeSInt16:	return (SInt8) mValue.mSInt16;
		case kTypeSInt32:	return (SInt8)mValue.mSInt32;
		case kTypeSInt64:	return (SInt8)mValue.mSInt64;
		case kTypeUInt8:	return mValue.mUInt8;
		case kTypeUInt16:	return (SInt8)mValue.mUInt16;
		case kTypeUInt32:	return (SInt8)mValue.mUInt32;
		case kTypeUInt64:	return (SInt8)mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFail();

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
SInt16 SValue::getSInt16(SInt16 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mType) {
		case kTypeSInt8:	return mValue.mSInt8;
		case kTypeSInt16:	return mValue.mSInt16;
		case kTypeSInt32:	return (SInt16) mValue.mSInt32;
		case kTypeSInt64:	return (SInt16) mValue.mSInt64;
		case kTypeUInt8:	return mValue.mUInt8;
		case kTypeUInt16:	return mValue.mUInt16;
		case kTypeUInt32:	return (SInt16) mValue.mUInt32;
		case kTypeUInt64:	return (SInt16) mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFail();

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
SInt32 SValue::getSInt32(SInt32 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mType) {
		case kTypeSInt8:	return mValue.mSInt8;
		case kTypeSInt16:	return mValue.mSInt16;
		case kTypeSInt32:	return mValue.mSInt32;
		case kTypeSInt64:	return (SInt32) mValue.mSInt64;
		case kTypeUInt8:	return mValue.mUInt8;
		case kTypeUInt16:	return mValue.mUInt16;
		case kTypeUInt32:	return mValue.mUInt32;
		case kTypeUInt64:	return (SInt32) mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFail();

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
SInt64 SValue::getSInt64(SInt64 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mType) {
		case kTypeSInt8:	return mValue.mSInt8;
		case kTypeSInt16:	return mValue.mSInt16;
		case kTypeSInt32:	return mValue.mSInt32;
		case kTypeSInt64:	return mValue.mSInt64;
		case kTypeUInt8:	return mValue.mUInt8;
		case kTypeUInt16:	return mValue.mUInt16;
		case kTypeUInt32:	return mValue.mUInt32;
		case kTypeUInt64:	return mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFail();

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
UInt8 SValue::getUInt8(UInt8 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mType) {
		case kTypeSInt8:	return mValue.mSInt8;
		case kTypeSInt16:	return (UInt8) mValue.mSInt16;
		case kTypeSInt32:	return (UInt8) mValue.mSInt32;
		case kTypeSInt64:	return (UInt8) mValue.mSInt64;
		case kTypeUInt8:	return mValue.mUInt8;
		case kTypeUInt16:	return (UInt8) mValue.mUInt16;
		case kTypeUInt32:	return (UInt8) mValue.mUInt32;
		case kTypeUInt64:	return (UInt8) mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFail();

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
UInt16 SValue::getUInt16(UInt16 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mType) {
		case kTypeSInt8:	return mValue.mSInt8;
		case kTypeSInt16:	return mValue.mSInt16;
		case kTypeSInt32:	return (UInt16) mValue.mSInt32;
		case kTypeSInt64:	return (UInt16) mValue.mSInt64;
		case kTypeUInt8:	return mValue.mUInt8;
		case kTypeUInt16:	return mValue.mUInt16;
		case kTypeUInt32:	return (UInt16) mValue.mUInt32;
		case kTypeUInt64:	return (UInt16) mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFail();

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
UInt32 SValue::getUInt32(UInt32 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mType) {
		case kTypeSInt8:	return mValue.mSInt8;
		case kTypeSInt16:	return mValue.mSInt16;
		case kTypeSInt32:	return mValue.mSInt32;
		case kTypeSInt64:	return (UInt32) mValue.mSInt64;
		case kTypeUInt8:	return mValue.mUInt8;
		case kTypeUInt16:	return mValue.mUInt16;
		case kTypeUInt32:	return mValue.mUInt32;
		case kTypeUInt64:	return (UInt32) mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFail();

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 SValue::getUInt64(UInt64 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mType) {
		case kTypeSInt8:	return mValue.mSInt8;
		case kTypeSInt16:	return mValue.mSInt16;
		case kTypeSInt32:	return mValue.mSInt32;
		case kTypeSInt64:	return mValue.mSInt64;
		case kTypeUInt8:	return mValue.mUInt8;
		case kTypeUInt16:	return mValue.mUInt16;
		case kTypeUInt32:	return mValue.mUInt32;
		case kTypeUInt64:	return mValue.mUInt64;
		default:
			// Cannot coerce value
			AssertFail();

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
SValue::Opaque SValue::getOpaque() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Verify value type
	AssertFailIf(mType != kTypeOpaque);

	return (mType == kTypeOpaque) ? mValue.mOpaque : nil;
}

//----------------------------------------------------------------------------------------------------------------------
bool SValue::equals(const SValue& other, OpaqueEqualsProc opaqueEqualsProc) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	if (mType != other.mType)
		// Mismatch
		return false;

	switch (mType) {
		case kTypeEmpty:
			// Empty
			return true;

		case kTypeBool:
			// Bool
			return mValue.mBool == other.mValue.mBool;

		case kTypeArrayOfDictionaries:
			// Array of Dictionaries
			return *mValue.mArrayOfDictionaries == *other.mValue.mArrayOfDictionaries;

		case kTypeArrayOfStrings:
			// Array of Strings
			return *mValue.mArrayOfStrings == *other.mValue.mArrayOfStrings;

		case kTypeData:
			// Data
			return *mValue.mData == *other.mValue.mData;

		case kTypeDictionary:
			// Dictionary
			return *mValue.mDictionary == *other.mValue.mDictionary;

		case kTypeString:
			// String
			return *mValue.mString == *other.mValue.mString;

		case kTypeFloat32:
			// Float32
			return mValue.mFloat32 == other.mValue.mFloat32;

		case kTypeFloat64:
			// Float64
			return mValue.mFloat64 == other.mValue.mFloat64;

		case kTypeSInt8:
			// SInt8
			return mValue.mSInt8 == other.mValue.mSInt8;

		case kTypeSInt16:
			// SInt16
			return mValue.mSInt16 == other.mValue.mSInt16;

		case kTypeSInt32:
			// SInt32
			return mValue.mSInt32 == other.mValue.mSInt32;

		case kTypeSInt64:
			// SInt64
			return mValue.mSInt64 == other.mValue.mSInt64;

		case kTypeUInt8:
			// UInt8
			return mValue.mUInt8 == other.mValue.mUInt8;

		case kTypeUInt16:
			// UInt16
			return mValue.mUInt16 == other.mValue.mUInt16;

		case kTypeUInt32:
			// UInt32
			return mValue.mUInt32 == other.mValue.mUInt32;

		case kTypeUInt64:
			// UInt64
			return mValue.mUInt64 == other.mValue.mUInt64;

		case kTypeOpaque:
			// ItemRef
			return (opaqueEqualsProc != nil) ?
					opaqueEqualsProc(mValue.mOpaque, other.mValue.mOpaque) : mValue.mOpaque == other.mValue.mOpaque;

#if defined(TARGET_OS_WINDOWS)
		default:
			// Just to make compiler happy.  Will never get here.
			return false;
#endif
	}
}

//----------------------------------------------------------------------------------------------------------------------
void SValue::dispose(OpaqueDisposeProc opaqueDisposeProc)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	if (mType == kTypeArrayOfDictionaries) {
		// Array of dictionaries
		Delete(mValue.mArrayOfDictionaries);
	} else if (mType == kTypeArrayOfStrings) {
		// Array of strings
		Delete(mValue.mArrayOfStrings);
	} else if (mType == kTypeData) {
		// Data
		Delete(mValue.mData);
	} else if (mType == kTypeDictionary) {
		// Dictionary
		Delete(mValue.mDictionary);
	} else if (mType == kTypeString) {
		// String
		Delete(mValue.mString);
	} else if ((mType == kTypeOpaque) && (opaqueDisposeProc != nil)) {
		// Item Ref and have item dispose proc
		opaqueDisposeProc(mValue.mOpaque);
	}
}

//----------------------------------------------------------------------------------------------------------------------
SValue& SValue::operator=(const SValue& other)
//----------------------------------------------------------------------------------------------------------------------
{
	// Copy type
	mType = other.mType;

	// Check value type
	switch (mType) {
		case kTypeArrayOfDictionaries:
			// Array of dictionaries
			mValue.mArrayOfDictionaries = new TArray<CDictionary>(*other.mValue.mArrayOfDictionaries);
			break;

		case kTypeArrayOfStrings:
			// Array of strings
			mValue.mArrayOfStrings = new TArray<CString>(*other.mValue.mArrayOfStrings);
			break;

		case kTypeData:
			// Data
			mValue.mData = new CData(*other.mValue.mData);
			break;

		case kTypeDictionary:
			// Dictionary
			mValue.mDictionary = new CDictionary(*other.mValue.mDictionary);
			break;

		case kTypeString:
			// String
			mValue.mString = new CString(*other.mValue.mString);
			break;

		case kTypeEmpty:
		case kTypeBool:
		case kTypeFloat32:
		case kTypeFloat64:
		case kTypeSInt8:
		case kTypeSInt16:
		case kTypeSInt32:
		case kTypeSInt64:
		case kTypeUInt8:
		case kTypeUInt16:
		case kTypeUInt32:
		case kTypeUInt64:
		case kTypeOpaque:
			// Can copy
			mValue = other.mValue;
			break;
	}

	return *this;
}

//----------------------------------------------------------------------------------------------------------------------
const CDictionary& SValue::getEmptyDictionary()
//----------------------------------------------------------------------------------------------------------------------
{
	return CDictionary::mEmpty;
}
