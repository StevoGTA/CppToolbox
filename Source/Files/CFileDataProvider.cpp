//----------------------------------------------------------------------------------------------------------------------
//	CFileDataProvider.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFileDataProvider.h"

#include "CFileReader.h"
#include "CppToolboxAssert.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CFileDataProviderInternals

class CFileDataProviderInternals {
	public:
		CFileDataProviderInternals(const CFile& file) :
			mFile(file), mFileReader(mFile), mError(mFileReader.open(true))
			{}
		~CFileDataProviderInternals() {}

		CFile		mFile;
		CFileReader	mFileReader;
		UError		mError;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFileDataProvider

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CFileDataProvider::CFileDataProvider(const CFile& file) : CDataProvider()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CFileDataProviderInternals(file);
}

//----------------------------------------------------------------------------------------------------------------------
CFileDataProvider::~CFileDataProvider()
//----------------------------------------------------------------------------------------------------------------------
{
	DisposeOf(mInternals);
}

// MARK: CDataProvider methods

//----------------------------------------------------------------------------------------------------------------------
UInt64 CFileDataProvider::getSize() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mFile.getSize();
}

//----------------------------------------------------------------------------------------------------------------------
UError CFileDataProvider::readData(void* buffer, UInt64 byteCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mFileReader.readData(buffer, byteCount);
}

//----------------------------------------------------------------------------------------------------------------------
SInt64 CFileDataProvider::getPos() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mFileReader.getPos();
}

//----------------------------------------------------------------------------------------------------------------------
UError CFileDataProvider::setPos(EDataProviderPosition position, SInt64 newPos) const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mFileReader.setPos((EFileReaderPositionMode) position, newPos);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMappedFileDataProviderInternals

class CMappedFileDataProviderInternals {
	public:
		CMappedFileDataProviderInternals(const CFile& file) :
			mFile(file), mFileReader(mFile), mFileMemoryMap(nil), mCurrentOffset(0)
			{
				// Open file reader
				mError = mFileReader.open();
				ReturnIfError(mError);

				// Create file memory map
				mFileMemoryMap = new CFileMemoryMap(mFileReader.getFileMemoryMap(0, mFile.getSize(), mError));
			}
		~CMappedFileDataProviderInternals()
			{
				DisposeOf(mFileMemoryMap);
			}

		CFile			mFile;
		CFileReader		mFileReader;
		CFileMemoryMap*	mFileMemoryMap;
		UError			mError;

		UInt64			mCurrentOffset;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMappedFileDataProvider

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CMappedFileDataProvider::CMappedFileDataProvider(const CFile& file) : CDataProvider()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CMappedFileDataProviderInternals(file);
}

//----------------------------------------------------------------------------------------------------------------------
CMappedFileDataProvider::~CMappedFileDataProvider()
//----------------------------------------------------------------------------------------------------------------------
{
	DisposeOf(mInternals);
}

// MARK: CDataProvider methods

//----------------------------------------------------------------------------------------------------------------------
UInt64 CMappedFileDataProvider::getSize() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	AssertFailIf(mInternals->mFileMemoryMap == nil);
	if (mInternals->mFileMemoryMap == nil)
		return 0;

	return mInternals->mFileMemoryMap->getByteCount();
}

//----------------------------------------------------------------------------------------------------------------------
UError CMappedFileDataProvider::readData(void* buffer, UInt64 byteCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	AssertFailIf(mInternals->mFileMemoryMap == nil);
	if (mInternals->mFileMemoryMap == nil)
		return mInternals->mError;

	// Setup
	UError	error = kNoError;

	// Parameter and internals check
	AssertFailIf(byteCount > (mInternals->mFileMemoryMap->getByteCount() - mInternals->mCurrentOffset));
	if (byteCount > (mInternals->mFileMemoryMap->getByteCount() - mInternals->mCurrentOffset))
		// Attempting to ready beyond end of data
		return kDataProviderReadBeyondEndError;

	// Copy data
	::memcpy(buffer, (UInt8*) mInternals->mFileMemoryMap->getBytePtr() + mInternals->mCurrentOffset, byteCount);
	mInternals->mCurrentOffset += byteCount;

	return error;
}

//----------------------------------------------------------------------------------------------------------------------
SInt64 CMappedFileDataProvider::getPos() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mCurrentOffset;
}

//----------------------------------------------------------------------------------------------------------------------
UError CMappedFileDataProvider::setPos(EDataProviderPosition position, SInt64 newPos) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Preflight
	AssertFailIf(mInternals->mFileMemoryMap == nil);
	if (mInternals->mFileMemoryMap == nil)
		return mInternals->mError;

	// Figure new offset
	SInt64	offset;
	switch (position) {
		case kDataProviderPositionFromBeginning:
			// From beginning
			offset = newPos;
			break;

		case kDataProviderPositionFromCurrent:
			// From current
			offset = mInternals->mCurrentOffset + newPos;
			break;

		case kDataProviderPositionFromEnd:
			// From end
			offset = mInternals->mFileMemoryMap->getByteCount() - newPos;
			break;
	}

	// Ensure new offset is within available window
	AssertFailIf(newPos < 0)
	if (newPos < 0)
		return kDataProviderSetPosBeforeStartError;

	AssertFailIf(newPos >= mInternals->mFileMemoryMap->getByteCount());
	if (newPos >= mInternals->mFileMemoryMap->getByteCount())
		return kDataProviderSetPosAfterEndError;

	// Set
	mInternals->mCurrentOffset = offset;

	return kNoError;
}
