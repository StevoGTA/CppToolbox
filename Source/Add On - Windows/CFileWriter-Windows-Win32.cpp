//----------------------------------------------------------------------------------------------------------------------
//	CFileWriter-Windows-Win32.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFileWriter.h"

#include "CLogServices.h"
#include "CReferenceCountable.h"
#include "SError-Windows.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Macros

#define	CFileWriterReportErrorAndReturnError(error, message, file)													\
				{																									\
					CLogServices::logError(error, message,															\
							CString(__FILE__, sizeof(__FILE__), CString::kEncodingUTF8),							\
							CString(__func__, sizeof(__func__), CString::kEncodingUTF8), __LINE__);					\
					CLogServices::logError(																			\
							CString::mSpaceX4 + CString(OSSTR("File: ")) + file.getFilesystemPath().getString());	\
																													\
					return OV<SError>(error);																					\
				}
#define	CFileWriterReportErrorAndReturnValue(error, message, file, value)											\
				{																									\
					CLogServices::logError(error, message,															\
							CString(__FILE__, sizeof(__FILE__), CString::kEncodingUTF8),							\
							CString(__func__, sizeof(__func__), CString::kEncodingUTF8), __LINE__);					\
					CLogServices::logError(																			\
							CString::mSpaceX4 + CString(OSSTR("File: ")) + file.getFilesystemPath().getString());	\
																													\
					return value;																					\
				}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFileWriter::Internals

class CFileWriter::Internals : public TReferenceCountableAutoDelete<Internals> {
	public:
		Internals(const CFile& file) :
			TReferenceCountableAutoDelete(),
					mFile(file), mRemoveIfNotClosed(false), mFileHandle(INVALID_HANDLE_VALUE)
			{}
		~Internals()
			{
				// Check if have HANDLE
				if (mFileHandle != INVALID_HANDLE_VALUE) {
					// Close
					::CloseHandle(mFileHandle);

					// Check if removing
					if (mRemoveIfNotClosed)
						// Remove
						mFile.remove();
				}
			}

		CFile	mFile;

