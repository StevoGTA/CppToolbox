//----------------------------------------------------------------------------------------------------------------------
//	CByteReader.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CByteReader.h"

#include "CData.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CByteReaderInternals

class CByteReaderInternals : public TReferenceCountable<CByteReaderInternals> {
	public:
		CByteReaderInternals(const I<CSeekableDataSource>& seekableDataSource, UInt64 dataSourceOffset, UInt64 size,
				bool isBigEndian) :
			TReferenceCountable(), mIsBigEndian(isBigEndian),
					mSeekableDataSource(seekableDataSource), mInitialDataSourceOffset(dataSourceOffset),
					mCurrentDataSourceOffset(dataSourceOffset), mSize(size)
			{}

		bool					mIsBigEndian;
		I<CSeekableDataSource>	mSeekableDataSource;
		UInt64					mInitialDataSourceOffset;
		UInt64					mCurrentDataSourceOffset;
		UInt64					mSize;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CByteReader

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CByteReader::CByteReader(const I<CSeekableDataSource>& seekableDataSource, bool isBigEndian)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CByteReaderInternals(seekableDataSource, 0, seekableDataSource->getSize(), isBigEndian);
}

//----------------------------------------------------------------------------------------------------------------------
CByteReader::CByteReader(const I<CSeekableDataSource>& seekableDataSource, UInt64 offset, UInt64 size, bool isBigEndian)
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	AssertFailIf((offset + size) > seekableDataSource->getSize());

	// Setup
	mInternals = new CByteReaderInternals(seekableDataSource, offset, size, isBigEndian);
}

//----------------------------------------------------------------------------------------------------------------------
CByteReader::CByteReader(const CByteReader& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CByteReader::~CByteReader()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
UInt64 CByteReader::getSize() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mSize;
}

//----------------------------------------------------------------------------------------------------------------------
SInt64 CByteReader::getPos() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mCurrentDataSourceOffset - mInternals->mInitialDataSourceOffset;
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CByteReader::setPos(Position position, SInt64 newPos) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check position
	switch (position) {
		case kPositionFromBeginning:
			// From beginning
			mInternals->mCurrentDataSourceOffset = mInternals->mInitialDataSourceOffset + newPos;
			break;

		case kPositionFromCurrent:
			// From current
			mInternals->mCurrentDataSourceOffset += newPos;
			break;

		case kPositionFromEnd:
			// From end
			mInternals->mCurrentDataSourceOffset = mInternals->mInitialDataSourceOffset + mInternals->mSize - newPos;
			break;
	}

	// Check
	AssertFailIf(mInternals->mCurrentDataSourceOffset < mInternals->mInitialDataSourceOffset);
	AssertFailIf(mInternals->mCurrentDataSourceOffset > (mInternals->mInitialDataSourceOffset + mInternals->mSize));

	return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CByteReader::readData(void* buffer, UInt64 byteCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if can perform read
	if ((mInternals->mCurrentDataSourceOffset - mInternals->mInitialDataSourceOffset + byteCount) > mInternals->mSize)
		// Can't read that many bytes
		return OI<SError>(SError::mEndOfData);

	// Read
	OI<SError>	error =
						mInternals->mSeekableDataSource->readData(mInternals->mCurrentDataSourceOffset, buffer,
								byteCount);
	ReturnErrorIfError(error);

	// Update
	mInternals->mCurrentDataSourceOffset += byteCount;

	return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
TIResult<CData> CByteReader::readData(CData::Size byteCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	CData		data(byteCount);
	OI<SError>	error = readData(data.getMutableBytePtr(), byteCount);
	ReturnValueIfError(error, TIResult<CData>(*error));

	return TIResult<CData>(data);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<SInt8> CByteReader::readSInt8() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	SInt8		value;
	OI<SError>	error = readData(&value, sizeof(SInt8));
	ReturnValueIfError(error, TVResult<SInt8>(*error));

	return TVResult<SInt8>(value);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<SInt16> CByteReader::readSInt16() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	SInt16		value;
	OI<SError>	error = readData(&value, sizeof(SInt16));
	ReturnValueIfError(error, TVResult<SInt16>(*error));

	return TVResult<SInt16>(mInternals->mIsBigEndian ? EndianS16_BtoN(value) : EndianS16_LtoN(value));
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<SInt32> CByteReader::readSInt32() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	SInt32		value;
	OI<SError>	error = readData(&value, sizeof(SInt32));
	ReturnValueIfError(error, TVResult<SInt32>(*error));

	return TVResult<SInt32>(mInternals->mIsBigEndian ? EndianS32_BtoN(value) : EndianS32_LtoN(value));
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<SInt64> CByteReader::readSInt64() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	SInt64		value;
	OI<SError>	error = readData(&value, sizeof(SInt64));
	ReturnValueIfError(error, TVResult<SInt64>(*error));

	return TVResult<SInt64>(mInternals->mIsBigEndian ? EndianS64_BtoN(value) : EndianS64_LtoN(value));
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<UInt8> CByteReader::readUInt8() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	UInt8		value;
	OI<SError>	error = readData(&value, sizeof(UInt8));
	ReturnValueIfError(error, TVResult<UInt8>(*error));

	return TVResult<UInt8>(value);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<UInt16> CByteReader::readUInt16() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	UInt16		value;
	OI<SError>	error = readData(&value, sizeof(UInt16));
	ReturnValueIfError(error, TVResult<UInt16>(*error));

	return TVResult<UInt16>(mInternals->mIsBigEndian ? EndianU16_BtoN(value) : EndianU16_LtoN(value));
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<UInt32> CByteReader::readUInt32() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	UInt32		value;
	OI<SError>	error = readData(&value, sizeof(UInt32));
	ReturnValueIfError(error, TVResult<UInt32>(*error));

	return TVResult<UInt32>(mInternals->mIsBigEndian ? EndianU32_BtoN(value) : EndianU32_LtoN(value));
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<UInt64> CByteReader::readUInt64() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	UInt64		value;
	OI<SError>	error = readData(&value, sizeof(UInt64));
	ReturnValueIfError(error, TVResult<UInt64>(*error));

	return TVResult<UInt64>(mInternals->mIsBigEndian ? EndianU64_BtoN(value) : EndianU64_LtoN(value));
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<OSType> CByteReader::readOSType() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	OSType		value;
	OI<SError>	error = readData(&value, sizeof(OSType));
	ReturnValueIfError(error, TVResult<OSType>(*error));

	return TVResult<OSType>(EndianU32_BtoN(value));
}
