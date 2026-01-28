//----------------------------------------------------------------------------------------------------------------------
//	CTableColumn.h			©2025 Stevo Brock		All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CTableColumn.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CTableColumn::Internals

class CTableColumn::Internals {
	public:
		Internals(const CString& identifier, UInt16 minWidth, UInt16 maxWidth, UInt16 defaultWidth,
				bool isInitiallyDisplayed, bool isEditable) :
			mIdentifier(identifier),
					mMinWidth(minWidth), mMaxWidth(maxWidth), mWidth(defaultWidth),
					mIsDisplayed(isInitiallyDisplayed), mIsEditable(isEditable)
			{
			}

		CString		mIdentifier;

		UInt16		mMinWidth;
		UInt16		mMaxWidth;
		UInt16		mWidth;

		bool		mIsDisplayed;
		bool		mIsEditable;

		OV<Procs>	mProcs;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: CTableColumn

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CTableColumn::CTableColumn(const CString& identifier, UInt16 minWidth, UInt16 maxWidth, UInt16 defaultWidth,
		bool isInitiallyDisplayed, bool isEditable)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(identifier, minWidth, maxWidth, defaultWidth, isInitiallyDisplayed, isEditable);
}

//----------------------------------------------------------------------------------------------------------------------
CTableColumn::~CTableColumn()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
const CString& CTableColumn::getIdentifier() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mIdentifier;
}

//----------------------------------------------------------------------------------------------------------------------
UInt16 CTableColumn::getMinWidth() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mMinWidth;
}

//----------------------------------------------------------------------------------------------------------------------
UInt16 CTableColumn::getMaxWidth() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mMaxWidth;
}

//----------------------------------------------------------------------------------------------------------------------
UInt16 CTableColumn::getWidth() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mWidth;
}

//----------------------------------------------------------------------------------------------------------------------
void CTableColumn::setWidth(UInt16 width)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mWidth = width;
}

//----------------------------------------------------------------------------------------------------------------------
bool CTableColumn::isDisplayed() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mIsDisplayed;
}

//----------------------------------------------------------------------------------------------------------------------
void CTableColumn::setIsDisplayed(bool isDisplayed)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mIsDisplayed = isDisplayed;
}

//----------------------------------------------------------------------------------------------------------------------
bool CTableColumn::isEditable() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mIsEditable;
}

//----------------------------------------------------------------------------------------------------------------------
void CTableColumn::setProcs(const Procs& procs)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mProcs.setValue(procs);
}

