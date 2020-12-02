//----------------------------------------------------------------------------------------------------------------------
//	CFileDataSource.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFileDataSource.h"

#include "CFileReader.h"
#include "CppToolboxAssert.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CFileDataSourceInternals

class CFileDataSourceInternals : public TCopyOnWriteReferenceCountable<CFileDataSourceInternals> {
	public:
						CFileDataSourceInternals(const CFile& file) : mFile(file), mFileReader(nil), mError(kNoError) {}
						CFileDataSourceInternals(const CFileDataSourceInternals& other) :
							mFile(other.mFile), mFileReader(nil), mError(kNoError)
							{}
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
CFileDataSource::CFileDataSource(const CFileDataSource& other) : CDataSource()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CFileDataSource::~CFileDataSource()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: CDataSource methods

//----------------------------------------------------------------------------------------------------------------------
UInt64 CFileDataSource::getSize() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mFile.getSize();
}

//----------------------------------------------------------------------------------------------------------------------
UError CFileDataSource::readData(void* buffer, UInt64 byteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for change
	mInternals = mInternals->prepareForWrite();

	return mInternals->getFileReader().readData(buffer, byteCount);
}

//----------------------------------------------------------------------------------------------------------------------
SInt64 CFileDataSource::getPos() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->getFileReader().getPos();
}

//----------------------------------------------------------------------------------------------------------------------
UError CFileDataSource::setPos(EDataSourcePosition position, SInt64 newPos)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for change
	mInternals = mInternals->prepareForWrite();

	return mInternals->getFileReader().setPos((EFileReaderPositionMode) position, newPos);
}

//----------------------------------------------------------------------------------------------------------------------
void CFileDataSource::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for change
	mInternals = mInternals->prepareForWrite();

	// Reset
	mInternals->reset();
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMappedFileDataSourceInternals

class CMappedFileDataSourceInternals : public TCopyOnWriteReferenceCountable<CMappedFileDataSourceInternals> {
	public:
						CMappedFileDataSourceInternals(const CFile& file) :
							mFile(file), mFileReader(nil), mFileMemoryMap(nil), mError(kNoError), mCurrentOffset(0)
							{}
						CMappedFileDataSourceInternals(const CMappedFileDataSourceInternals& other) :
							mFile(other.mFile), mFileReader(nil), mFileMemoryMap(nil), mError(kNoError),
									mCurrentOffset(0)
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
CMappedFileDataSource::CMappedFileDataSource(const CMappedFileDataSource& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CMappedFileDataSource::~CMappedFileDataSource()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: CDataSource methods

//----------------------------------------------------------------------------------------------------------------------
UInt64 CMappedFileDataSource::getSize() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->getFileMemoryMap().getByteCount();
}

//----------------------------------------------------------------------------------------------------------------------
UError CMappedFileDataSource::readData(void* buffer, UInt64 byteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for change
	mInternals = mInternals->prepareForWrite();

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
UError CMappedFileDataSource::setPos(EDataSourcePosition position, SInt64 newPos)
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	AssertFailIf(mInternals->mError != kNoError);
	if (mInternals->mError != kNoError)
		return mInternals->mError;

	// Prepare for change
	mInternals = mInternals->prepareForWrite();

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
	AssertFailIf(offset < 0)
	if (offset < 0)
		return kDataProviderSetPosBeforeStartError;

	AssertFailIf(offset > (SInt64) mInternals->getFileMemoryMap().getByteCount());
	if (offset > (SInt64) mInternals->getFileMemoryMap().getByteCount())
		return kDataProviderSetPosAfterEndError;

	// Set
	mInternals->mCurrentOffset = offset;

	return kNoError;
}

//----------------------------------------------------------------------------------------------------------------------
void CMappedFileDataSource::reset()
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for change
	mInternals = mInternals->prepareForWrite();

	// Reset
	mInternals->reset();
}
