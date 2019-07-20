//----------------------------------------------------------------------------------------------------------------------
//	CFileWriterPOSIXImplementation.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFileWriter.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Macros

#define	CFileWriterReportErrorAndReturnError(error, message)								\
				{																			\
					CLogServices::logError(error, message, __FILE__, __func__, __LINE__);	\
					mInternals->mFile.logAsError(CString::mSpaceX4);						\
																							\
					return error;															\
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
// MARK: - CFileWriterInternals

class CFileWriterInternals {
	public:
								CFileWriterInternals(const CFile& file) :
									mFile(file), mRemoveIfNotClosed(false), mFILE(nil), mFD(-1)
									{}
								~CFileWriterInternals()
									{
										// Check if need to remove
										bool	needToRemove = mRemoveIfNotClosed && ((mFILE != nil) || (mFD != -1));

										// Close
										close();

										// Check if need to remove
										if (needToRemove)
											// Remove
											mFile.remove();
									}

		CFileWriterInternals*	addReference()
									{ mReferenceCount++; return this; }
		void					removeReference()
									{
										// Remove reference and see if we are the last one
										if (--mReferenceCount == 0) {
											// Last one
											CFileWriterInternals*	THIS = this;
											DisposeOf(THIS);
										}
									}

		UError					write(const void* buffer, UInt64 byteCount)
									{
										// Check open mode
										if (mFILE != nil) {
											// Write to FILE
											size_t	bytesWritten = ::fwrite(buffer, 1, (size_t) byteCount, mFILE);
											if (bytesWritten == byteCount)
												// Success
												return kNoError;
											else
												// Unable to write
												return kFileUnableToWriteError;
										} else if (mFD != -1) {
											// Write to file
											ssize_t	bytes = ::write(mFD, buffer, (size_t) byteCount);
											if (bytes != -1)
												// Success
												return kNoError;
											else
												// Unable to write
												return MAKE_UError(kPOSIXErrorDomain, errno);
										} else
											// Not open
											return kFileNotOpenError;
									}
		UError					close()
									{
										if (mFILE != nil)
											::fclose(mFILE);
										if (mFD != -1)
											::close(mFD);

										return kNoError;
									}

		CFile	mFile;
		UInt32	mReferenceCount;

		bool	mRemoveIfNotClosed;
		FILE*	mFILE;
		SInt32	mFD;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFileWriter

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CFileWriter::CFileWriter(const CFile& file)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CFileWriterInternals(file);
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
UError CFileWriter::open(bool append, bool buffered, bool removeIfNotClosed) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Store
	mInternals->mRemoveIfNotClosed = removeIfNotClosed;

	// Check buffered
	if (buffered) {
		// Buffered, check if open
		if (mInternals->mFD != -1) {
			// Close
			::close(mInternals->mFD);
			mInternals->mFD = -1;
		}

		if (mInternals->mFILE == nil) {
			// Open
			mInternals->mFILE =
					::fopen(mInternals->mFile.getFilesystemPath().getString().getCString(kStringEncodingUTF8),
							!append ? "wb" : "ab+");

			if (mInternals->mFILE != nil)
				// Success
				return kNoError;
			else
				// Unable to open
				CFileWriterReportErrorAndReturnError(MAKE_UError(kPOSIXErrorDomain, errno), "opening buffered");
		} else
			// Already open, reset to beginning of file
			return setPos(kFileWriterPositionModeFromBeginning, 0);
	} else {
		// Not buffered, check if open
		if (mInternals->mFILE != nil) {
			// Close
			::fclose(mInternals->mFILE);
			mInternals->mFILE = nil;
		}

		if (mInternals->mFD == -1) {
			// Open
			mInternals->mFD =
					::open(mInternals->mFile.getFilesystemPath().getString().getCString(kStringEncodingUTF8),
							!append ? (O_RDWR | O_CREAT | O_EXCL) : (O_RDWR | O_APPEND | O_EXLOCK), 0);
			if (mInternals->mFD != -1)
				// Success
				return kNoError;
			else
				// Unable to open
				CFileWriterReportErrorAndReturnError(MAKE_UError(kPOSIXErrorDomain, errno), "opening buffered");
		} else
			// Already open, reset to beginning of file
			return setPos(kFileWriterPositionModeFromBeginning, 0);
	}
}

//----------------------------------------------------------------------------------------------------------------------
UError CFileWriter::write(const void* buffer, UInt64 byteCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	UError	error = mInternals->write(buffer, byteCount);
	if (error == kNoError)
		// Success
		return kNoError;
	else
		// Error
		CFileWriterReportErrorAndReturnError(error, "writing");
}

//----------------------------------------------------------------------------------------------------------------------
SInt64 CFileWriter::getPos() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check open mode
	if (mInternals->mFILE != nil)
		// FILE
		return ::ftello(mInternals->mFILE);
	else if (mInternals->mFD != -1) {
		// file
		SInt64	filePos = ::lseek(mInternals->mFD, 0, SEEK_CUR);
		if (filePos != -1)
			// Success
			return filePos;
		else
			// Unable to get position
			CFileWriterReportErrorAndReturnValue(MAKE_UError(kPOSIXErrorDomain, errno), "getting position", 0);
	} else
		// File not open!
		CFileWriterReportErrorAndReturnValue(kFileNotOpenError, "getting position", 0);
}

