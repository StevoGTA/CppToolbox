//----------------------------------------------------------------------------------------------------------------------
//	CByteReader.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CByteReader.h"

#include "CData.h"
#include "CReferenceCountable.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CByteReader::Internals

class CByteReader::Internals : public TReferenceCountableAutoDelete<Internals> {
	public:
		Internals(const I<CRandomAccessDataSource>& randomAccessDataSource, UInt64 dataSourceOffset, UInt64 byteCount,
				bool isBigEndian) :
			TReferenceCountableAutoDelete(),
					mIsBigEndian(isBigEndian), mRandomAccessDataSource(randomAccessDataSource),
					mInitialDataSourceOffset(dataSourceOffset), mCurrentDataSourceOffset(dataSourceOffset),
					mByteCount(byteCount)
			{}

		bool						mIsBigEndian;
		I<CRandomAccessDataSource>	mRandomAccessDataSource;
		UInt64						mInitialDataSourceOffset;
		UInt64						mCurrentDataSourceOffset;
		UInt64						mByteCount;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CByteReader

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CByteReader::CByteReader(const I<CRandomAccessDataSource>& randomAccessDataSource, bool isBigEndian)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(randomAccessDataSource, 0, randomAccessDataSource->getByteCount(), isBigEndian);
}

//----------------------------------------------------------------------------------------------------------------------
CByteReader::CByteReader(const I<CRandomAccessDataSource>& randomAccessDataSource, UInt64 offset, UInt64 size,
		bool isBigEndian)
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	AssertFailIf((offset + size) > randomAccessDataSource->getByteCount());

	// Setup
	mInternals = new Internals(randomAccessDataSource, offset, size, isBigEndian);
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
const I<CRandomAccessDataSource>& CByteReader::getRandomAccessDataSource() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mRandomAccessDataSource;
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CByteReader::getByteCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mByteCount;
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CByteReader::getPos() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mCurrentDataSourceOffset - mInternals->mInitialDataSourceOffset;
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CByteReader::setPos(Position position, SInt64 newPos) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Compose position
	UInt64	dataSourceOffset;
	switch (position) {
		case kPositionFromBeginning:
			// From beginning
			dataSourceOffset = mInternals->mInitialDataSourceOffset + newPos;
			break;

		case kPositionFromCurrent:
			// From current
			dataSourceOffset = mInternals->mCurrentDataSourceOffset + newPos;
			break;

		case kPositionFromEnd:
			// From end
			dataSourceOffset = mInternals->mInitialDataSourceOffset + mInternals->mByteCount - newPos;
			break;

#if defined(TARGET_OS_WINDOWS)
		// Making the Windows compiler happy
		default:	dataSourceOffset = 0;	break;
#endif
	}

	// Check
	AssertFailIf(dataSourceOffset < mInternals->mInitialDataSourceOffset);
	if (dataSourceOffset < mInternals->mInitialDataSourceOffset)
		// Before start
		return OV<SError>(CRandomAccessDataSource::mSetPosBeforeStartError);

	AssertFailIf(dataSourceOffset >
			(mInternals->mInitialDataSourceOffset + mInternals->mByteCount));
	if (dataSourceOffset > (mInternals->mInitialDataSourceOffset + mInternals->mByteCount))
		// After end
		return OV<SError>(CRandomAccessDataSource::mSetPosAfterEndError);

	// All good
	mInternals->mCurrentDataSourceOffset = dataSourceOffset;

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CByteReader::readData(void* buffer, UInt64 byteCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if can perform read
	if ((mInternals->mCurrentDataSourceOffset - mInternals->mInitialDataSourceOffset + byteCount) >
			mInternals->mByteCount)
		// Can't read that many bytes
		return OV<SError>(SError::mEndOfData);

	// Read
	OV<SError>	error =
						mInternals->mRandomAccessDataSource->readData(mInternals->mCurrentDataSourceOffset, buffer,
								byteCount);
	ReturnErrorIfError(error);

	// Update
	mInternals->mCurrentDataSourceOffset += byteCount;

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CData> CByteReader::readData(CData::ByteCount byteCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	CData		data(byteCount);
	OV<SError>	error = readData(data.getMutableBytePtr(), byteCount);
	ReturnValueIfError(error, TVResult<CData>(*error));

	return TVResult<CData>(data);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<SInt8> CByteReader::readSInt8() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	SInt8		value;
	OV<SError>	error = readData(&value, sizeof(SInt8));
	ReturnValueIfError(error, TVResult<SInt8>(*error));

	return TVResult<SInt8>(value);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<SInt16> CByteReader::readSInt16() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	SInt16		value;
	OV<SError>	error = readData(&value, sizeof(SInt16));
	ReturnValueIfError(error, TVResult<SInt16>(*error));

	return TVResult<SInt16>(mInternals->mIsBigEndian ? EndianS16_BtoN(value) : EndianS16_LtoN(value));
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<SInt32> CByteReader::readSInt32() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	SInt32		value;
	OV<SError>	error = readData(&value, sizeof(SInt32));
	ReturnValueIfError(error, TVResult<SInt32>(*error));

	return TVResult<SInt32>(mInternals->mIsBigEndian ? EndianS32_BtoN(value) : EndianS32_LtoN(value));
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<SInt64> CByteReader::readSInt64() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	SInt64		value;
	OV<SError>	error = readData(&value, sizeof(SInt64));
	ReturnValueIfError(error, TVResult<SInt64>(*error));

	return TVResult<SInt64>(mInternals->mIsBigEndian ? EndianS64_BtoN(value) : EndianS64_LtoN(value));
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<UInt8> CByteReader::readUInt8() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	UInt8		value;
	OV<SError>	error = readData(&value, sizeof(UInt8));
	ReturnValueIfError(error, TVResult<UInt8>(*error));

	return TVResult<UInt8>(value);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<UInt16> CByteReader::readUInt16() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	UInt16		value;
	OV<SError>	error = readData(&value, sizeof(UInt16));
	ReturnValueIfError(error, TVResult<UInt16>(*error));

	return TVResult<UInt16>(mInternals->mIsBigEndian ? EndianU16_BtoN(value) : EndianU16_LtoN(value));
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<UInt32> CByteReader::readUInt32() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	UInt32		value;
	OV<SError>	error = readData(&value, sizeof(UInt32));
	ReturnValueIfError(error, TVResult<UInt32>(*error));

	return TVResult<UInt32>(mInternals->mIsBigEndian ? EndianU32_BtoN(value) : EndianU32_LtoN(value));
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<UInt64> CByteReader::readUInt64() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	UInt64		value;
	OV<SError>	error = readData(&value, sizeof(UInt64));
	ReturnValueIfError(error, TVResult<UInt64>(*error));

	return TVResult<UInt64>(mInternals->mIsBigEndian ? EndianU64_BtoN(value) : EndianU64_LtoN(value));
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<OSType> CByteReader::readOSType() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	OSType		value;
	OV<SError>	error = readData(&value, sizeof(OSType));
	ReturnValueIfError(error, TVResult<OSType>(*error));

	return TVResult<OSType>(EndianU32_BtoN(value));
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CUUID> CByteReader::readUUID() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	CUUID::Bytes	uuidBytes;
	OV<SError>		error = readData(&uuidBytes, sizeof(CUUID::Bytes));
	ReturnValueIfError(error, TVResult<CUUID>(*error));

	return TVResult<CUUID>(CUUID(uuidBytes));
}