		bool	mRemoveIfNotClosed;
		HANDLE	mFileHandle;
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
	if (mInternals->mFileHandle == INVALID_HANDLE_VALUE) {
		// Open
		CREATEFILE2_EXTENDED_PARAMETERS	extendedParameters = {0};
		extendedParameters.dwSize = sizeof(CREATEFILE2_EXTENDED_PARAMETERS);
		extendedParameters.dwFileAttributes = buffered ? FILE_ATTRIBUTE_NORMAL : FILE_FLAG_WRITE_THROUGH;
		extendedParameters.dwFileFlags = FILE_FLAG_RANDOM_ACCESS;
		mInternals->mFileHandle =
				::CreateFile2(mInternals->mFile.getFilesystemPath().getString().getOSString(),
						GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ,
						append ? OPEN_ALWAYS : CREATE_NEW, &extendedParameters);
		if (mInternals->mFileHandle == INVALID_HANDLE_VALUE)
			// Unable to open
			CFileWriterReportErrorAndReturnError(SErrorFromWindowsGetLastError(), CString(OSSTR("opening")),
					mInternals->mFile);

		// Check if appending
		if (append) {
			// Skip to the end
			auto	result = ::SetFilePointer(mInternals->mFileHandle, {0}, NULL, FILE_END);
			if (!result)
				// Error
				CFileWriterReportErrorAndReturnError(SErrorFromWindowsGetLastError(),
						CString(OSSTR("setting position")), mInternals->mFile);
		}

		return OV<SError>();
	} else {
		// Already open
		::SetFilePointerEx(mInternals->mFileHandle, {0}, NULL, FILE_BEGIN);

		return OV<SError>();
	}
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CData> CFileWriter::read(CData::ByteCount byteCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CData	data(byteCount);

	// Read
	BOOL	result =
					::ReadFile(mInternals->mFileHandle, *data.getMutableBuffer(byteCount), (DWORD) byteCount, NULL,
							NULL);
	if (!result) {
		// Error
		SError	error = SErrorFromWindowsGetLastError();
		CFileWriterReportErrorAndReturnValue(error, CString(OSSTR("reading")), mInternals->mFile,
				TVResult<CData>(error));
	}

	return TVResult<CData>(data);
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFileWriter::write(const void* buffer, UInt64 byteCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if open
	if (mInternals->mFileHandle == INVALID_HANDLE_VALUE)
		// Not open
		return OV<SError>(CFile::mNotOpenError);

	// Write
	DWORD	bytesWritten;
	auto	result = ::WriteFile(mInternals->mFileHandle, buffer, (DWORD) byteCount, &bytesWritten, NULL);
	if (!result)
		// Error
		CFileWriterReportErrorAndReturnError(SErrorFromWindowsGetLastError(), CString(OSSTR("writing")),
				mInternals->mFile);

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CFileWriter::getPosition() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if open
	if (mInternals->mFileHandle == INVALID_HANDLE_VALUE)
		// Not open
		return 0;

	// Get current position
	LARGE_INTEGER	currentPosition{0};
	::SetFilePointerEx(mInternals->mFileHandle, {0}, &currentPosition, FILE_CURRENT);

	return currentPosition.QuadPart;
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFileWriter::setPosition(Position position, SInt64 newPos) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if open
	if (mInternals->mFileHandle == INVALID_HANDLE_VALUE)
		// Not open
		return OV<SError>(CFile::mNotOpenError);

	// Check position
	LARGE_INTEGER	distanceToMove = {0};
	distanceToMove.QuadPart = newPos;

	BOOL	result = true;
	switch (position) {
		case kPositionFromBeginning:
			// From beginning
			result = ::SetFilePointerEx(mInternals->mFileHandle, distanceToMove, NULL, FILE_BEGIN);
			break;

		case kPositionFromCurrent:
			// From current
			result = ::SetFilePointerEx(mInternals->mFileHandle, distanceToMove, NULL, FILE_CURRENT);
			break;

		case kPositionFromEnd:
			// From end
			result = ::SetFilePointerEx(mInternals->mFileHandle, distanceToMove, NULL, FILE_END);
			break;
	}
	if (!result)
		// Error
		CFileWriterReportErrorAndReturnError(SErrorFromWindowsGetLastError(), CString(OSSTR("setting position")),
				mInternals->mFile);

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFileWriter::setByteCount(UInt64 byteCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if open
	if (mInternals->mFileHandle == INVALID_HANDLE_VALUE)
		// Not open
		return OV<SError>(CFile::mNotOpenError);

	// Set position
	LARGE_INTEGER	position = {0};
	position.QuadPart = byteCount;

	auto	result = ::SetFilePointerEx(mInternals->mFileHandle, position, NULL, FILE_BEGIN);
	if (!result)
		// Error
		CFileWriterReportErrorAndReturnError(SErrorFromWindowsGetLastError(), CString(OSSTR("setting position")),
				mInternals->mFile);

	// Set End of File
	result = ::SetEndOfFile(mInternals->mFileHandle);
	if (!result)
		// Error
		CFileWriterReportErrorAndReturnError(SErrorFromWindowsGetLastError(), CString(OSSTR("setting end of file")),
				mInternals->mFile);

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFileWriter::flush() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if open
	if (mInternals->mFileHandle == INVALID_HANDLE_VALUE)
		// Not open
		return OV<SError>(CFile::mNotOpenError);

	// Flush
	auto	result = ::FlushFileBuffers(mInternals->mFileHandle);
	if (!result)
		// Error
		CFileWriterReportErrorAndReturnError(SErrorFromWindowsGetLastError(), CString(OSSTR("flushing")),
				mInternals->mFile);

	return OV<SError>();
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFileWriter::close() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if open
	if (mInternals->mFileHandle == INVALID_HANDLE_VALUE)
		// Not open
		return OV<SError>(CFile::mNotOpenError);

	// Close
	auto	result = ::CloseHandle(mInternals->mFileHandle);
	mInternals->mFileHandle = INVALID_HANDLE_VALUE;

	if (result)
		// Success
		return OV<SError>();
	else
		// Error
		CFileWriterReportErrorAndReturnError(SErrorFromWindowsGetLastError(), CString(OSSTR("closing")),
				mInternals->mFile);
}
