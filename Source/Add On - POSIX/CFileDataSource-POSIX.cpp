//----------------------------------------------------------------------------------------------------------------------
//	CFileDataSource-POSIX.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFileDataSource.h"

#include "ConcurrencyPrimitives.h"
#include "SError-POSIX.h"

#include <sys/mman.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: Macros

#define	CFileDataSourceReportError(error, message)													\
				{																					\
					CLogServices::logError(error, message,											\
							CString(__FILE__, sizeof(__FILE__), CString::kEncodingUTF8),			\
							CString(__func__, sizeof(__func__), CString::kEncodingUTF8), __LINE__);	\
				}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFileDataSource::Internals

class CFileDataSource::Internals {
	public:
		Internals(const CFile& file, bool buffered) :
			mByteCount(file.getByteCount()), mFILE(nil), mFD(-1)
			{
				// Setup
				CString::C	path = file.getFilesystemPath().getString().getUTF8String();

				// Check buffered
				if (buffered) {
					// Buffered
					mFILE = ::fopen(*path, "rb");

					if (mFILE == nil) {
						// Unable to open
						mError = OV<SError>(SErrorFromPOSIXerror(errno));
						CFileDataSourceReportError(*mError, CString(OSSTR("opening buffered")));
					}
				} else {
					// Not buffered
					mFD = ::open(*path, O_RDONLY, 0);
					if (mFD == -1) {
						// Unable to open
						mError = OV<SError>(SErrorFromPOSIXerror(errno));
						CFileDataSourceReportError(*mError, CString(OSSTR("opening non-buffered")));
					}
				}
			}
		~Internals()
			{
				if (mFILE != nil)
					::fclose(mFILE);
				if (mFD != -1)
					::close(mFD);
			}

		UInt64		mByteCount;
		CLock		mLock;

		FILE*		mFILE;
		SInt32		mFD;
		OV<SError>	mError;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFileDataSource

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CFileDataSource::CFileDataSource(const CFile& file, bool buffered) : CRandomAccessDataSource()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(file, buffered);
}

//----------------------------------------------------------------------------------------------------------------------
CFileDataSource::~CFileDataSource()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CRandomAccessDataSource methods

//----------------------------------------------------------------------------------------------------------------------
UInt64 CFileDataSource::getByteCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mByteCount;
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFileDataSource::readData(UInt64 position, void* buffer, CData::ByteCount byteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check for error
	if (mInternals->mError.hasValue())
		// Error
		return mInternals->mError;

	// Preflight
	AssertFailIf((position + byteCount) > mInternals->mByteCount);
	if ((position + byteCount) > mInternals->mByteCount)
		// Attempting to ready beyond end of data
		return OV<SError>(SError::mEndOfData);

	// One at a time
	mInternals->mLock.lock();

	// Check mode
	OV<SError>	error;
	if (mInternals->mFILE != nil) {
		// FILE
		off_t	offset = ::fseeko(mInternals->mFILE, position, SEEK_SET);
		if (offset != -1) {
			// Read
			ssize_t	bytesRead = ::fread(buffer, 1, (size_t) byteCount, mInternals->mFILE);
			if (bytesRead != (ssize_t) byteCount) {
				// Error
				error = OV<SError>(SErrorFromPOSIXerror(errno));
				CFileDataSourceReportError(*error, CString(OSSTR("reading data buffered")));
			}
		} else {
			// Error
			error = OV<SError>(SErrorFromPOSIXerror(errno));
			CFileDataSourceReportError(*error, CString(OSSTR("setting position buffered")));
		}
	} else {
		// file
		off_t	offset = ::lseek(mInternals->mFD, position, SEEK_SET);
		if (offset != -1) {
			// Read
			ssize_t bytesRead = ::read(mInternals->mFD, buffer, (size_t) byteCount);
			if (bytesRead == -1) {
				// Error
				error = OV<SError>(SErrorFromPOSIXerror(errno));
				CFileDataSourceReportError(*error, CString(OSSTR("reading data non-buffered")));
			}
		} else {
			// Error
			error = OV<SError>(SErrorFromPOSIXerror(errno));
			CFileDataSourceReportError(*error, CString(OSSTR("setting non-position buffered")));
		}
	}

	// Done
	mInternals->mLock.unlock();

	return error;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMappedFileDataSource::Internals

class CMappedFileDataSource::Internals {
	public:
		Internals(const CFile& file, UInt64 byteOffset, UInt64 byteCount)
			{
				// Open
				CString::C	path = file.getFilesystemPath().getString().getUTF8String();
				mFD = ::open(*path, O_RDONLY, 0);
				if (mFD != -1) {
					// Limit to bytes remaining
					byteCount = std::min<UInt64>(byteCount, file.getByteCount() - byteOffset);

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
						mError = OV<SError>(SErrorFromPOSIXerror(errno));
						CFileDataSourceReportError(*mError, CString(OSSTR("mapping data")));
					}
				} else {
					// Unable to open
					mBytePtr = nil;
					mByteCount = 0;
					mError = OV<SError>(SErrorFromPOSIXerror(errno));
					CFileDataSourceReportError(*mError, CString(OSSTR("opening")));
				}
			}
		~Internals()
			{
				if (mBytePtr != nil)
					::munmap(mBytePtr, (size_t) mByteCount);
				if (mFD != -1)
					::close(mFD);
			}

		SInt32		mFD;
		void*		mBytePtr;
		UInt64		mByteCount;
		OV<SError>	mError;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMappedFileDataSource

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CMappedFileDataSource::CMappedFileDataSource(const CFile& file, UInt64 byteOffset, UInt64 byteCount) :
		CRandomAccessDataSource()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(file, byteOffset, byteCount);
}

//----------------------------------------------------------------------------------------------------------------------
CMappedFileDataSource::CMappedFileDataSource(const CFile& file) : CRandomAccessDataSource()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(file, 0, file.getByteCount());
}

//----------------------------------------------------------------------------------------------------------------------
CMappedFileDataSource::~CMappedFileDataSource()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CRandomAccessDataSource methods

//----------------------------------------------------------------------------------------------------------------------
UInt64 CMappedFileDataSource::getByteCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mByteCount;
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CMappedFileDataSource::readData(UInt64 position, void* buffer, CData::ByteCount byteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check for error
	if (mInternals->mError.hasValue())
		// Error
		return mInternals->mError;

	// Preflight
	AssertFailIf((position + byteCount) > mInternals->mByteCount);
	if ((position + byteCount) > mInternals->mByteCount)
		// Attempting to ready beyond end of data
		return OV<SError>(SError::mEndOfData);

	// Copy bytes
	::memcpy(buffer, (UInt8*) mInternals->mBytePtr + position, byteCount);

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CData> CMappedFileDataSource::readData(UInt64 position, CData::ByteCount byteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check for error
	if (mInternals->mError.hasValue())
		// Error
		return TVResult<CData>(*mInternals->mError);

	// Preflight
	AssertFailIf((position + byteCount) > mInternals->mByteCount);
	if ((position + byteCount) > mInternals->mByteCount)
		// Attempting to ready beyond end of data
		return TVResult<CData>(SError::mEndOfData);

	return TVResult<CData>(CData((UInt8*) mInternals->mBytePtr + position, byteCount, false));
}
