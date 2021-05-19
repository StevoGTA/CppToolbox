//----------------------------------------------------------------------------------------------------------------------
//	CBitParceller.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CBitParceller.h"

#include "CData.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CBitParcellerInternals

class CBitParcellerInternals : public TReferenceCountable<CBitParcellerInternals> {
	public:
		CBitParcellerInternals(const I<CDataSource>& dataSource, UInt64 dataSourceOffset, UInt64 size,
				bool isBigEndian) :
			TReferenceCountable(), mIsBigEndian(isBigEndian), mNeedToSetPos(true), mDataSource(dataSource),
					mDataSourceOffset(dataSourceOffset), mSize(size), mCurrentByte(0), mCurrentByteBitsAvailable(0)
			{}

		SInt64		getPos()
						{ return mNeedToSetPos ? mDataSourceOffset : mDataSource->getPos() - mDataSourceOffset; }
		OI<SError>	setPos(CDataSource::Position position, SInt64 newPos)
						{
							// Setup
							SInt64	newPosUse;
							if (mNeedToSetPos) {
								// Yes
								switch (position) {
									case CDataSource::kPositionFromBeginning:
									case CDataSource::kPositionFromCurrent:
										// From beginning or "current" (which is "beginning" since hasn't actually been
										//	set yet)
										newPosUse = mDataSourceOffset + newPos;
										break;

									case CDataSource::kPositionFromEnd:
										// From end
										newPosUse = mDataSourceOffset + mSize - newPos;
										break;
								}
							} else {
								// Setup complete
								switch (position) {
									case CDataSource::kPositionFromBeginning:
										// From beginning
										newPosUse = mDataSourceOffset + newPos;
										break;

									case CDataSource::kPositionFromCurrent:
										// Check request
										if (newPos == 0) {
											// Advance to next byte boundary
											mCurrentByte = 0;
											mCurrentByteBitsAvailable = 0;

											return OI<SError>();
										} else
											// From current
											newPosUse = getPos() + newPos;
										break;

									case CDataSource::kPositionFromEnd:
										newPosUse = mDataSourceOffset + mSize - newPos;
										break;
								}
							}

							// Check
							AssertFailIf(newPosUse < 0);
							AssertFailIf((UInt64) newPosUse > (mDataSourceOffset + mSize));

							// Do it
							OI<SError>	error = mDataSource->setPos(CDataSource::kPositionFromBeginning, newPosUse);
							ReturnErrorIfError(error);

							// Setup complete
							mNeedToSetPos = false;

							return OI<SError>();
						}

		OI<SError>	readData(void* buffer, UInt64 byteCount)
						{
							// Reset bit reading
							mCurrentByte = 0;
							mCurrentByteBitsAvailable = 0;

							// Check if can read
							if ((mSize - getPos()) < byteCount)
								// Can't read that many bytes
								return OI<SError>(SError::mEndOfData);

							// Check if need to finish setup
							if (mNeedToSetPos) {
								// Yes
								OI<SError>	error = setPos(CDataSource::kPositionFromBeginning, 0);
								ReturnErrorIfError(error);
							}

							return mDataSource->readData(buffer, byteCount);
						}

		void		reloadCurrentByte(OI<SError>& outError)
						{
							// Check if need to reload current byte
							if (mCurrentByteBitsAvailable == 0) {
								// Read next byte
								outError = readData(&mCurrentByte, 1);
								ReturnIfError(outError);

								mCurrentByteBitsAvailable = 8;
							}
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

		bool			mIsBigEndian;
		bool			mNeedToSetPos;
		I<CDataSource>	mDataSource;
		UInt64			mDataSourceOffset;
		UInt64			mSize;

		UInt8			mCurrentByte;
		UInt8			mCurrentByteBitsAvailable;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CBitParceller

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CBitParceller::CBitParceller(const I<CDataSource>& dataSource, bool isBigEndian)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CBitParcellerInternals(dataSource, 0, dataSource->getSize(), isBigEndian);
}

//----------------------------------------------------------------------------------------------------------------------
CBitParceller::CBitParceller(const I<CDataSource>& dataSource, UInt64 offset, UInt64 size, bool isBigEndian)
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	AssertFailIf((offset + size) > dataSource->getSize());

	// Setup
	mInternals = new CBitParcellerInternals(dataSource, offset, size, isBigEndian);
}

