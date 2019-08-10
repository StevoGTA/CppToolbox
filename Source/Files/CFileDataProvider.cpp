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
							mFile(file), mFileReader(nil), mError(kNoError)
							{}
						~CFileDataProviderInternals()
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
								DisposeOf(mFileReader);

								// No longer any error
								mError = kNoError;
							}

		CFile			mFile;
		CFileReader*	mFileReader;
		UError			mError;
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
	return mInternals->getFileReader().readData(buffer, byteCount);
}

//----------------------------------------------------------------------------------------------------------------------
SInt64 CFileDataProvider::getPos() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->getFileReader().getPos();
}

//----------------------------------------------------------------------------------------------------------------------
UError CFileDataProvider::setPos(EDataProviderPosition position, SInt64 newPos) const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->getFileReader().setPos((EFileReaderPositionMode) position, newPos);
}

//----------------------------------------------------------------------------------------------------------------------
void CFileDataProvider::reset() const
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->reset();
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMappedFileDataProviderInternals

class CMappedFileDataProviderInternals {
	public:
						CMappedFileDataProviderInternals(const CFile& file) :
							mFile(file), mFileReader(nil), mFileMemoryMap(nil), mCurrentOffset(0)
							{}
						~CMappedFileDataProviderInternals()
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
								DisposeOf(mFileMemoryMap);
								DisposeOf(mFileReader);

								// No longer any error
								mError = kNoError;
							}

		CFile			mFile;
		CFileReader*	mFileReader;
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
	return mInternals->getFileMemoryMap().getByteCount();
}

//----------------------------------------------------------------------------------------------------------------------
UError CMappedFileDataProvider::readData(void* buffer, UInt64 byteCount) const
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
	AssertFailIf(mInternals->mError != kNoError);
	if (mInternals->mError != kNoError)
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
			offset = mInternals->getFileMemoryMap().getByteCount() - newPos;
			break;
	}

	// Ensure new offset is within available window
	AssertFailIf(newPos < 0)
	if (newPos < 0)
		return kDataProviderSetPosBeforeStartError;

	AssertFailIf(newPos >= mInternals->getFileMemoryMap().getByteCount());
	if (newPos >= mInternals->getFileMemoryMap().getByteCount())
		return kDataProviderSetPosAfterEndError;

	// Set
	mInternals->mCurrentOffset = offset;

	return kNoError;
}

//----------------------------------------------------------------------------------------------------------------------
void CMappedFileDataProvider::reset() const
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->reset();
}
