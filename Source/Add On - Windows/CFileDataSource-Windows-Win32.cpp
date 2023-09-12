//----------------------------------------------------------------------------------------------------------------------
//	CFileDataSource-Windows-Win32.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFileDataSource.h"

#include "ConcurrencyPrimitives.h"
#include "SError-Windows.h"

#undef THIS

//----------------------------------------------------------------------------------------------------------------------
// MARK: CFileDataSource::Internals

class CFileDataSource::Internals {
	public:
		Internals(const CFile& file) :
			mFile(file), mByteCount(file.getByteCount())
			{
				// Open
				CREATEFILE2_EXTENDED_PARAMETERS	extendedParameters = {0};
				extendedParameters.dwSize = sizeof(CREATEFILE2_EXTENDED_PARAMETERS);
				extendedParameters.dwFileAttributes = FILE_ATTRIBUTE_READONLY;
				extendedParameters.dwFileFlags = FILE_FLAG_RANDOM_ACCESS;
				mFileHandle =
						::CreateFile2(file.getFilesystemPath().getString().getOSString(), GENERIC_READ,
								FILE_SHARE_READ, OPEN_EXISTING, &extendedParameters);
				if (mFileHandle == INVALID_HANDLE_VALUE) {
					// Unable to open
					mError.setValue(SErrorFromWindowsGetLastError());
					CLogServices::logError(*mError, "opening buffered", __FILE__, __func__, __LINE__);
					mFile.logAsError(CString::mSpaceX4);
				}
			}
		~Internals()
			{
				::CloseHandle(mFileHandle);
			}

		CFile		mFile;
		UInt64		mByteCount;
		CLock		mLock;

		HANDLE		mFileHandle;
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
	mInternals = new Internals(file);
}

//----------------------------------------------------------------------------------------------------------------------
CFileDataSource::~CFileDataSource()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CSeekableDataSource methods

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

	// Set position
	LARGE_INTEGER	localPosition;
	localPosition.QuadPart = position;

	DWORD		newPositionLow =
						::SetFilePointer(mInternals->mFileHandle, localPosition.LowPart, &localPosition.HighPart,
								FILE_BEGIN);
	if (newPositionLow != INVALID_SET_FILE_POINTER) {
		// Read
		BOOL	result = ::ReadFile(mInternals->mFileHandle, buffer, (DWORD) byteCount, NULL, NULL);
		if (!result) {
			// Error
			mInternals->mError.setValue(SErrorFromWindowsGetLastError());
			CLogServices::logError(*mInternals->mError, "reading data buffered", __FILE__, __func__, __LINE__);
			mInternals->mFile.logAsError(CString::mSpaceX4);
		}
	} else {
		// Error
		mInternals->mError.setValue(SErrorFromWindowsGetLastError());
		CLogServices::logError(*mInternals->mError, "setting position buffered", __FILE__, __func__, __LINE__);
		mInternals->mFile.logAsError(CString::mSpaceX4);
	}

	// Done
	mInternals->mLock.unlock();

	return mInternals->mError;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMappedFileDataSource::Internals

class CMappedFileDataSource::Internals {
	public:
		Internals(const CFile& file, UInt64 byteOffset, UInt64 byteCount) :
			mFile(file)
			{
				// Open
				CREATEFILE2_EXTENDED_PARAMETERS	extendedParameters = {0};
				extendedParameters.dwSize = sizeof(CREATEFILE2_EXTENDED_PARAMETERS);
				extendedParameters.dwFileAttributes = FILE_ATTRIBUTE_READONLY;
				extendedParameters.dwFileFlags = FILE_FLAG_RANDOM_ACCESS;
				mFileHandle =
						::CreateFile2(file.getFilesystemPath().getString().getOSString(), GENERIC_READ, FILE_SHARE_READ,
								OPEN_EXISTING, &extendedParameters);
				if (mFileHandle != INVALID_HANDLE_VALUE) {
					// Create file mapping
					mFileMappingHandle = ::CreateFileMapping(mFileHandle, NULL, PAGE_READONLY, 0, 0, NULL);
					if (mFileMappingHandle != NULL) {
						// Limit to bytes remaining
						byteCount = std::min<UInt64>(byteCount, file.getByteCount() - byteOffset);

						// Create map
						ULARGE_INTEGER	fileOffset;
						fileOffset.QuadPart = byteOffset;

						mBytePtr =
								::MapViewOfFile(mFileMappingHandle, FILE_MAP_READ, fileOffset.HighPart,
										fileOffset.LowPart, (SIZE_T) byteCount);
						if (mBytePtr != NULL)
							// Success
							mByteCount = byteCount;
						else {
							// Failed
							mByteCount = 0;
							mError.setValue(SErrorFromWindowsGetLastError());
							CLogServices::logError(*mError, "creating file view", __FILE__, __func__, __LINE__);
							mFile.logAsError(CString::mSpaceX4);
						}
					} else {
						// Error
						mBytePtr = NULL;
						mByteCount = 0;
						mError.setValue(SErrorFromWindowsGetLastError());
						CLogServices::logError(*mError, "creating file mapping", __FILE__, __func__, __LINE__);
						mFile.logAsError(CString::mSpaceX4);
					}
				} else {
					// Unable to open
					mFileMappingHandle = NULL;
					mBytePtr = NULL;
					mByteCount = 0;
					mError.setValue(SErrorFromWindowsGetLastError());
					CLogServices::logError(*mError, "opening buffered", __FILE__, __func__, __LINE__);
					mFile.logAsError(CString::mSpaceX4);
				}
			}
		~Internals()
			{
				// Check if need to unmap
				if (mBytePtr != nil)
					// Clean up
					::UnmapViewOfFile(mBytePtr);

				::CloseHandle(mFileMappingHandle);
				::CloseHandle(mFileHandle);
			}

		CFile		mFile;
		HANDLE		mFileHandle;
		HANDLE		mFileMappingHandle;
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

// MARK: CSeekableDataSource methods

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
	::memcpy(buffer, (UInt8*) mInternals->mBytePtr + position, (SIZE_T) byteCount);

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