//----------------------------------------------------------------------------------------------------------------------
CBitParceller::CBitParceller(const CBitParceller& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CBitParceller::~CBitParceller()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
UInt64 CBitParceller::getSize() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mSize;
}

//----------------------------------------------------------------------------------------------------------------------
SInt64 CBitParceller::getPos() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->getPos();
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CBitParceller::setPos(CDataSource::Position position, SInt64 newPos) const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->setPos(position, newPos);
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CBitParceller::readData(void* buffer, UInt64 byteCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->readData(buffer, byteCount);
}

//----------------------------------------------------------------------------------------------------------------------
OI<CData> CBitParceller::readData(UInt64 byteCount, OI<SError>& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	OI<CData>	data(new CData((CData::Size) byteCount));

	// Read
	outError = mInternals->readData(data->getMutableBytePtr(), byteCount);
	ReturnValueIfError(outError, OI<CData>());

	return data;
}

//----------------------------------------------------------------------------------------------------------------------
OV<SInt8> CBitParceller::readSInt8(OI<SError>& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	SInt8	value = 0;
	outError = mInternals->readData(&value, sizeof(SInt8));
	ReturnValueIfError(outError, OV<SInt8>());

	return OV<SInt8>(value);
}

////----------------------------------------------------------------------------------------------------------------------
//OV<SInt8> CBitParceller::readSInt8(UInt8 bitCount, OI<SError>& outError) const
////----------------------------------------------------------------------------------------------------------------------
//{
//	// Preflight
//	AssertFailIf(bitCount > 8);
//}

//----------------------------------------------------------------------------------------------------------------------
OV<SInt16> CBitParceller::readSInt16(OI<SError>& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	SInt16	value = 0;
	outError = mInternals->readData(&value, sizeof(SInt16));
	ReturnValueIfError(outError, OV<SInt16>());

	return OV<SInt16>(mInternals->mIsBigEndian ? EndianS16_BtoN(value) : EndianS16_LtoN(value));
}

////----------------------------------------------------------------------------------------------------------------------
//OV<SInt16> CBitParceller::readSInt16(UInt8 bitCount, OI<SError>& outError) const
////----------------------------------------------------------------------------------------------------------------------
//{
//	// Preflight
//	AssertFailIf(bitCount > 16);
//
//	if (bitCount == 0) return OV<SInt16>(0);
//}

//----------------------------------------------------------------------------------------------------------------------
OV<SInt32> CBitParceller::readSInt32(OI<SError>& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	SInt32	value = 0;
	outError = mInternals->readData(&value, sizeof(SInt32));
	ReturnValueIfError(outError, OV<SInt32>());

	return OV<SInt32>(mInternals->mIsBigEndian ? EndianS32_BtoN(value) : EndianS32_LtoN(value));
}

////----------------------------------------------------------------------------------------------------------------------
//OV<SInt32> CBitParceller::readSInt32(UInt8 bitCount, OI<SError>& outError) const
////----------------------------------------------------------------------------------------------------------------------
//{
//	// Preflight
//	AssertFailIf(bitCount > 32);
//
//	if (bitCount == 0) return OV<SInt32>(0);
//}

//----------------------------------------------------------------------------------------------------------------------
OV<SInt64> CBitParceller::readSInt64(OI<SError>& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	SInt64	value = 0;
	outError = mInternals->readData(&value, sizeof(SInt64));
	ReturnValueIfError(outError, OV<SInt64>());

	return OV<SInt64>(mInternals->mIsBigEndian ? EndianS64_BtoN(value) : EndianS64_LtoN(value));
}

////----------------------------------------------------------------------------------------------------------------------
//OV<SInt64> CBitParceller::readSInt64(UInt8 bitCount, OI<SError>& outError) const
////----------------------------------------------------------------------------------------------------------------------
//{
//	// Preflight
//	AssertFailIf(bitCount > 64);
//
//	if (bitCount == 0) return OV<SInt64>(0);
//}

//----------------------------------------------------------------------------------------------------------------------
OV<UInt8> CBitParceller::readUInt8(OI<SError>& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	UInt8	value = 0;
	outError = mInternals->readData(&value, sizeof(UInt8));
	ReturnValueIfError(outError, OV<UInt8>());

	return OV<UInt8>(value);
}

