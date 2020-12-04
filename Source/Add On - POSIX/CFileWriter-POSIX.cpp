//----------------------------------------------------------------------------------------------------------------------
//	CFileWriter-POSIX.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFileWriter.h"

#include "SError-POSIX.h"

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

class CFileWriterInternals : public TReferenceCountable<CFileWriterInternals> {
	public:
					CFileWriterInternals(const CFile& file) :
						TReferenceCountable(), mFile(file), mRemoveIfNotClosed(false), mFILE(nil), mFD(-1)
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

		OI<SError>	write(const void* buffer, UInt64 byteCount)
						{
							// Check open mode
							if (mFILE != nil) {
								// Write to FILE
								size_t	bytesWritten = ::fwrite(buffer, 1, (size_t) byteCount, mFILE);

								return (bytesWritten == byteCount) ?
										OI<SError>() : OI<SError>(CFile::mUnableToWriteError);
							} else if (mFD != -1) {
								// Write to file
								ssize_t	bytes = ::write(mFD, buffer, (size_t) byteCount);

								return (bytes != -1) ? OI<SError>() : SErrorFromPOSIXerror(errno);
							} else
								// Not open
								return OI<SError>(CFile::mNotOpenError);
						}
		OI<SError>	close()
						{
							if (mFILE != nil)
								::fclose(mFILE);
							if (mFD != -1)
								::close(mFD);

							return OI<SError>();
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
OI<SError> CFileWriter::open(bool append, bool buffered, bool removeIfNotClosed) const
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
					::fopen(*mInternals->mFile.getFilesystemPath().getString().getCString(CString::kEncodingUTF8),
							!append ? "wb" : "ab+");

			if (mInternals->mFILE != nil)
				// Success
				return OI<SError>();
			else
				// Unable to open
				CFileWriterReportErrorAndReturnError(SErrorFromPOSIXerror(errno), "opening buffered");
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
					::open(*mInternals->mFile.getFilesystemPath().getString().getCString(CString::kEncodingUTF8),
							!append ? (O_RDWR | O_CREAT | O_EXCL) : (O_RDWR | O_APPEND | O_EXLOCK), 0);
			if (mInternals->mFD != -1)
				// Success
				return OI<SError>();
			else
				// Unable to open
				CFileWriterReportErrorAndReturnError(SErrorFromPOSIXerror(errno), "opening buffered");
		} else
			// Already open, reset to beginning of file
			return setPos(kFileWriterPositionModeFromBeginning, 0);
	}
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CFileWriter::write(const void* buffer, UInt64 byteCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	OI<SError>	error = mInternals->write(buffer, byteCount);
	if (!error.hasInstance())
		// Success
		return OI<SError>();
	else
		// Error
		CFileWriterReportErrorAndReturnError(*error, "writing");
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
			CFileWriterReportErrorAndReturnValue(SErrorFromPOSIXerror(errno), "getting position", 0);
	} else
		// File not open!
		CFileWriterReportErrorAndReturnValue(CFile::mNotOpenError, "getting position", 0);
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CFileWriter::setPos(EFileWriterPositionMode mode, SInt64 newPos) const
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
			return OI<SError>();
		else
			// Error
			CFileWriterReportErrorAndReturnError(SErrorFromPOSIXerror(errno), "setting position");
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
			return OI<SError>();
		else
			// Error
			CFileWriterReportErrorAndReturnError(SErrorFromPOSIXerror(errno), "setting position");
	} else
		// File not open!
		CFileWriterReportErrorAndReturnError(CFile::mNotOpenError, "setting position");
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CFileWriter::setSize(UInt64 newSize) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check open mode
	if (mInternals->mFILE != nil) {
		// FILE
		if (::ftruncate(::fileno(mInternals->mFILE), newSize) == 0)
			// Success
			return OI<SError>();
		else
			// Error
			CFileWriterReportErrorAndReturnError(SErrorFromPOSIXerror(errno), "setting size");
	} else if (mInternals->mFD != -1) {
		// file
		if (::ftruncate(mInternals->mFD, newSize) == 0)
			// Success
			return OI<SError>();
		else
			// Error
			CFileWriterReportErrorAndReturnError(SErrorFromPOSIXerror(errno), "setting size");
	} else
		// File not open!
		CFileWriterReportErrorAndReturnError(CFile::mNotOpenError, "setting size");
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CFileWriter::flush() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check open mode
	if (mInternals->mFILE != nil) {
		// FILE
		if (::fflush(mInternals->mFILE) == 0)
			// Success
			return OI<SError>();
		else
			// Error
			CFileWriterReportErrorAndReturnError(SErrorFromPOSIXerror(errno), "flushing");
	} else if (mInternals->mFD != -1) {
		// file
		if (::fsync(mInternals->mFD) == 0)
			// Success
			return OI<SError>();
		else
			// Error
			CFileWriterReportErrorAndReturnError(SErrorFromPOSIXerror(errno), "flushing");
	} else
		// File not open!
		CFileWriterReportErrorAndReturnError(CFile::mNotOpenError, "flushing");
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CFileWriter::close() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->close();
}
