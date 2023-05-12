//----------------------------------------------------------------------------------------------------------------------
//	CBitReader.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CBitReader.h"

#include "CData.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CBitReader::Internals

class CBitReader::Internals : public TReferenceCountable<Internals> {
	public:
		Internals(const I<CRandomAccessDataSource>& randomAccessDataSource, bool isBigEndian) :
			TReferenceCountable(),
					mIsBigEndian(isBigEndian), mRandomAccessDataSource(randomAccessDataSource), mDataSourceOffset(0),
					mByteCount(mRandomAccessDataSource->getByteCount()),
					mCurrentByte(0), mCurrentByteBitsAvailable(0)
			{}

		UInt64		getPos()
						{ return mDataSourceOffset; }
		OV<SError>	setPos(CBitReader::Position position, SInt64 newPos)
						{
							// Compose position
							SInt64	dataSourceOffset;
							switch (position) {
								case CBitReader::kPositionFromBeginning:
									// From beginning
									dataSourceOffset = newPos;
									break;

								case CBitReader::kPositionFromCurrent:
									// From current
									dataSourceOffset = mDataSourceOffset + newPos;
									break;

								case CBitReader::kPositionFromEnd:
									// From end
									dataSourceOffset = mByteCount - newPos;
									break;

#if defined(TARGET_OS_WINDOWS)
								// Making the Windows compiler happy
								default:	dataSourceOffset = 0;	break;
#endif
							}

							// Check
							AssertFailIf(dataSourceOffset < 0);
							if (dataSourceOffset < 0)
								// Before start
								return OV<SError>(CRandomAccessDataSource::mSetPosBeforeStartError);

							AssertFailIf((UInt64) dataSourceOffset > mByteCount);
							if ((UInt64) dataSourceOffset > mByteCount)
								// After end
								return OV<SError>(CRandomAccessDataSource::mSetPosAfterEndError);

							// All good
							mDataSourceOffset = (UInt64) dataSourceOffset;

							return OV<SError>();
						}

		OV<SError>	readData(void* buffer, UInt64 byteCount)
						{
							// Reset bit reading
							mCurrentByte = 0;
							mCurrentByteBitsAvailable = 0;

							// Check if can perform read
							if ((mDataSourceOffset + byteCount) > mByteCount)
								// Can't read that many bytes
								return OV<SError>(SError::mEndOfData);

							// Read
							OV<SError>	error = mRandomAccessDataSource->readData(mDataSourceOffset, buffer, byteCount);
							ReturnErrorIfError(error);

							// Update
							mDataSourceOffset += byteCount;

							return OV<SError>();
						}

		OV<SError>	reloadCurrentByte()
						{
							// Check if need to reload current byte
							if (mCurrentByteBitsAvailable == 0) {
								// Read next byte
								OV<SError>	error = readData(&mCurrentByte, 1);
								ReturnErrorIfError(error);

								mCurrentByteBitsAvailable = 8;
							}

							return OV<SError>();
						}
		UInt8		readBits(UInt8 bitCount)
						{
							// Compose value
							UInt8	value =
											((mCurrentByte << (8 - mCurrentByteBitsAvailable)) & 0xFF) >>
													(8 - bitCount);

							// Update
							mCurrentByteBitsAvailable -= bitCount;

							return value;
						}

		bool						mIsBigEndian;
		I<CRandomAccessDataSource>	mRandomAccessDataSource;
		UInt64						mDataSourceOffset;
		UInt64						mByteCount;

		UInt8						mCurrentByte;
		UInt8						mCurrentByteBitsAvailable;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CBitReader

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CBitReader::CBitReader(const I<CRandomAccessDataSource>& randomAccessDataSource, bool isBigEndian)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(randomAccessDataSource, isBigEndian);
}

