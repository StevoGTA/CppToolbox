//----------------------------------------------------------------------------------------------------------------------
//	CFileDataSource.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFileDataSource.h"

#include "CFileReader.h"
#include "CppToolboxAssert.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CFileDataSourceInternals

class CFileDataSourceInternals {
	public:
						CFileDataSourceInternals(const CFile& file) : mFile(file), mFileReader(nil) {}
						~CFileDataSourceInternals()
							{
								reset();
							}

		CFileReader&	getFileReader()
							{
								// Check if have file reader
								if (mFileReader == nil) {
									// Setup
									mFileReader = new CFileReader(mFile);
									mError = mFileReader->open();
								}

								return *mFileReader;
							}
		void			reset()
							{
								// Cleanup
								Delete(mFileReader);

								// No longer any error
								mError = OI<SError>();
							}

		CFile			mFile;
		CFileReader*	mFileReader;
		OI<SError>		mError;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFileDataSource

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CFileDataSource::CFileDataSource(const CFile& file) : CDataSource()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CFileDataSourceInternals(file);
}

//----------------------------------------------------------------------------------------------------------------------
CFileDataSource::~CFileDataSource()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CDataSource methods

//----------------------------------------------------------------------------------------------------------------------
UInt64 CFileDataSource::getSize() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mFile.getSize();
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CFileDataSource::readData(void* buffer, UInt64 byteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->getFileReader().readData(buffer, byteCount);
}

//----------------------------------------------------------------------------------------------------------------------
SInt64 CFileDataSource::getPos() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->getFileReader().getPos();
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CFileDataSource::setPos(Position position, SInt64 newPos)
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->getFileReader().setPos((CFileReader::Position) position, newPos);
}

//----------------------------------------------------------------------------------------------------------------------
void CFileDataSource::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Reset
	mInternals->reset();
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMappedFileDataSourceInternals

class CMappedFileDataSourceInternals {
	public:
						CMappedFileDataSourceInternals(const CFile& file) :
							mFile(file), mFileReader(nil), mFileMemoryMap(nil), mCurrentOffset(0)
							{}
						~CMappedFileDataSourceInternals()
							{
								reset();
							}

		CFileMemoryMap&	getFileMemoryMap()
							{
								// Check if have file memory map
								if (mFileMemoryMap == nil) {
									// Setup
									mFileReader = new CFileReader(mFile);
									mError = mFileReader->open();

									mFileMemoryMap =
											new CFileMemoryMap(
													mFileReader->getFileMemoryMap(0, mFile.getSize(), mError));
								}

								return *mFileMemoryMap;
							}
		void			reset()
							{
								// Cleanup
								Delete(mFileMemoryMap);
								Delete(mFileReader);

								// No longer any error
								mError = OI<SError>();

								// Reset
								mCurrentOffset = 0;
							}

		CFile			mFile;
		CFileReader*	mFileReader;
		CFileMemoryMap*	mFileMemoryMap;
		OI<SError>		mError;

		UInt64			mCurrentOffset;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMappedFileDataSource

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CMappedFileDataSource::CMappedFileDataSource(const CFile& file) : CDataSource()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CMappedFileDataSourceInternals(file);
}

//----------------------------------------------------------------------------------------------------------------------
CMappedFileDataSource::~CMappedFileDataSource()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CDataSource methods

//----------------------------------------------------------------------------------------------------------------------
UInt64 CMappedFileDataSource::getSize() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->getFileMemoryMap().getByteCount();
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CMappedFileDataSource::readData(void* buffer, UInt64 byteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	AssertFailIf(mInternals->mError.hasInstance());
	if (mInternals->mError.hasInstance())
		return mInternals->mError;

	// Setup
	OI<SError>	error;

	// Parameter and internals check
	AssertFailIf(byteCount > (mInternals->getFileMemoryMap().getByteCount() - mInternals->mCurrentOffset));
	if (byteCount > (mInternals->getFileMemoryMap().getByteCount() - mInternals->mCurrentOffset))
		// Attempting to ready beyond end of data
		return OI<SError>(SError::mEndOfData);

	// Copy data
	::memcpy(buffer, (UInt8*) mInternals->getFileMemoryMap().getBytePtr() + mInternals->mCurrentOffset, byteCount);
	mInternals->mCurrentOffset += byteCount;

	return error;
}

//----------------------------------------------------------------------------------------------------------------------
SInt64 CMappedFileDataSource::getPos() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mCurrentOffset;
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CMappedFileDataSource::setPos(Position position, SInt64 newPos)
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	AssertFailIf(mInternals->mError.hasInstance());
	if (mInternals->mError.hasInstance())
		return mInternals->mError;

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
			offset = mInternals->getFileMemoryMap().getByteCount() - newPos;
			break;
	}

	// Ensure new offset is within available window
	AssertFailIf(offset < 0)
	if (offset < 0)
		return CDataSource::mSetPosBeforeStartError;

	AssertFailIf(offset > (SInt64) mInternals->getFileMemoryMap().getByteCount());
	if (offset > (SInt64) mInternals->getFileMemoryMap().getByteCount())
		return CDataSource::mSetPosAfterEndError;

	// Set
	mInternals->mCurrentOffset = offset;

	return OI<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
void CMappedFileDataSource::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Reset
	mInternals->reset();
}
