//----------------------------------------------------------------------------------------------------------------------
//	SValue.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "SValue.h"

#include "CDictionary.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SValue

// MARK: Properties

SValue	SValue::mEmpty;

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
SValue::SValue() : mType(kEmpty), mValue(false)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SValue::SValue(bool value) : mType(kBool), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SValue::SValue(const TArray<CDictionary>& value) : mType(kArrayOfDictionaries), mValue(new TArray<CDictionary>(value))
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SValue::SValue(const TArray<CString>& value) : mType(kArrayOfStrings), mValue(new TArray<CString>(value))
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SValue::SValue(const CData& value) : mType(kData), mValue(new CData(value))
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SValue::SValue(const CDictionary& value) : mType(kDictionary), mValue(new CDictionary(value))
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SValue::SValue(const CString& value) : mType(kString), mValue(new CString(value))
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SValue::SValue(Float32 value) : mType(kFloat32), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SValue::SValue(Float64 value) : mType(kFloat64), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SValue::SValue(SInt8 value) : mType(kSInt8), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SValue::SValue(SInt16 value) : mType(kSInt16), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SValue::SValue(SInt32 value) : mType(kSInt32), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SValue::SValue(SInt64 value) : mType(kSInt64), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SValue::SValue(UInt8 value) : mType(kUInt8), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SValue::SValue(UInt16 value) : mType(kUInt16), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SValue::SValue(UInt32 value) : mType(kUInt32), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SValue::SValue(UInt64 value) : mType(kUInt64), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SValue::SValue(Opaque value) : mType(kOpaque), mValue(value)
//----------------------------------------------------------------------------------------------------------------------
{}

//----------------------------------------------------------------------------------------------------------------------
SValue::SValue(const SValue& other, OpaqueCopyProc opaqueCopyProc) : mType(other.mType), mValue(other.mValue)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check for ItemCopyProc
	if (opaqueCopyProc != nil) {
		// Copy value
		if (mType == kArrayOfDictionaries)
			// Array of dictionaries
			mValue.mArrayOfDictionaries = new TNArray<CDictionary>(*mValue.mArrayOfDictionaries);
		else if (mType == kArrayOfStrings)
			// Array of strings
			mValue.mArrayOfStrings = new TNArray<CString>(*mValue.mArrayOfStrings);
		else if (mType == kData)
			// Data
			mValue.mData = new CData(*mValue.mData);
		else if (mType == kDictionary)
			// Dictionary
			mValue.mDictionary = new CDictionary(*mValue.mDictionary);
		else if (mType == kString)
			// String
			mValue.mString = new CString(*mValue.mString);
		else if ((mType == kOpaque) && (opaqueCopyProc != nil))
			// Opaque and have copy proc
			mValue.mOpaque = opaqueCopyProc(mValue.mOpaque);
	}
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
bool SValue::getBool(bool defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mType) {
		case kBool:		return mValue.mBool;
		case kSInt8:	return mValue.mSInt8 == 1;
		case kSInt16:	return mValue.mSInt16 == 1;
		case kSInt32:	return mValue.mSInt32 == 1;
		case kSInt64:	return mValue.mSInt64 == 1;
		case kUInt8:	return mValue.mUInt8 == 1;
		case kUInt16:	return mValue.mUInt16 == 1;
		case kUInt32:	return mValue.mUInt32 == 1;
		case kUInt64:	return mValue.mUInt64 == 1;
		default:
			// Cannot coerce value
			AssertFail();

			return defaultValue;
	}
}

