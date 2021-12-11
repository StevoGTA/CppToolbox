//----------------------------------------------------------------------------------------------------------------------
//	CFileDataSource-Windows-WinRT.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFileDataSource.h"

#include "ConcurrencyPrimitives.h"
#include "SError-Windows-WinRT.h"

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.FileProperties.h>
#include <winrt/Windows.Storage.Streams.h>

using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::Streams;

//----------------------------------------------------------------------------------------------------------------------
// MARK: CFileDataSourceInternals

class CFileDataSourceInternals {
	public:
		CFileDataSourceInternals(const CFile& file) //:
			//mFile(file), mByteCount(file.getByteCount())
			{
				// Catch errors
				try {
					// Setup
					auto	storageFile =
									StorageFile::GetFileFromPathAsync(
													file.getFilesystemPath().getString().getOSString())
											.get();
					mByteCount = storageFile.GetBasicPropertiesAsync().get().Size();

					//
					mRandomAccessStream = storageFile.OpenReadAsync().get();
					mByteCount = mRandomAccessStream.Size();

				} catch (const hresult_error& exception) {
					// Error
					mError = OI<SError>(SErrorFromHRESULTError(exception));
					CLogServices::logError(*mError, "opening buffered", __FILE__, __func__, __LINE__);
				}
			}
		~CFileDataSourceInternals()
			{
				//::CloseHandle(mFileHandle);
			}

		UInt64								mByteCount;
		CLock								mLock;

		IRandomAccessStreamWithContentType	mRandomAccessStream;
		OI<SError>							mError;
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
UInt64 CFileDataSource::getByteCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mByteCount;
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CFileDataSource::readData(UInt64 position, void* buffer, CData::ByteCount byteCount)
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

	// Catch errors
	OI<SError>	error;
	try {
		// Set position
		mInternals->mRandomAccessStream.Seek(position);

		// Read
		auto	result =
						mInternals->mRandomAccessStream.ReadAsync(Buffer((uint32_t) byteCount), (uint32_t) byteCount,
										InputStreamOptions::ReadAhead)
								.get();
		::memcpy(buffer, result.data(), byteCount);
	} catch (const hresult_error& exception) {
		// Error
		error = OI<SError>(SErrorFromHRESULTError(exception));
		CLogServices::logError(*error, "reading", __FILE__, __func__, __LINE__);
	}

	// Done
	mInternals->mLock.unlock();

	return error;
}

#if 0

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMappedFileDataSourceInternals

class CMappedFileDataSourceInternals {
	public:
		CMappedFileDataSourceInternals(const CFile& file, UInt64 byteOffset, UInt64 byteCount) :
			mFile(file)
			{
				//// Open
				//CREATEFILE2_EXTENDED_PARAMETERS	extendedParameters = {0};
				//extendedParameters.dwSize = sizeof(CREATEFILE2_EXTENDED_PARAMETERS);
				//extendedParameters.dwFileAttributes = FILE_ATTRIBUTE_READONLY;
				//extendedParameters.dwFileFlags = FILE_FLAG_RANDOM_ACCESS;
				//mFileHandle =
				//		::CreateFile2(mFile.getFilesystemPath().getString().getOSString(), GENERIC_READ,
				//				FILE_SHARE_READ, OPEN_EXISTING, &extendedParameters);
				//if (mFileHandle != NULL) {
				//	// Create file mapping
				//	mFileMappingHandle = ::CreateFileMapping(mFileHandle, NULL, PAGE_READONLY, 0, 0, NULL);
				//	if (mFileMappingHandle != NULL) {
				//		// Limit to bytes remaining
				//		byteCount = std::min<UInt64>(byteCount, mFile.getByteCount() - byteOffset);

				//		// Create map
				//		ULARGE_INTEGER	fileOffset;
				//		fileOffset.QuadPart = byteOffset;

				//		mBytePtr =
				//				::MapViewOfFile(mFileMappingHandle, FILE_MAP_READ, fileOffset.HighPart,
				//						fileOffset.LowPart, (SIZE_T) byteCount);
				//		if (mBytePtr != NULL)
				//			// Success
				//			mByteCount = byteCount;
				//		else {
				//			// Failed
				//			mByteCount = 0;
				//			mError = OI<SError>(SErrorFromWindowsGetLastError());
				//			CLogServices::logError(*mError, "creating file view", __FILE__, __func__, __LINE__);
				//		}
				//	} else {
				//		// Error
				//		mBytePtr = NULL;
				//		mByteCount = 0;
				//		mError = OI<SError>(SErrorFromWindowsGetLastError());
				//		CLogServices::logError(*mError, "creating file mapping", __FILE__, __func__, __LINE__);
				//	}
				//} else {
				//	// Unable to open
				//	mBytePtr = NULL;
				//	mByteCount = 0;
				//	mError = OI<SError>(SErrorFromWindowsGetLastError());
				//	CLogServices::logError(*mError, "opening buffered", __FILE__, __func__, __LINE__);
				//}
			}
		~CMappedFileDataSourceInternals()
			{
				//// Check if need to unmap
				//if (mBytePtr != nil)
				//	// Clean up
				//	::UnmapViewOfFile(mBytePtr);

				//::CloseHandle(mFileMappingHandle);
				//::CloseHandle(mFileHandle);
			}

		CFile		mFile;

		//HANDLE		mFileHandle;
		//HANDLE		mFileMappingHandle;
		//void*		mBytePtr;
		UInt64		mByteCount;
		//OI<SError>	mError;
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
	mInternals = new CMappedFileDataSourceInternals(file, 0, file.getByteCount());
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
OI<SError> CMappedFileDataSource::readData(UInt64 position, void* buffer, CData::ByteCount byteCount)
//----------------------------------------------------------------------------------------------------------------------
{
	//// Check for error
	//if (mInternals->mError.hasInstance())
	//	// Error
	//	return mInternals->mError;

	//// Preflight
	//AssertFailIf((position + byteCount) > mInternals->mByteCount);
	//if ((position + byteCount) > mInternals->mByteCount)
	//	// Attempting to ready beyond end of data
	//	return OI<SError>(SError::mEndOfData);

	//// Copy bytes
	//::memcpy(buffer, (UInt8*) mInternals->mBytePtr + position, (SIZE_T) byteCount);

	return OI<SError>();
}

#endif