//----------------------------------------------------------------------------------------------------------------------
OV<UInt8> CBitParceller::readUInt8(UInt8 bitCount, OI<SError>& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	AssertFailIf(bitCount > 8);

	// Compose value
	UInt8	value = 0;
	while (bitCount > 0) {
		// Reload current byte
		mInternals->reloadCurrentByte(outError);
		ReturnValueIfError(outError, OV<UInt8>());

		// Process
		UInt8	bitsToProcess =
						(bitCount < mInternals->mCurrentByteBitsAvailable) ?
								bitCount : mInternals->mCurrentByteBitsAvailable;
		bitCount -= bitsToProcess;
		value = (value << bitsToProcess) | mInternals->readBits(bitsToProcess);
	}

	return OV<UInt8>(value);
}

//----------------------------------------------------------------------------------------------------------------------
OV<UInt16> CBitParceller::readUInt16(OI<SError>& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	UInt16	value = 0;
	outError = mInternals->readData(&value, sizeof(UInt16));
	ReturnValueIfError(outError, OV<UInt16>());

	return OV<UInt16>(mInternals->mIsBigEndian ? EndianU16_BtoN(value) : EndianU16_LtoN(value));
}

////----------------------------------------------------------------------------------------------------------------------
//OV<UInt16> CBitParceller::readUInt16(UInt8 bitCount, OI<SError>& outError) const
////----------------------------------------------------------------------------------------------------------------------
//{
//	// Preflight
//	AssertFailIf(bitCount > 16);
//
//	if (bitCount == 0) return OV<UInt16>(0);
//}

//----------------------------------------------------------------------------------------------------------------------
OV<UInt32> CBitParceller::readUInt32(OI<SError>& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	UInt32	value = 0;
	outError = mInternals->readData(&value, sizeof(UInt32));
	ReturnValueIfError(outError, OV<UInt32>());

	return OV<UInt32>(mInternals->mIsBigEndian ? EndianU32_BtoN(value) : EndianU32_LtoN(value));
}

//----------------------------------------------------------------------------------------------------------------------
OV<UInt32> CBitParceller::readUInt32(UInt8 bitCount, OI<SError>& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	AssertFailIf(bitCount > 32);

	// Compose value
	UInt32	value = 0;
	while (bitCount > 0) {
		// Reload current byte
		mInternals->reloadCurrentByte(outError);
		ReturnValueIfError(outError, OV<UInt32>());

		// Process
		UInt8	bitsToProcess =
						(bitCount < mInternals->mCurrentByteBitsAvailable) ?
								bitCount : mInternals->mCurrentByteBitsAvailable;
		bitCount -= bitsToProcess;
		value = (value << bitsToProcess) | mInternals->readBits(bitsToProcess);
	}

	return OV<UInt32>(value);
}

//----------------------------------------------------------------------------------------------------------------------
OV<UInt64> CBitParceller::readUInt64(OI<SError>& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	UInt64	value = 0;
	outError = mInternals->readData(&value, sizeof(UInt64));
	ReturnValueIfError(outError, OV<UInt64>());

	return OV<UInt64>(mInternals->mIsBigEndian ? EndianU64_BtoN(value) : EndianU64_LtoN(value));
}

////----------------------------------------------------------------------------------------------------------------------
//OV<UInt64> CBitParceller::readUInt64(UInt8 bitCount, OI<SError>& outError) const
////----------------------------------------------------------------------------------------------------------------------
//{
//	// Preflight
//	AssertFailIf(bitCount > 64);
//
//	if (bitCount == 0) return OV<UInt64>(0);
//}

//----------------------------------------------------------------------------------------------------------------------
OV<UInt32> CBitParceller::readUEColumbusCode(OI<SError>& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Compose value
	SInt8	leadingZeroBits = -1;
	for (UInt8 b = 0; !b; leadingZeroBits++) {
		// Reload current byte
		mInternals->reloadCurrentByte(outError);
		ReturnValueIfError(outError, OV<UInt32>());

		// Read bit
		b = mInternals->readBits(1);
	}

	OV<UInt32>	value = readUInt32(leadingZeroBits, outError);
	ReturnValueIfError(outError, OV<UInt32>());

	return OV<UInt32>((1 << leadingZeroBits) - 1 + *value);
}

//----------------------------------------------------------------------------------------------------------------------
void CBitParceller::reset() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Reset
	mInternals->mNeedToSetPos = true;
	mInternals->mDataSource->reset();

	mInternals->mCurrentByte = 0;
	mInternals->mCurrentByteBitsAvailable = 0;
}