//----------------------------------------------------------------------------------------------------------------------
UError CFileWriter::setPos(EFileWriterPositionMode mode, SInt64 newPos) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check open mode
	if (mInternals->mFILE != nil) {
		// FILE
		int	posMode;
		switch (mode) {
			case kFileWriterPositionModeFromBeginning:	posMode = SEEK_SET;	break;
			case kFileWriterPositionModeFromCurrent:	posMode = SEEK_CUR;	break;
			case kFileWriterPositionModeFromEnd:		posMode = SEEK_END;	break;
		}

		// Set position
		off_t	offset = ::fseeko(mInternals->mFILE, newPos, posMode);
		if (offset != -1)
			// Success
			return offset;
		else
			// Error
			CFileWriterReportErrorAndReturnError(MAKE_UError(kPOSIXErrorDomain, errno), "setting position");
	} else if (mInternals->mFD != -1) {
		// file
		SInt32	posMode;
		switch (mode) {
			case kFileWriterPositionModeFromBeginning:	posMode = SEEK_SET;	break;
			case kFileWriterPositionModeFromCurrent:	posMode = SEEK_CUR;	break;
			case kFileWriterPositionModeFromEnd:		posMode = SEEK_END;	break;
		}

		// Set position
		off_t	offset = ::lseek(mInternals->mFD, newPos, posMode);
		if (offset != -1)
			// Success
			return kNoError;
		else
			// Error
			CFileWriterReportErrorAndReturnError(MAKE_UError(kPOSIXErrorDomain, errno), "setting position");
	} else
		// File not open!
		CFileWriterReportErrorAndReturnError(kFileNotOpenError, "setting position");
}

//----------------------------------------------------------------------------------------------------------------------
UError CFileWriter::setSize(UInt64 newSize) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check open mode
	if (mInternals->mFILE != nil) {
		// FILE
		if (::ftruncate(::fileno(mInternals->mFILE), newSize) == 0)
			// Success
			return kNoError;
		else
			// Error
			CFileWriterReportErrorAndReturnError(MAKE_UError(kPOSIXErrorDomain, errno), "setting size");
	} else if (mInternals->mFD != -1) {
		// file
		if (::ftruncate(mInternals->mFD, newSize) == 0)
			// Success
			return kNoError;
		else
			// Error
			CFileWriterReportErrorAndReturnError(MAKE_UError(kPOSIXErrorDomain, errno), "setting size");
	} else
		// File not open!
		CFileWriterReportErrorAndReturnError(kFileNotOpenError, "setting size");
}

//----------------------------------------------------------------------------------------------------------------------
UError CFileWriter::close() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->close();
}
