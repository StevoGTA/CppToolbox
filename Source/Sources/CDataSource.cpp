//----------------------------------------------------------------------------------------------------------------------
//	CDataSource.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CDataSource.h"

#include "CData.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CSeekableDataSource

// MARK: Properties

SError	CSeekableDataSource::mSetPosBeforeStartError(CString(OSSTR("CDataSource")), 1,
				CString(OSSTR("Data source set position before start")));
SError	CSeekableDataSource::mSetPosAfterEndError(CString(OSSTR("CDataSource")), 2,
				CString(OSSTR("Data source set position after end")));

// MARK: CDataSource methods

//----------------------------------------------------------------------------------------------------------------------
TIResult<CData> CSeekableDataSource::readData()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CData	data((CData::ByteCount) getByteCount());

	// Read
	OI<SError>	error = readData(0, data.getMutableBytePtr(), getByteCount());
	ReturnValueIfError(error, TIResult<CData>(*error));

	return TIResult<CData>(data);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDataDataSourceInternals

class CDataDataSourceInternals {
	public:
		CDataDataSourceInternals(const CData& data) : mData(data), mCurrentOffset(0) {}

		CData	mData;
		UInt64	mCurrentOffset;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDataDataSource

// Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CDataDataSource::CDataDataSource(const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CDataDataSourceInternals(data);
}

//----------------------------------------------------------------------------------------------------------------------
CDataDataSource::~CDataDataSource()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// CDataSource methods

//----------------------------------------------------------------------------------------------------------------------
UInt64 CDataDataSource::getByteCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mData.getByteCount();
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CDataDataSource::readData(UInt64 position, void* buffer, CData::ByteCount byteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	AssertFailIf((position + byteCount) > mInternals->mData.getByteCount());
	if ((position + byteCount) > mInternals->mData.getByteCount())
		// Attempting to ready beyond end of data
		return OI<SError>(SError::mEndOfData);

	// Copy bytes
	::memcpy(buffer, (UInt8*) mInternals->mData.getBytePtr() + position, (size_t) byteCount);

	return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
TIResult<CData> CDataDataSource::readData(UInt64 position, CData::ByteCount byteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	AssertFailIf((position + byteCount) > mInternals->mData.getByteCount());
	if ((position + byteCount) > mInternals->mData.getByteCount())
		// Attempting to ready beyond end of data
		return TIResult<CData>(SError::mEndOfData);

	return TIResult<CData>(CData((UInt8*) mInternals->mData.getBytePtr() + position, byteCount, false));
}
