//----------------------------------------------------------------------------------------------------------------------
//	CDataSource.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CDataSource.h"

#include "CData.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CDataSource

// MARK: Properties

SError	CDataSource::mSetPosBeforeStartError(CString(OSSTR("CDataSource")), 1,
				CString(OSSTR("Data source set position before start")));
SError	CDataSource::mSetPosAfterEndError(CString(OSSTR("CDataSource")), 2,
				CString(OSSTR("Data source set position after end")));

//----------------------------------------------------------------------------------------------------------------------
OI<CData> CDataSource::readData(UInt64 byteCount, OI<SError>& outError)
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
OI<CData> CDataSource::readData(OI<SError>& outError)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	UInt64		byteCount = getSize() - getPos();
	OI<CData>	data(new CData((CData::Size) byteCount));

	// Read
	outError = readData(data->getMutableBytePtr(), byteCount);
	ReturnValueIfError(outError, OI<CData>());

	return data;
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
OI<SError> CDataDataSource::readData(void* buffer, UInt64 byteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	if (byteCount > (mInternals->mData.getSize() - mInternals->mCurrentOffset))
		// Attempting to ready beyond end of data
		return OI<SError>(SError::mEndOfData);

	// Copy
	::memcpy(buffer, (UInt8*) mInternals->mData.getBytePtr() + mInternals->mCurrentOffset, byteCount);

	// Update
	mInternals->mCurrentOffset += byteCount;

	return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
SInt64 CDataDataSource::getPos() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mCurrentOffset;
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CDataDataSource::setPos(Position position, SInt64 newPos)
//----------------------------------------------------------------------------------------------------------------------
{
	// Figure new offset
	SInt64	offset;
	switch (position) {
		case kPositionFromBeginning:
			// From beginning
			offset = newPos;
			break;

		case kPositionFromCurrent:
			// From current
			offset = mInternals->mCurrentOffset + newPos;
			break;

		case kPositionFromEnd:
			// From end
			offset = mInternals->mData.getSize() - newPos;
			break;
	}

	// Ensure new offset is within available window
	AssertFailIf(offset < 0)
	if (offset < 0)
		return CDataSource::mSetPosBeforeStartError;

	AssertFailIf(offset > (SInt64) mInternals->mData.getSize());
	if (offset > (SInt64) mInternals->mData.getSize())
		return CDataSource::mSetPosAfterEndError;

	// Set
	mInternals->mCurrentOffset = offset;

	return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
void CDataDataSource::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mCurrentOffset = 0;
}
