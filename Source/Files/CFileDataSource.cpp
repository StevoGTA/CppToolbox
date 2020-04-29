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
						CFileDataSourceInternals(const CFile& file) : mFile(file), mFileReader(nil), mError(kNoError) {}
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
								mError = kNoError;
							}

		CFile			mFile;
		CFileReader*	mFileReader;
		UError			mError;
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
UError CFileDataSource::readData(void* buffer, UInt64 byteCount) const
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
UError CFileDataSource::setPos(EDataSourcePosition position, SInt64 newPos) const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->getFileReader().setPos((EFileReaderPositionMode) position, newPos);
}

//----------------------------------------------------------------------------------------------------------------------
void CFileDataSource::reset() const
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->reset();
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMappedFileDataSourceInternals

class CMappedFileDataSourceInternals {
	public:
						CMappedFileDataSourceInternals(const CFile& file) :
							mFile(file), mFileReader(nil), mFileMemoryMap(nil), mError(kNoError), mCurrentOffset(0)
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
								mError = kNoError;

								// Reset
								mCurrentOffset = 0;
							}

		CFile			mFile;
		CFileReader*	mFileReader;
		CFileMemoryMap*	mFileMemoryMap;
		UError			mError;

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
UError CMappedFileDataSource::readData(void* buffer, UInt64 byteCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	AssertFailIf(mInternals->mError != kNoError);
	if (mInternals->mError != kNoError)
		return mInternals->mError;

	// Setup
	UError	error = kNoError;

	// Parameter and internals check
	AssertFailIf(byteCount > (mInternals->getFileMemoryMap().getByteCount() - mInternals->mCurrentOffset));
	if (byteCount > (mInternals->getFileMemoryMap().getByteCount() - mInternals->mCurrentOffset))
		// Attempting to ready beyond end of data
		return kDataProviderReadBeyondEndError;

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
UError CMappedFileDataSource::setPos(EDataSourcePosition position, SInt64 newPos) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	AssertFailIf(mInternals->mError != kNoError);
	if (mInternals->mError != kNoError)
		return mInternals->mError;

	// Figure new offset
	SInt64	offset;
	switch (position) {
		case kDataSourcePositionFromBeginning:
			// From beginning
			offset = newPos;
			break;

		case kDataSourcePositionFromCurrent:
			// From current
			offset = mInternals->mCurrentOffset + newPos;
			break;

		case kDataSourcePositionFromEnd:
			// From end
			offset = mInternals->getFileMemoryMap().getByteCount() - newPos;
			break;
	}

	// Ensure new offset is within available window
	AssertFailIf(newPos < 0)
	if (newPos < 0)
		return kDataProviderSetPosBeforeStartError;

	AssertFailIf(newPos >= (SInt64) mInternals->getFileMemoryMap().getByteCount());
	if (newPos >= (SInt64) mInternals->getFileMemoryMap().getByteCount())
		return kDataProviderSetPosAfterEndError;

	// Set
	mInternals->mCurrentOffset = offset;

	return kNoError;
}

//----------------------------------------------------------------------------------------------------------------------
void CMappedFileDataSource::reset() const
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->reset();
}
