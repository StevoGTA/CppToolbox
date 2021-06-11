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
	CData	data((CData::Size) getSize());

	// Read
	OI<SError>	error = readData(0, data.getMutableBytePtr(), getSize());
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
UInt64 CDataDataSource::getSize() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mData.getSize();
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CDataDataSource::readData(UInt64 position, void* buffer, CData::Size byteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	AssertFailIf((position + byteCount) > mInternals->mData.getSize());
	if ((position + byteCount) > mInternals->mData.getSize())
		// Attempting to ready beyond end of data
		return OI<SError>(SError::mEndOfData);

	// Copy bytes
	::memcpy(buffer, (UInt8*) mInternals->mData.getBytePtr() + position, byteCount);

	return OI<SError>();
}