//----------------------------------------------------------------------------------------------------------------------
CTableColumn::ValueCompareResult CTableColumn::compare(const SValue& value1, const SValue& value2) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check value type
	switch (value1.getType()) {
		case SValue::kTypeBool: {
			// Bool
			bool	bool1 = value1.getBool();
			bool	bool2 = value2.getBool();
			if (!bool1 && bool2)
				// Ascending
				return CTableColumn::kValueCompareResultAscending;
			else if (bool1 && !bool2)
				// Descending
				return CTableColumn::kValueCompareResultDescending;
			else
				// Same
				return CTableColumn::kValueCompareResultSame;
		}

		case SValue::kTypeString: {
			// String
			const	CString&	string1 = value1.getString();
			const	CString&	string2 = value2.getString();
			if (string1.compareTo(string2, CString::kCompareToOptionsSortDefault))
				// Ascending
				return CTableColumn::kValueCompareResultAscending;
			else if (string2.compareTo(string1, CString::kCompareToOptionsSortDefault))
				// Descending
				return CTableColumn::kValueCompareResultDescending;
			else
				// Same
				return CTableColumn::kValueCompareResultSame;
		}

		case SValue::kTypeFloat32: {
			// Float32
			Float32	float32_1 = value1.getFloat32();
			Float32	float32_2 = value2.getFloat32();
			if (float32_1 < float32_2)
				// Ascending
				return CTableColumn::kValueCompareResultAscending;
			else if (float32_1 > float32_2)
				// Descending
				return CTableColumn::kValueCompareResultDescending;
			else
				// Same
				return CTableColumn::kValueCompareResultSame;
		}

		case SValue::kTypeFloat64: {
			// Float64
			Float64	float64_1 = value1.getFloat64();
			Float64	float64_2 = value2.getFloat64();
			if (float64_1 < float64_2)
				// Ascending
				return CTableColumn::kValueCompareResultAscending;
			else if (float64_1 > float64_2)
				// Descending
				return CTableColumn::kValueCompareResultDescending;
			else
				// Same
				return CTableColumn::kValueCompareResultSame;
		}

		case SValue::kTypeSInt8: {
			// SInt8
			SInt8	sInt8_1 = value1.getSInt8();
			SInt8	sInt8_2 = value2.getSInt8();
			if (sInt8_1 < sInt8_2)
				// Ascending
				return CTableColumn::kValueCompareResultAscending;
			else if (sInt8_1 > sInt8_2)
				// Descending
				return CTableColumn::kValueCompareResultDescending;
			else
				// Same
				return CTableColumn::kValueCompareResultSame;
		}

		case SValue::kTypeSInt16: {
			// SInt16
			SInt16	sInt16_1 = value1.getSInt16();
			SInt16	sInt16_2 = value2.getSInt16();
			if (sInt16_1 < sInt16_2)
				// Ascending
				return CTableColumn::kValueCompareResultAscending;
			else if (sInt16_1 > sInt16_2)
				// Descending
				return CTableColumn::kValueCompareResultDescending;
			else
				// Same
				return CTableColumn::kValueCompareResultSame;
		}

		case SValue::kTypeSInt32: {
			// SInt32
			SInt32	sInt32_1 = value1.getSInt32();
			SInt32	sInt32_2 = value2.getSInt32();
			if (sInt32_1 < sInt32_2)
				// Ascending
				return CTableColumn::kValueCompareResultAscending;
			else if (sInt32_1 > sInt32_2)
				// Descending
				return CTableColumn::kValueCompareResultDescending;
			else
				// Same
				return CTableColumn::kValueCompareResultSame;
		}

		case SValue::kTypeSInt64: {
			// SInt64
			SInt64	sInt64_1 = value1.getSInt64();
			SInt64	sInt64_2 = value2.getSInt64();
			if (sInt64_1 < sInt64_2)
				// Ascending
				return CTableColumn::kValueCompareResultAscending;
			else if (sInt64_1 > sInt64_2)
				// Descending
				return CTableColumn::kValueCompareResultDescending;
			else
				// Same
				return CTableColumn::kValueCompareResultSame;
		}

		case SValue::kTypeUInt8: {
			// UInt8
			UInt8	uInt8_1 = value1.getUInt8();
			UInt8	uInt8_2 = value2.getUInt8();
			if (uInt8_1 < uInt8_2)
				// Ascending
				return CTableColumn::kValueCompareResultAscending;
			else if (uInt8_1 > uInt8_2)
				// Descending
				return CTableColumn::kValueCompareResultDescending;
			else
				// Same
				return CTableColumn::kValueCompareResultSame;
		}

		case SValue::kTypeUInt16: {
			// UInt16
			UInt16	uInt16_1 = value1.getUInt16();
			UInt16	uInt16_2 = value2.getUInt16();
			if (uInt16_1 < uInt16_2)
				// Ascending
				return CTableColumn::kValueCompareResultAscending;
			else if (uInt16_1 > uInt16_2)
				// Descending
				return CTableColumn::kValueCompareResultDescending;
			else
				// Same
				return CTableColumn::kValueCompareResultSame;
		}

		case SValue::kTypeUInt32: {
			// UInt32
			UInt32	uInt32_1 = value1.getUInt32();
			UInt32	uInt32_2 = value2.getUInt32();
			if (uInt32_1 < uInt32_2)
				// Ascending
				return CTableColumn::kValueCompareResultAscending;
			else if (uInt32_1 > uInt32_2)
				// Descending
				return CTableColumn::kValueCompareResultDescending;
			else
				// Same
				return CTableColumn::kValueCompareResultSame;
		}

		case SValue::kTypeUInt64: {
			// UInt64
			UInt64	uInt64_1 = value1.getUInt64();
			UInt64	uInt64_2 = value2.getUInt64();
			if (uInt64_1 < uInt64_2)
				// Ascending
				return CTableColumn::kValueCompareResultAscending;
			else if (uInt64_1 > uInt64_2)
				// Descending
				return CTableColumn::kValueCompareResultDescending;
			else
				// Same
				return CTableColumn::kValueCompareResultSame;
		}

		default:
			// The rest
			return CTableColumn::kValueCompareResultSame;
	}
}

// MARK: Subclass methods

//----------------------------------------------------------------------------------------------------------------------
void CTableColumn::noteContentChanged() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Call proc
	mInternals->mProcs->noteContentChanged(*this);
}

//----------------------------------------------------------------------------------------------------------------------
void CTableColumn::noteTitleChanged() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Call proc
	mInternals->mProcs->noteTitleChanged(*this);
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
bool CTableColumn::compareTitle(const I<CTableColumn>& tableColumn1, const I<CTableColumn>& tableColumn2,
		void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	return tableColumn1->getTitle().compareTo(tableColumn2->getTitle(), CString::kCompareToOptionsSortDefault);
}
