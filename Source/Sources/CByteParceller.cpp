//----------------------------------------------------------------------------------------------------------------------
//	CByteParceller.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CByteParceller.h"

#include "CData.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CByteParcellerInternals

class CByteParcellerInternals : public TReferenceCountable<CByteParcellerInternals> {
	public:
		CByteParcellerInternals(const I<CDataSource>& dataSource) :
			TReferenceCountable(), mNeedToSetPos(false), mDataSource(dataSource), mDataSourceOffset(0),
					mSize(dataSource->getSize())
			{}
		CByteParcellerInternals(const I<CDataSource>& dataSource, UInt64 dataSourceOffset, UInt64 size) :
			TReferenceCountable(), mNeedToSetPos(true), mDataSource(dataSource), mDataSourceOffset(dataSourceOffset),
					mSize(size)
			{}

		bool			mNeedToSetPos;
		I<CDataSource>	mDataSource;
		UInt64			mDataSourceOffset;
		UInt64			mSize;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CByteParceller

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CByteParceller::CByteParceller(const I<CDataSource>& dataSource)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CByteParcellerInternals(dataSource);
}

//----------------------------------------------------------------------------------------------------------------------
CByteParceller::CByteParceller(const CByteParceller& other, UInt64 offset, UInt64 size)
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	AssertFailIf((offset + size) > other.mInternals->mDataSource->getSize());

	// Setup
	mInternals = new CByteParcellerInternals(I<CDataSource>(other.mInternals->mDataSource->clone()), offset, size);
}

//----------------------------------------------------------------------------------------------------------------------
CByteParceller::CByteParceller(const CByteParceller& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CByteParceller::~CByteParceller()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
UInt64 CByteParceller::getSize() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mSize;
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CByteParceller::readData(void* buffer, UInt64 byteCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if can read
	if ((mInternals->mSize - getPos()) < byteCount)
		// Can't read that many bytes
		return OI<SError>(SError::mEndOfData);

	// Check if need to finish setup
	if (mInternals->mNeedToSetPos) {
		// Yes
		OI<SError>	error = setPos(kDataSourcePositionFromBeginning, 0);
		ReturnErrorIfError(error);
	}

	return mInternals->mDataSource->readData(buffer, byteCount);
}

//----------------------------------------------------------------------------------------------------------------------
CData CByteParceller::readData(UInt64 byteCount, OI<SError>& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CData	data((CData::Size) byteCount);

	// Read
	outError = readData(data.getMutableBytePtr(), byteCount);
	ReturnValueIfError(outError, CData::mEmpty);

	return data;
}

//----------------------------------------------------------------------------------------------------------------------
CData CByteParceller::readData(OI<SError>& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt64	byteCount = getSize() - getPos();
	CData	data((CData::Size) byteCount);

	// Read
	outError = readData(data.getMutableBytePtr(), byteCount);
	ReturnValueIfError(outError, CData::mEmpty);

	return data;
}

//----------------------------------------------------------------------------------------------------------------------
SInt64 CByteParceller::getPos() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mNeedToSetPos ?
			mInternals->mDataSourceOffset : mInternals->mDataSource->getPos() - mInternals->mDataSourceOffset;
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CByteParceller::setPos(EDataSourcePosition position, SInt64 newPos) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SInt64	newPosUse;
	if (mInternals->mNeedToSetPos) {
		// Yes
		switch (position) {
			case kDataSourcePositionFromBeginning:
			case kDataSourcePositionFromCurrent:
				// From beginning or "current" (which is "beginning" since hasn't actually been set yet)
				newPosUse = mInternals->mDataSourceOffset + newPos;
				break;

			case kDataSourcePositionFromEnd:
				// From end
				newPosUse = mInternals->mDataSourceOffset + mInternals->mSize - newPos;
				break;
		}
	} else {
		// Setup complete
		switch (position) {
			case kDataSourcePositionFromBeginning:
				// From beginning
				newPosUse = mInternals->mDataSourceOffset + newPos;
				break;

			case kDataSourcePositionFromCurrent:
				// From current
				newPosUse = getPos() + newPos;
				break;

			case kDataSourcePositionFromEnd:
				newPosUse = mInternals->mDataSourceOffset + mInternals->mSize - newPos;
				break;
		}
	}

	// Check
	AssertFailIf(newPosUse < 0);
	AssertFailIf((UInt64) newPosUse > (mInternals->mDataSourceOffset + mInternals->mSize));

	// Do it
	OI<SError>	error = mInternals->mDataSource->setPos(kDataSourcePositionFromBeginning, newPosUse);
	ReturnErrorIfError(error);

	// Setup complete
	mInternals->mNeedToSetPos = false;

	return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
void CByteParceller::reset() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Reset
	mInternals->mDataSource->reset();
	mInternals->mNeedToSetPos = true;
}