//----------------------------------------------------------------------------------------------------------------------
const TArray<CDictionary>& SValue::getArrayOfDictionaries(const TArray<CDictionary>& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Verify value type
	AssertFailIf(mType != kArrayOfDictionaries);

	return (mType == kArrayOfDictionaries) ? *mValue.mArrayOfDictionaries : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
const TArray<CString>& SValue::getArrayOfStrings(const TArray<CString>& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Verify value type
	AssertFailIf(mType != kArrayOfStrings);

	return (mType == kArrayOfStrings) ? *mValue.mArrayOfStrings : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
const CData& SValue::getData(const CData& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Verify value type
	AssertFailIf(mType != kData);

	return (mType == kData) ? *mValue.mData : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
const CDictionary& SValue::getDictionary(const CDictionary& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Verify value type
	AssertFailIf(mType != kDictionary);

	return (mType == kDictionary) ? *mValue.mDictionary : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
const CString& SValue::getString(const CString& defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Verify value type
	AssertFailIf(mType != kString);

	return (mType == kString) ? *mValue.mString : defaultValue;
}

//----------------------------------------------------------------------------------------------------------------------
Float32 SValue::getFloat32(Float32 defaultValue) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (mType) {
		case kFloat32:	return mValue.mFloat32;
		case kFloat64:	return (Float32) mValue.mFloat64;
		case kSInt8:	return mValue.mSInt8;
		case kSInt16:	return mValue.mSInt16;
		case kSInt32:	return (Float32) mValue.mSInt32;
		case kSInt64:	return (Float32) mValue.mSInt64;
		case kUInt8:	return mValue.mUInt8;
		case kUInt16:	return mValue.mUInt16;
		case kUInt32:	return (Float32) mValue.mUInt32;
		case kUInt64:	return (Float32) mValue.mUInt64;
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
		case kFloat32:	return mValue.mFloat32;
		case kFloat64:	return mValue.mFloat64;
		case kSInt8:	return mValue.mSInt8;
		case kSInt16:	return mValue.mSInt16;
		case kSInt32:	return mValue.mSInt32;
		case kSInt64:	return (Float64) mValue.mSInt64;
		case kUInt8:	return mValue.mUInt8;
		case kUInt16:	return mValue.mUInt16;
		case kUInt32:	return mValue.mUInt32;
		case kUInt64:	return (Float64) mValue.mUInt64;
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
		case kSInt8:	return mValue.mSInt8;
		case kSInt16:	return (SInt8) mValue.mSInt16;
		case kSInt32:	return (SInt8)mValue.mSInt32;
		case kSInt64:	return (SInt8)mValue.mSInt64;
		case kUInt8:	return mValue.mUInt8;
		case kUInt16:	return (SInt8)mValue.mUInt16;
		case kUInt32:	return (SInt8)mValue.mUInt32;
		case kUInt64:	return (SInt8)mValue.mUInt64;
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
		case kSInt8:	return mValue.mSInt8;
		case kSInt16:	return mValue.mSInt16;
		case kSInt32:	return mValue.mSInt32;
		case kSInt64:	return (SInt16) mValue.mSInt64;
		case kUInt8:	return mValue.mUInt8;
		case kUInt16:	return mValue.mUInt16;
		case kUInt32:	return mValue.mUInt32;
		case kUInt64:	return (SInt16) mValue.mUInt64;
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
		case kSInt8:	return mValue.mSInt8;
		case kSInt16:	return mValue.mSInt16;
		case kSInt32:	return mValue.mSInt32;
		case kSInt64:	return (SInt32) mValue.mSInt64;
		case kUInt8:	return mValue.mUInt8;
		case kUInt16:	return mValue.mUInt16;
		case kUInt32:	return mValue.mUInt32;
		case kUInt64:	return (SInt32) mValue.mUInt64;
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
		case kSInt8:	return mValue.mSInt8;
		case kSInt16:	return mValue.mSInt16;
		case kSInt32:	return mValue.mSInt32;
		case kSInt64:	return mValue.mSInt64;
		case kUInt8:	return mValue.mUInt8;
		case kUInt16:	return mValue.mUInt16;
		case kUInt32:	return mValue.mUInt32;
		case kUInt64:	return mValue.mUInt64;
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
		case kSInt8:	return mValue.mSInt8;
		case kSInt16:	return (UInt8) mValue.mSInt16;
		case kSInt32:	return (UInt8) mValue.mSInt32;
		case kSInt64:	return (UInt8) mValue.mSInt64;
		case kUInt8:	return mValue.mUInt8;
		case kUInt16:	return (UInt8) mValue.mUInt16;
		case kUInt32:	return (UInt8) mValue.mUInt32;
		case kUInt64:	return (UInt8) mValue.mUInt64;
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
		case kSInt8:	return mValue.mSInt8;
		case kSInt16:	return mValue.mSInt16;
		case kSInt32:	return mValue.mSInt32;
		case kSInt64:	return (UInt16) mValue.mSInt64;
		case kUInt8:	return mValue.mUInt8;
		case kUInt16:	return mValue.mUInt16;
		case kUInt32:	return mValue.mUInt32;
		case kUInt64:	return (UInt16) mValue.mUInt64;
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
		case kSInt8:	return mValue.mSInt8;
		case kSInt16:	return mValue.mSInt16;
		case kSInt32:	return mValue.mSInt32;
		case kSInt64:	return (UInt32) mValue.mSInt64;
		case kUInt8:	return mValue.mUInt8;
		case kUInt16:	return mValue.mUInt16;
		case kUInt32:	return mValue.mUInt32;
		case kUInt64:	return (UInt32) mValue.mUInt64;
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
		case kSInt8:	return mValue.mSInt8;
		case kSInt16:	return mValue.mSInt16;
		case kSInt32:	return mValue.mSInt32;
		case kSInt64:	return mValue.mSInt64;
		case kUInt8:	return mValue.mUInt8;
		case kUInt16:	return mValue.mUInt16;
		case kUInt32:	return mValue.mUInt32;
		case kUInt64:	return mValue.mUInt64;
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
	AssertFailIf(mType != kOpaque);

	return (mType == kOpaque) ? mValue.mOpaque : nil;
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
		case kEmpty:
			// Empty
			return true;

		case kBool:
			// Bool
			return mValue.mBool == other.mValue.mBool;

		case kArrayOfDictionaries:
			// Array of Dictionaries
			return *mValue.mArrayOfDictionaries == *other.mValue.mArrayOfDictionaries;

		case kArrayOfStrings:
			// Array of Strings
			return *mValue.mArrayOfStrings == *other.mValue.mArrayOfStrings;

		case kData:
			// Data
			return *mValue.mData == *other.mValue.mData;

		case kDictionary:
			// Dictionary
			return *mValue.mDictionary == *other.mValue.mDictionary;

		case kString:
			// String
			return *mValue.mString == *other.mValue.mString;

		case kFloat32:
			// Float32
			return mValue.mFloat32 == other.mValue.mFloat32;

		case kFloat64:
			// Float64
			return mValue.mFloat64 == other.mValue.mFloat64;

		case kSInt8:
			// SInt8
			return mValue.mSInt8 == other.mValue.mSInt8;

		case kSInt16:
			// SInt16
			return mValue.mSInt16 == other.mValue.mSInt16;

		case kSInt32:
			// SInt32
			return mValue.mSInt32 == other.mValue.mSInt32;

		case kSInt64:
			// SInt64
			return mValue.mSInt64 == other.mValue.mSInt64;

		case kUInt8:
			// UInt8
			return mValue.mUInt8 == other.mValue.mUInt8;

		case kUInt16:
			// UInt16
			return mValue.mUInt16 == other.mValue.mUInt16;

		case kUInt32:
			// UInt32
			return mValue.mUInt32 == other.mValue.mUInt32;

		case kUInt64:
			// UInt64
			return mValue.mUInt64 == other.mValue.mUInt64;

		case kOpaque:
			// ItemRef
			return (opaqueEqualsProc != nil) ?
					opaqueEqualsProc(mValue.mOpaque, other.mValue.mOpaque) : mValue.mOpaque == other.mValue.mOpaque;

#if TARGET_OS_WINDOWS
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
	if (mType == kArrayOfDictionaries) {
		// Array of dictionaries
		Delete(mValue.mArrayOfDictionaries);
	} else if (mType == kArrayOfStrings) {
		// Array of strings
		Delete(mValue.mArrayOfStrings);
	} else if (mType == kData) {
		// Data
		Delete(mValue.mData);
	} else if (mType == kDictionary) {
		// Dictionary
		Delete(mValue.mDictionary);
	} else if (mType == kString) {
		// String
		Delete(mValue.mString);
	} else if ((mType == kOpaque) && (opaqueDisposeProc != nil)) {
		// Item Ref and have item dispose proc
		opaqueDisposeProc(mValue.mOpaque);
	}
}

//----------------------------------------------------------------------------------------------------------------------
const CDictionary& SValue::getEmptyDictionary()
//----------------------------------------------------------------------------------------------------------------------
{
	return CDictionary::mEmpty;
}