//----------------------------------------------------------------------------------------------------------------------
CBitReader::CBitReader(const CBitReader& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CBitReader::~CBitReader()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
UInt64 CBitReader::getByteCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mByteCount;
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CBitReader::getPos() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->getPos();
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CBitReader::setPos(Position position, SInt64 newPos) const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->setPos(position, newPos);
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CBitReader::readData(void* buffer, UInt64 byteCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->readData(buffer, byteCount);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CData> CBitReader::readData(CData::ByteCount byteCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	CData		data(byteCount);
	OV<SError>	error = mInternals->readData(data.getMutableBytePtr(), byteCount);
	ReturnValueIfError(error, TVResult<CData>(*error));

	return TVResult<CData>(data);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<SInt8> CBitReader::readSInt8() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	SInt8		value;
	OV<SError>	error = mInternals->readData(&value, sizeof(SInt8));
	ReturnValueIfError(error, TVResult<SInt8>(*error));

	return TVResult<SInt8>(value);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<SInt16> CBitReader::readSInt16() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	SInt16		value;
	OV<SError>	error = mInternals->readData(&value, sizeof(SInt16));
	ReturnValueIfError(error, TVResult<SInt16>(*error));

	return TVResult<SInt16>(mInternals->mIsBigEndian ? EndianS16_BtoN(value) : EndianS16_LtoN(value));
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<SInt32> CBitReader::readSInt32() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	SInt32		value;
	OV<SError>	error = mInternals->readData(&value, sizeof(SInt32));
	ReturnValueIfError(error, TVResult<SInt32>(*error));

	return TVResult<SInt32>(mInternals->mIsBigEndian ? EndianS32_BtoN(value) : EndianS32_LtoN(value));
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<SInt64> CBitReader::readSInt64() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	SInt64		value;
	OV<SError>	error = mInternals->readData(&value, sizeof(SInt64));
	ReturnValueIfError(error, TVResult<SInt64>(*error));

	return TVResult<SInt64>(mInternals->mIsBigEndian ? EndianS64_BtoN(value) : EndianS64_LtoN(value));
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<UInt8> CBitReader::readUInt8() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	UInt8		value;
	OV<SError>	error = mInternals->readData(&value, sizeof(UInt8));
	ReturnValueIfError(error, TVResult<UInt8>(*error));

	return TVResult<UInt8>(value);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<UInt8> CBitReader::readUInt8(UInt8 bitCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	AssertFailIf(bitCount > 8);

	// Compose value
	UInt8	value = 0;
	while (bitCount > 0) {
		// Reload current byte
		OV<SError>	error = mInternals->reloadCurrentByte();
		ReturnValueIfError(error, TVResult<UInt8>(*error));

		// Process
		UInt8	bitsToProcess =
						(bitCount < mInternals->mCurrentByteBitsAvailable) ?
								bitCount : mInternals->mCurrentByteBitsAvailable;
		bitCount -= bitsToProcess;
		value = (value << bitsToProcess) | mInternals->readBits(bitsToProcess);
	}

	return TVResult<UInt8>(value);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<UInt16> CBitReader::readUInt16() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	UInt16		value;
	OV<SError>	error = mInternals->readData(&value, sizeof(UInt16));
	ReturnValueIfError(error, TVResult<UInt16>(*error));

	return TVResult<UInt16>(mInternals->mIsBigEndian ? EndianU16_BtoN(value) : EndianU16_LtoN(value));
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<UInt32> CBitReader::readUInt32() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	UInt32		value;
	OV<SError>	error = mInternals->readData(&value, sizeof(UInt32));
	ReturnValueIfError(error, TVResult<UInt32>(*error));

	return TVResult<UInt32>(mInternals->mIsBigEndian ? EndianU32_BtoN(value) : EndianU32_LtoN(value));
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<UInt32> CBitReader::readUInt32(UInt8 bitCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	AssertFailIf(bitCount > 32);

	// Compose value
	UInt32	value = 0;
	while (bitCount > 0) {
		// Reload current byte
		OV<SError>	error = mInternals->reloadCurrentByte();
		ReturnValueIfError(error, TVResult<UInt32>(*error));

		// Process
		UInt8	bitsToProcess =
						(bitCount < mInternals->mCurrentByteBitsAvailable) ?
								bitCount : mInternals->mCurrentByteBitsAvailable;
		bitCount -= bitsToProcess;
		value = (value << bitsToProcess) | mInternals->readBits(bitsToProcess);
	}

	return TVResult<UInt32>(value);
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<UInt64> CBitReader::readUInt64() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	UInt64		value;
	OV<SError>	error = mInternals->readData(&value, sizeof(UInt64));
	ReturnValueIfError(error, TVResult<UInt64>(*error));

	return TVResult<UInt64>(mInternals->mIsBigEndian ? EndianU64_BtoN(value) : EndianU64_LtoN(value));
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<UInt32> CBitReader::readUEColumbusCode() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Compose value
	SInt8	leadingZeroBits = -1;
	for (UInt8 b = 0; !b; leadingZeroBits++) {
		// Reload current byte
		OV<SError>	error = mInternals->reloadCurrentByte();
		ReturnValueIfError(error, TVResult<UInt32>(*error));

		// Read bit
		b = mInternals->readBits(1);
	}

	TVResult<UInt32>	value = readUInt32(leadingZeroBits);
	ReturnValueIfResultError(value, TVResult<UInt32>(value.getError()));

	return TVResult<UInt32>((1 << leadingZeroBits) - 1 + *value);
}
