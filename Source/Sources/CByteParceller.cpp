//----------------------------------------------------------------------------------------------------------------------
//	CByteParceller.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CByteParceller.h"

#include "CData.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CByteParcellerInternals

class CByteParcellerInternals : public TReferenceCountable<CByteParcellerInternals> {
	public:
		CByteParcellerInternals(const I<CDataSource>& dataSource, UInt64 dataSourceOffset, UInt64 size,
				bool isBigEndian) :
			TReferenceCountable(), mIsBigEndian(isBigEndian), mNeedToSetPos(true), mDataSource(dataSource),
					mDataSourceOffset(dataSourceOffset), mSize(size)
			{}

		bool			mIsBigEndian;
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
CByteParceller::CByteParceller(const I<CDataSource>& dataSource, bool isBigEndian)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CByteParcellerInternals(dataSource, 0, dataSource->getSize(), isBigEndian);
}

//----------------------------------------------------------------------------------------------------------------------
CByteParceller::CByteParceller(const I<CDataSource>& dataSource, UInt64 offset, UInt64 size, bool isBigEndian)
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	AssertFailIf((offset + size) > dataSource->getSize());

	// Setup
	mInternals = new CByteParcellerInternals(dataSource, offset, size, isBigEndian);
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
SInt64 CByteParceller::getPos() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mNeedToSetPos ?
			mInternals->mDataSourceOffset : mInternals->mDataSource->getPos() - mInternals->mDataSourceOffset;
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CByteParceller::setPos(CDataSource::Position position, SInt64 newPos) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SInt64	newPosUse;
	if (mInternals->mNeedToSetPos) {
		// Yes
		switch (position) {
			case CDataSource::kPositionFromBeginning:
			case CDataSource::kPositionFromCurrent:
				// From beginning or "current" (which is "beginning" since hasn't actually been set yet)
				newPosUse = mInternals->mDataSourceOffset + newPos;
				break;

			case CDataSource::kPositionFromEnd:
				// From end
				newPosUse = mInternals->mDataSourceOffset + mInternals->mSize - newPos;
				break;
		}
	} else {
		// Setup complete
		switch (position) {
			case CDataSource::kPositionFromBeginning:
				// From beginning
				newPosUse = mInternals->mDataSourceOffset + newPos;
				break;

			case CDataSource::kPositionFromCurrent:
				// From current
				newPosUse = getPos() + newPos;
				break;

			case CDataSource::kPositionFromEnd:
				newPosUse = mInternals->mDataSourceOffset + mInternals->mSize - newPos;
				break;
		}
	}

	// Check
	AssertFailIf(newPosUse < 0);
	AssertFailIf((UInt64) newPosUse > (mInternals->mDataSourceOffset + mInternals->mSize));

	// Do it
	OI<SError>	error = mInternals->mDataSource->setPos(CDataSource::kPositionFromBeginning, newPosUse);
	ReturnErrorIfError(error);

	// Setup complete
	mInternals->mNeedToSetPos = false;

	return OI<SError>();
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
		OI<SError>	error = setPos(CDataSource::kPositionFromBeginning, 0);
		ReturnErrorIfError(error);
	}

	return mInternals->mDataSource->readData(buffer, byteCount);
}

//----------------------------------------------------------------------------------------------------------------------
OI<CData> CByteParceller::readData(UInt64 byteCount, OI<SError>& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	OI<CData>	data(new CData((CData::Size) byteCount));

	// Read
	outError = readData(data->getMutableBytePtr(), byteCount);
	ReturnValueIfError(outError, OI<CData>());

	return data;
}

//----------------------------------------------------------------------------------------------------------------------
OV<SInt8> CByteParceller::readSInt8(OI<SError>& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	SInt8	value = 0;
	outError = readData(&value, sizeof(SInt8));
	ReturnValueIfError(outError, OV<SInt8>());

	return OV<SInt8>(value);
}

//----------------------------------------------------------------------------------------------------------------------
OV<SInt16> CByteParceller::readSInt16(OI<SError>& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	SInt16	value = 0;
	outError = readData(&value, sizeof(SInt16));
	ReturnValueIfError(outError, OV<SInt16>());

	return OV<SInt16>(mInternals->mIsBigEndian ? EndianS16_BtoN(value) : EndianS16_LtoN(value));
}

//----------------------------------------------------------------------------------------------------------------------
OV<SInt32> CByteParceller::readSInt32(OI<SError>& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	SInt32	value = 0;
	outError = readData(&value, sizeof(SInt32));
	ReturnValueIfError(outError, OV<SInt32>());

	return OV<SInt32>(mInternals->mIsBigEndian ? EndianS32_BtoN(value) : EndianS32_LtoN(value));
}

//----------------------------------------------------------------------------------------------------------------------
OV<SInt64> CByteParceller::readSInt64(OI<SError>& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	SInt64	value = 0;
	outError = readData(&value, sizeof(SInt64));
	ReturnValueIfError(outError, OV<SInt64>());

	return OV<SInt64>(mInternals->mIsBigEndian ? EndianS64_BtoN(value) : EndianS64_LtoN(value));
}

//----------------------------------------------------------------------------------------------------------------------
OV<UInt8> CByteParceller::readUInt8(OI<SError>& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	UInt8	value = 0;
	outError = readData(&value, sizeof(UInt8));
	ReturnValueIfError(outError, OV<UInt8>());

	return OV<UInt8>(value);
}

//----------------------------------------------------------------------------------------------------------------------
OV<UInt16> CByteParceller::readUInt16(OI<SError>& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	UInt16	value = 0;
	outError = readData(&value, sizeof(UInt16));
	ReturnValueIfError(outError, OV<UInt16>());

	return OV<UInt16>(mInternals->mIsBigEndian ? EndianU16_BtoN(value) : EndianU16_LtoN(value));
}

//----------------------------------------------------------------------------------------------------------------------
OV<UInt32> CByteParceller::readUInt32(OI<SError>& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	UInt32	value = 0;
	outError = readData(&value, sizeof(UInt32));
	ReturnValueIfError(outError, OV<UInt32>());

	return OV<UInt32>(mInternals->mIsBigEndian ? EndianU32_BtoN(value) : EndianU32_LtoN(value));
}

//----------------------------------------------------------------------------------------------------------------------
OV<UInt64> CByteParceller::readUInt64(OI<SError>& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	UInt64	value = 0;
	outError = readData(&value, sizeof(UInt64));
	ReturnValueIfError(outError, OV<UInt64>());

	return OV<UInt64>(mInternals->mIsBigEndian ? EndianU64_BtoN(value) : EndianU64_LtoN(value));
}

//----------------------------------------------------------------------------------------------------------------------
OV<OSType> CByteParceller::readOSType(OI<SError>& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	OSType	value = 0;
	outError = readData(&value, sizeof(OSType));
	ReturnValueIfError(outError, OV<OSType>());

	return OV<OSType>(EndianU32_BtoN(value));
}

//----------------------------------------------------------------------------------------------------------------------
void CByteParceller::reset() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Reset
	mInternals->mNeedToSetPos = true;
	mInternals->mDataSource->reset();
}
