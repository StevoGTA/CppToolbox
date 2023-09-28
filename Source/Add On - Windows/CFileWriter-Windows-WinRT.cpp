//----------------------------------------------------------------------------------------------------------------------
//	CFileWriter-Windows-WinRT.cpp			©2021 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFileWriter.h"

#include "CReferenceCountable.h"
#undef Delete
#include <Unknwn.h>

#include "SError-Windows-WinRT.h"

#include <winrt/Windows.Foundation.h>
#include <winrt/Windows.Storage.h>
#include <winrt/Windows.Storage.Streams.h>

using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::Storage;
using namespace winrt::Windows::Storage::Streams;

//----------------------------------------------------------------------------------------------------------------------
// MARK: Macros

#define	CFileWriterReportErrorAndReturnError(error, message)								\
				{																			\
					CLogServices::logError(error, message, __FILE__, __func__, __LINE__);	\
					mInternals->mFile.logAsError(CString::mSpaceX4);						\
																							\
					return OV<SError>(error);												\
				}
#define	CFileWriterReportErrorAndReturnValue(error, message, value)							\
				{																			\
					CLogServices::logError(error, message, __FILE__, __func__, __LINE__);	\
					mInternals->mFile.logAsError(CString::mSpaceX4);						\
																							\
					return value;															\
				}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFileWriter::Internals

class CFileWriter::Internals : public TReferenceCountableAutoDelete<Internals> {
	public:
		Internals(const CFile& file) :
			TReferenceCountableAutoDelete(),
					mFile(file), mRemoveIfNotClosed(false)
			{}
		~Internals()
			{
				// Check if have Random Access Stream
				if (mRandomAccessStream.CanWrite()) {
					// Close
					mRandomAccessStream.Close();

					// Check if need to remove
					if (mRemoveIfNotClosed)
						// Remove
						mFile.remove();
				}
			}

		CFile				mFile;

		bool				mRemoveIfNotClosed;
		IRandomAccessStream	mRandomAccessStream;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFileWriter

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CFileWriter::CFileWriter(const CFile& file)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new Internals(file);
}

//----------------------------------------------------------------------------------------------------------------------
CFileWriter::CFileWriter(const CFileWriter& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CFileWriter::~CFileWriter()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
const CFile& CFileWriter::getFile() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mFile;
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFileWriter::open(bool append, bool buffered, bool removeIfNotClosed) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mRemoveIfNotClosed = removeIfNotClosed;

	// Check if open
	if (mInternals->mRandomAccessStream.CanWrite()) {
		// Open
		try {
			// Setup
			auto	storageFolder =
							StorageFolder::GetFolderFromPathAsync(
									mInternals->mFile.getFolder().getFilesystemPath().getString().getOSString()).get();
			auto	storageFile =
							storageFolder.CreateFileAsync(mInternals->mFile.getName().getOSString(),
									CreationCollisionOption::ReplaceExisting).get();

			// Create Random Access Stream
			mInternals->mRandomAccessStream =
					storageFile.OpenAsync(FileAccessMode::ReadWrite, StorageOpenOptions::None).get();

			return OV<SError>();
		} catch (const hresult_error& exception) {
			// Error
			SError	error = SErrorFromHRESULTError(exception);
			CLogServices::logError(error, "opening", __FILE__, __func__, __LINE__);

			return OV<SError>(error);
		}
	} else {
		// Already open
		mInternals->mRandomAccessStream.Seek(0);

		return OV<SError>();
	}
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFileWriter::write(const void* buffer, UInt64 byteCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if open
	if (!mInternals->mRandomAccessStream.CanWrite())
		// Not open
		return OV<SError>(CFile::mNotOpenError);

	// Catch errors
	try {
		// Write
		struct __declspec(uuid("905a0fef-bc53-11df-8c49-001e4fc686da")) IBufferByteAccess : ::IUnknown {
			virtual HRESULT __stdcall Buffer(void** value) = 0;
		};

		struct CFileWriterBuffer : implements<CFileWriterBuffer, IBuffer, IBufferByteAccess> {
			// Methods
								CFileWriterBuffer(const void* buffer, UInt64 byteCount) :
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
		mInternals->mRandomAccessStream.WriteAsync(make<CFileWriterBuffer>(buffer, byteCount)).get();

		return OV<SError>();
	} catch (const hresult_error& exception) {
		// Error
		SError	error = SErrorFromHRESULTError(exception);
		CLogServices::logError(error, "writing", __FILE__, __func__, __LINE__);

		return OV<SError>(error);
	}
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CFileWriter::getPos() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mRandomAccessStream.Position();
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFileWriter::setPos(Position position, SInt64 newPos) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if open
	if (!mInternals->mRandomAccessStream.CanWrite())
		// Not open
		return OV<SError>(CFile::mNotOpenError);

	// Check position
	switch (position) {
		case kPositionFromBeginning:
			// From beginning
			mInternals->mRandomAccessStream.Seek(newPos);
			break;

		case kPositionFromCurrent:
			// From current
			mInternals->mRandomAccessStream.Seek(mInternals->mRandomAccessStream.Position() + newPos);
			break;

		case kPositionFromEnd:
			// From end
			mInternals->mRandomAccessStream.Seek(mInternals->mRandomAccessStream.Size() - newPos);
			break;
	}

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFileWriter::setByteCount(UInt64 byteCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if open
	if (!mInternals->mRandomAccessStream.CanWrite())
		// Not open
		return OV<SError>(CFile::mNotOpenError);

	// Set size
	mInternals->mRandomAccessStream.Size(byteCount);

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFileWriter::flush() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if open
	if (!mInternals->mRandomAccessStream.CanWrite())
		// Not open
		return OV<SError>(CFile::mNotOpenError);

	// Flush
	mInternals->mRandomAccessStream.FlushAsync().get();

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFileWriter::close() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if open
	if (!mInternals->mRandomAccessStream.CanWrite())
		// Not open
		return OV<SError>(CFile::mNotOpenError);

	// Close
	mInternals->mRandomAccessStream.Close();
	mInternals->mRandomAccessStream = IRandomAccessStream(nullptr);

	return OV<SError>();
}
