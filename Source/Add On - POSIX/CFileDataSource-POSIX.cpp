//----------------------------------------------------------------------------------------------------------------------
//	CFileDataSource-POSIX.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFileDataSource.h"

#include "ConcurrencyPrimitives.h"
#include "SError-POSIX.h"

#include <sys/mman.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CFileDataSourceInternals

class CFileDataSourceInternals {
	public:
		CFileDataSourceInternals(const CFile& file, bool buffered) :
			mFile(file), mByteCount(mFile.getSize()), mFILE(nil), mFD(-1)
			{
				// Setup
				CString::C	path = mFile.getFilesystemPath().getString().getCString(CString::kEncodingUTF8);

				// Check buffered
				if (buffered) {
					// Buffered
					mFILE = ::fopen(*path, "rb");

					if (mFILE == nil) {
						// Unable to open
						mError = OI<SError>(SErrorFromPOSIXerror(errno));
						CLogServices::logError(*mError, "opening buffered", __FILE__, __func__, __LINE__);
					}
				} else {
					// Not buffered
					mFD = ::open(*path, O_RDONLY, 0);
					if (mFD == -1) {
						// Unable to open
						mError = OI<SError>(SErrorFromPOSIXerror(errno));
						CLogServices::logError(*mError, "opening non-buffered", __FILE__, __func__, __LINE__);
					}
				}
			}
		~CFileDataSourceInternals()
			{
				if (mFILE != nil)
					::fclose(mFILE);
				if (mFD != -1)
					::close(mFD);
			}

		CFile		mFile;
		UInt64		mByteCount;
		CLock		mLock;

		FILE*		mFILE;
		SInt32		mFD;
		OI<SError>	mError;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFileDataSource

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CFileDataSource::CFileDataSource(const CFile& file, bool buffered) : CSeekableDataSource()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CFileDataSourceInternals(file, buffered);
}

//----------------------------------------------------------------------------------------------------------------------
CFileDataSource::~CFileDataSource()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CSeekableDataSource methods

//----------------------------------------------------------------------------------------------------------------------
UInt64 CFileDataSource::getSize() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mByteCount;
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CFileDataSource::readData(UInt64 position, void* buffer, CData::Size byteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check for error
	if (mInternals->mError.hasInstance())
		// Error
		return mInternals->mError;

	// Preflight
	AssertFailIf((position + byteCount) > mInternals->mByteCount);
	if ((position + byteCount) > mInternals->mByteCount)
		// Attempting to ready beyond end of data
		return OI<SError>(SError::mEndOfData);

	// One at a time
	mInternals->mLock.lock();

	// Check mode
	OI<SError>	error;
	if (mInternals->mFILE != nil) {
		// FILE
		off_t	offset = ::fseeko(mInternals->mFILE, position, SEEK_SET);
		if (offset != -1) {
			// Read
			ssize_t	bytesRead = ::fread(buffer, 1, (size_t) byteCount, mInternals->mFILE);
			if (bytesRead != (ssize_t) byteCount) {
				// Error
				error = OI<SError>(SErrorFromPOSIXerror(errno));
				CLogServices::logError(*error, "reading data buffered", __FILE__, __func__, __LINE__);
			}
		} else {
			// Error
			error = OI<SError>(SErrorFromPOSIXerror(errno));
			CLogServices::logError(*error, "setting position buffered", __FILE__, __func__, __LINE__);
		}
	} else {
		// file
		off_t	offset = ::lseek(mInternals->mFD, position, SEEK_SET);
		if (offset != -1) {
			// Read
			ssize_t bytesRead = ::read(mInternals->mFD, buffer, (size_t) byteCount);
			if (bytesRead == -1) {
				// Error
				error = OI<SError>(SErrorFromPOSIXerror(errno));
				CLogServices::logError(*error, "reading data non-buffered", __FILE__, __func__, __LINE__);
			}
		} else {
			// Error
			error = OI<SError>(SErrorFromPOSIXerror(errno));
			CLogServices::logError(*error, "setting non-position buffered", __FILE__, __func__, __LINE__);
		}
	}

	// Done
	mInternals->mLock.unlock();

	return error;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMappedFileDataSourceInternals

class CMappedFileDataSourceInternals {
	public:
		CMappedFileDataSourceInternals(const CFile& file, UInt64 byteOffset, UInt64 byteCount) :
			mFile(file)
			{
				// Open
				CString::C	path = mFile.getFilesystemPath().getString().getCString(CString::kEncodingUTF8);
				mFD = ::open(*path, O_RDONLY, 0);
				if (mFD != -1) {
					// Limit to bytes remaining
					byteCount = std::min<UInt64>(byteCount, mFile.getSize() - byteOffset);

					// Create map
					void*	bytePtr =
									::mmap(nil, (size_t) byteCount, PROT_READ, MAP_FILE | MAP_PRIVATE, mFD, byteOffset);

					// Check for failure
					if (bytePtr != (void*) -1) {
						// Success
						mBytePtr = bytePtr;
						mByteCount = byteCount;
					} else {
						// Failed
						mBytePtr = nil;
						mByteCount = 0;
						mError = OI<SError>(SErrorFromPOSIXerror(errno));
						CLogServices::logError(*mError, "mapping data", __FILE__, __func__, __LINE__);
					}
				} else {
					// Unable to open
					mBytePtr = nil;
					mByteCount = 0;
					mError = OI<SError>(SErrorFromPOSIXerror(errno));
					CLogServices::logError(*mError, "opening", __FILE__, __func__, __LINE__);
				}
			}
		~CMappedFileDataSourceInternals()
			{
				if (mBytePtr != nil)
					::munmap(mBytePtr, (size_t) mByteCount);
				if (mFD != -1)
					::close(mFD);
			}

		CFile		mFile;

		SInt32		mFD;
		void*		mBytePtr;
		UInt64		mByteCount;
		OI<SError>	mError;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMappedFileDataSource

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CMappedFileDataSource::CMappedFileDataSource(const CFile& file, UInt64 byteOffset, UInt64 byteCount) :
		CSeekableDataSource()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CMappedFileDataSourceInternals(file, byteOffset, byteCount);
}

//----------------------------------------------------------------------------------------------------------------------
CMappedFileDataSource::CMappedFileDataSource(const CFile& file) : CSeekableDataSource()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CMappedFileDataSourceInternals(file, 0, file.getSize());
}

//----------------------------------------------------------------------------------------------------------------------
CMappedFileDataSource::~CMappedFileDataSource()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CSeekableDataSource methods

//----------------------------------------------------------------------------------------------------------------------
UInt64 CMappedFileDataSource::getSize() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mByteCount;
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CMappedFileDataSource::readData(UInt64 position, void* buffer, CData::Size byteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check for error
	if (mInternals->mError.hasInstance())
		// Error
		return mInternals->mError;

	// Preflight
	AssertFailIf((position + byteCount) > mInternals->mByteCount);
	if ((position + byteCount) > mInternals->mByteCount)
		// Attempting to ready beyond end of data
		return OI<SError>(SError::mEndOfData);

	// Copy bytes
	::memcpy(buffer, (UInt8*) mInternals->mBytePtr + position, byteCount);

	return OI<SError>();
}
