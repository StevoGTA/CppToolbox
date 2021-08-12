//----------------------------------------------------------------------------------------------------------------------
//	CFileDataSource-Windows.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFileDataSource.h"

#include "ConcurrencyPrimitives.h"
#include "SError-Windows.h"

#undef THIS

//----------------------------------------------------------------------------------------------------------------------
// MARK: CFileDataSourceInternals

class CFileDataSourceInternals {
	public:
		CFileDataSourceInternals(const CFile& file) :
			mFile(file), mByteCount(file.getSize())
			{
				// Open
				CREATEFILE2_EXTENDED_PARAMETERS	extendedParameters = {0};
				extendedParameters.dwSize = sizeof(CREATEFILE2_EXTENDED_PARAMETERS);
				extendedParameters.dwFileAttributes = FILE_ATTRIBUTE_READONLY;
				extendedParameters.dwFileFlags = FILE_FLAG_RANDOM_ACCESS;
				mFileHandle =
						CreateFile2(mFile.getFilesystemPath().getString().getOSString(), GENERIC_READ, FILE_SHARE_READ,
								OPEN_EXISTING, &extendedParameters);
				if (mFileHandle == NULL) {
					// Unable to open
					mError = OI<SError>(SErrorFromWindowsError(GetLastError()));
					CLogServices::logError(*mError, "opening buffered", __FILE__, __func__, __LINE__);
				}
			}
		~CFileDataSourceInternals()
			{
				CloseHandle(mFileHandle);
			}

		CFile		mFile;
		UInt64		mByteCount;
		CLock		mLock;

		HANDLE		mFileHandle;
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
	mInternals = new CFileDataSourceInternals(file);
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

	// Set position
	LARGE_INTEGER	localPosition;
	localPosition.QuadPart = position;

	DWORD		newPositionLow =
						SetFilePointer(mInternals->mFileHandle, localPosition.LowPart, &localPosition.HighPart,
								FILE_BEGIN);
	OI<SError>	error;
	if (newPositionLow != INVALID_SET_FILE_POINTER) {
		// Read
		BOOL	result = ReadFile(mInternals->mFileHandle, buffer, (DWORD) byteCount, NULL, NULL);
		if (!result) {
			// Error
			error = OI<SError>(SErrorFromWindowsError(GetLastError()));
			CLogServices::logError(*error, "reading data buffered", __FILE__, __func__, __LINE__);
		}
	} else {
		// Error
		error = OI<SError>(SErrorFromWindowsError(GetLastError()));
		CLogServices::logError(*error, "setting position buffered", __FILE__, __func__, __LINE__);
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
				CREATEFILE2_EXTENDED_PARAMETERS	extendedParameters = {0};
				extendedParameters.dwSize = sizeof(CREATEFILE2_EXTENDED_PARAMETERS);
				extendedParameters.dwFileAttributes = FILE_ATTRIBUTE_READONLY;
				extendedParameters.dwFileFlags = FILE_FLAG_RANDOM_ACCESS;
				mFileHandle =
						CreateFile2(mFile.getFilesystemPath().getString().getOSString(), GENERIC_READ, FILE_SHARE_READ,
								OPEN_EXISTING, &extendedParameters);
				if (mFileHandle != NULL) {
					// Create file mapping
					mFileMappingHandle = CreateFileMapping(mFileHandle, NULL, PAGE_READONLY, 0, 0, NULL);
					if (mFileMappingHandle != NULL) {
						// Limit to bytes remaining
						byteCount = std::min<UInt64>(byteCount, mFile.getSize() - byteOffset);

						// Create map
						ULARGE_INTEGER	fileOffset;
						fileOffset.QuadPart = byteOffset;

						mBytePtr =
								MapViewOfFile(mFileMappingHandle, FILE_MAP_READ, fileOffset.HighPart,
										fileOffset.LowPart, byteCount);
						if (mBytePtr != NULL)
							// Success
							mByteCount = byteCount;
						else {
							// Failed
							mByteCount = 0;
							mError = OI<SError>(SErrorFromWindowsError(GetLastError()));
							CLogServices::logError(*mError, "creating file view", __FILE__, __func__, __LINE__);
						}
					} else {
						// Error
						mBytePtr = NULL;
						mByteCount = 0;
						mError = OI<SError>(SErrorFromWindowsError(GetLastError()));
						CLogServices::logError(*mError, "creating file mapping", __FILE__, __func__, __LINE__);
					}
				} else {
					// Unable to open
					mBytePtr = NULL;
					mByteCount = 0;
					mError = OI<SError>(SErrorFromWindowsError(GetLastError()));
					CLogServices::logError(*mError, "opening buffered", __FILE__, __func__, __LINE__);
				}
			}
		~CMappedFileDataSourceInternals()
			{
				// Check if need to unmap
				if (mBytePtr != nil)
					// Clean up
					UnmapViewOfFile(mBytePtr);

				CloseHandle(mFileMappingHandle);
				CloseHandle(mFileHandle);
			}

		CFile		mFile;

		HANDLE		mFileHandle;
		HANDLE		mFileMappingHandle;
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
