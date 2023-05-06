//----------------------------------------------------------------------------------------------------------------------
//	CFileDataSource-Windows-WinRT.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFileDataSource.h"

#include "ConcurrencyPrimitives.h"

#undef Delete
#include <Unknwn.h>
#define Delete(x)	{ delete x; x = nil; }

#include "SError-Windows.h"
#include "SError-Windows-WinRT.h"

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.FileProperties.h>
#include <winrt/Windows.Storage.Streams.h>

typedef DWORD SHGDNF;
#include "windowsstoragecom.h"

using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::Streams;

//----------------------------------------------------------------------------------------------------------------------
// MARK: CFileDataSource::Internals

class CFileDataSource::Internals {
	public:
		Internals(const CFile& file)
			{
				// Catch errors
				try {
					// Setup
					auto	storageFile =
									StorageFile::GetFileFromPathAsync(
													file.getFilesystemPath().getString().getOSString())
											.get();

					// Open
					mRandomAccessStream = storageFile.OpenReadAsync().get();
					mByteCount = mRandomAccessStream.Size();
				} catch (const hresult_error& exception) {
					// Error
					mError = OV<SError>(SErrorFromHRESULTError(exception));
					CLogServices::logError(*mError, "opening buffered", __FILE__, __func__, __LINE__);
				}
			}

		UInt64								mByteCount;
		CLock								mLock;

		IRandomAccessStreamWithContentType	mRandomAccessStream;
		OV<SError>							mError;
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

	// Catch errors
	OV<SError>	error;
	try {
		// Set position
		mInternals->mRandomAccessStream.Seek(position);

		// Read
		struct __declspec(uuid("905a0fef-bc53-11df-8c49-001e4fc686da")) IBufferByteAccess : ::IUnknown {
			virtual HRESULT __stdcall Buffer(void** value) = 0;
		};

		struct CFileDataSourceBuffer : implements<CFileDataSourceBuffer, IBuffer, IBufferByteAccess> {
								// Methods
								CFileDataSourceBuffer(const void* buffer, UInt64 byteCount) :
									mBuffer(buffer), mByteCount(byteCount)
									{}
			uint32_t			Capacity() const { return (uint32_t) mByteCount; }
			uint32_t			Length() const { return (uint32_t) mByteCount; }
			void				Length(uint32_t length) {}

			HRESULT	__stdcall	Buffer(void** value)
									{ *value = (void*) mBuffer; return S_OK; }

			// Properties
			const	void*	mBuffer;
					UInt64	mByteCount;
		};

		// Read
		auto	result =
						mInternals->mRandomAccessStream.ReadAsync(make<CFileDataSourceBuffer>(buffer, byteCount),
										(uint32_t) byteCount, InputStreamOptions::ReadAhead)
								.get();
	} catch (const hresult_error& exception) {
		// Error
		error = OV<SError>(SErrorFromHRESULTError(exception));
		CLogServices::logError(*error, "reading", __FILE__, __func__, __LINE__);
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
		Internals(const CFile& file, UInt64 byteOffset, UInt64 byteCount) :
			mFile(file)
			{
				// Catch errors
				try {
					// Setup
					auto	storageFile =
									StorageFile::GetFileFromPathAsync(
													file.getFilesystemPath().getString().getOSString())
											.get();
					HRESULT	result =
									storageFile.as<IStorageItemHandleAccess>()->Create(HANDLE_ACCESS_OPTIONS::HAO_READ,
											HANDLE_SHARING_OPTIONS::HSO_SHARE_NONE, HANDLE_OPTIONS::HO_NONE, nullptr,
											&mFileHandle);
					if (mFileHandle != NULL) {
						// Create file mapping
						mFileMappingHandle = ::CreateFileMapping(mFileHandle, NULL, PAGE_READONLY, 0, 0, NULL);
						if (mFileMappingHandle != NULL) {
							// Limit to bytes remaining
							byteCount = std::min<UInt64>(byteCount, mFile.getByteCount() - byteOffset);

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
								mError = OV<SError>(SErrorFromWindowsGetLastError());
								CLogServices::logError(*mError, "creating file view", __FILE__, __func__, __LINE__);
							}
						} else {
							// Error
							mBytePtr = NULL;
							mByteCount = 0;
							mError = OV<SError>(SErrorFromWindowsGetLastError());
							CLogServices::logError(*mError, "creating file mapping", __FILE__, __func__, __LINE__);
						}
					} else {
						// Unable to open
						mBytePtr = NULL;
						mByteCount = 0;
						mError = OV<SError>(SErrorFromHRESULT(result));
						CLogServices::logError(*mError, "getting handle of file", __FILE__, __func__, __LINE__);
					}
				} catch (const hresult_error& exception) {
					// Error
					mError = OV<SError>(SErrorFromHRESULTError(exception));
					CLogServices::logError(*mError, "opening buffered", __FILE__, __func__, __LINE__);
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
