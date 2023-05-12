//----------------------------------------------------------------------------------------------------------------------
//	CDataSource.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CDataSource.h"

#include "CData.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CRandomAccessDataSource

// MARK: Properties

const	SError	CRandomAccessDataSource::mSetPosBeforeStartError(CString(OSSTR("CDataSource")), 1,
						CString(OSSTR("Data source set position before start")));
const	SError	CRandomAccessDataSource::mSetPosAfterEndError(CString(OSSTR("CDataSource")), 2,
						CString(OSSTR("Data source set position after end")));

// MARK: CDataSource methods

//----------------------------------------------------------------------------------------------------------------------
TVResult<CData> CRandomAccessDataSource::readData()
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CData	data((CData::ByteCount) getByteCount());

	// Read
	OV<SError>	error = readData(0, data.getMutableBytePtr(), getByteCount());
	ReturnValueIfError(error, TVResult<CData>(*error));

	return TVResult<CData>(data);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDataDataSource::Internals

class CDataDataSource::Internals {
	public:
		Internals(const CData& data) : mData(data), mCurrentOffset(0) {}

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
	mInternals = new Internals(data);
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
OV<SError> CDataDataSource::readData(UInt64 position, void* buffer, CData::ByteCount byteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	AssertFailIf((position + byteCount) > mInternals->mData.getByteCount());
	if ((position + byteCount) > mInternals->mData.getByteCount())
		// Attempting to ready beyond end of data
		return OV<SError>(SError::mEndOfData);

	// Copy bytes
	::memcpy(buffer, (UInt8*) mInternals->mData.getBytePtr() + position, (size_t) byteCount);

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CData> CDataDataSource::readData(UInt64 position, CData::ByteCount byteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	AssertFailIf((position + byteCount) > mInternals->mData.getByteCount());
	if ((position + byteCount) > mInternals->mData.getByteCount())
		// Attempting to ready beyond end of data
		return TVResult<CData>(SError::mEndOfData);

	return TVResult<CData>(CData((UInt8*) mInternals->mData.getBytePtr() + position, byteCount, false));
}
