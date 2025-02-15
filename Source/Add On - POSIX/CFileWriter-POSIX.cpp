//----------------------------------------------------------------------------------------------------------------------
//	CFileWriter-POSIX.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFileWriter.h"

#include "CReferenceCountable.h"
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
// MARK: - CFileWriter::Internals

class CFileWriter::Internals : public TReferenceCountableAutoDelete<Internals> {
	public:
					Internals(const CFile& file) :
						TReferenceCountableAutoDelete(),
								mFile(file), mRemoveIfNotClosed(false), mFILE(nil), mFD(-1)
						{}
					~Internals()
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

		OV<SError>	write(const void* buffer, UInt64 byteCount)
						{
							// Check open mode
							if (mFILE != nil) {
								// Write to FILE
								size_t	bytesWritten = ::fwrite(buffer, 1, (size_t) byteCount, mFILE);

								return (bytesWritten == byteCount) ?
										OV<SError>() : OV<SError>(CFile::mUnableToWriteError);
							} else if (mFD != -1) {
								// Write to file
								ssize_t	bytes = ::write(mFD, buffer, (size_t) byteCount);

								return (bytes != -1) ? OV<SError>() : SErrorFromPOSIXerror(errno);
							} else
								// Not open
								return OV<SError>(CFile::mNotOpenError);
						}
		OV<SError>	close()
						{
							if (mFILE != nil) {
								::fclose(mFILE);
								mFILE = nil;
							}
							if (mFD != -1) {
								::close(mFD);
								mFD = -1;
							}

							return OV<SError>();
						}

		CFile	mFile;

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
					::fopen(*mInternals->mFile.getFilesystemPath().getString().getUTF8String(),
							!append ? "wb+" : "ab+");

			if (mInternals->mFILE != nil)
				// Success
				return OV<SError>();
			else
				// Unable to open
				CFileWriterReportErrorAndReturnError(SErrorFromPOSIXerror(errno), "opening buffered");
		} else
			// Already open, reset to beginning of file
			return setPosition(kPositionFromBeginning, 0);
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
					::open(*mInternals->mFile.getFilesystemPath().getString().getUTF8String(),
							!append ? (O_RDWR | O_CREAT | O_EXCL) : (O_RDWR | O_CREAT | O_APPEND | O_EXLOCK),
							S_IWUSR | S_IRUSR | S_IRGRP | S_IROTH);
			if (mInternals->mFD != -1)
				// Success
				return OV<SError>();
			else
				// Unable to open
				CFileWriterReportErrorAndReturnError(SErrorFromPOSIXerror(errno), "opening non-buffered");
		} else
			// Already open, reset to beginning of file
			return setPosition(kPositionFromBeginning, 0);
	}
}

//----------------------------------------------------------------------------------------------------------------------
TVResult<CData> CFileWriter::read(CData::ByteCount byteCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CData	data(byteCount);
	
	// Check mode
	if (mInternals->mFILE != nil) {
		// FILE
		// Read
		ssize_t	bytesRead = ::fread(data.getMutableBytePtr(), 1, (size_t) byteCount, mInternals->mFILE);
		if (bytesRead != (ssize_t) byteCount) {
			// Error
			SError	error = SErrorFromPOSIXerror(errno);
			CLogServices::logError(error, "reading data buffered", __FILE__, __func__, __LINE__);

			return TVResult<CData>(error);
		}
	} else {
		// file
		// Read
		ssize_t bytesRead = ::read(mInternals->mFD, data.getMutableBytePtr(), (size_t) byteCount);
		if (bytesRead == -1) {
			// Error
			SError	error = SErrorFromPOSIXerror(errno);
			CLogServices::logError(error, "reading data non-buffered", __FILE__, __func__, __LINE__);

			return TVResult<CData>(error);
		}
	}

	return TVResult<CData>(data);
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFileWriter::write(const void* buffer, UInt64 byteCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	OV<SError>	error = mInternals->write(buffer, byteCount);
	if (!error.hasValue())
		// Success
		return OV<SError>();
	else
		// Error
		CFileWriterReportErrorAndReturnError(*error, "writing");
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CFileWriter::getPosition() const
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
OV<SError> CFileWriter::setPosition(Position position, SInt64 newPos) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check open mode
	if (mInternals->mFILE != nil) {
		// FILE
		int	posMode;
		switch (position) {
			case kPositionFromBeginning:	posMode = SEEK_SET;	break;
			case kPositionFromCurrent:		posMode = SEEK_CUR;	break;
			case kPositionFromEnd:			posMode = SEEK_END;	break;
		}

		// Set position
		off_t	offset = ::fseeko(mInternals->mFILE, newPos, posMode);
		if (offset != -1)
			// Success
			return OV<SError>();
		else
			// Error
			CFileWriterReportErrorAndReturnError(SErrorFromPOSIXerror(errno), "setting position");
	} else if (mInternals->mFD != -1) {
		// file
		SInt32	posMode;
		switch (position) {
			case kPositionFromBeginning:	posMode = SEEK_SET;	break;
			case kPositionFromCurrent:		posMode = SEEK_CUR;	break;
			case kPositionFromEnd:			posMode = SEEK_END;	break;
		}

		// Set position
		off_t	offset = ::lseek(mInternals->mFD, newPos, posMode);
		if (offset != -1)
			// Success
			return OV<SError>();
		else
			// Error
			CFileWriterReportErrorAndReturnError(SErrorFromPOSIXerror(errno), "setting position");
	} else
		// File not open!
		CFileWriterReportErrorAndReturnError(CFile::mNotOpenError, "setting position");
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFileWriter::setByteCount(UInt64 byteCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check open mode
	if (mInternals->mFILE != nil) {
		// FILE
		if (::ftruncate(::fileno(mInternals->mFILE), byteCount) == 0)
			// Success
			return OV<SError>();
		else
			// Error
			CFileWriterReportErrorAndReturnError(SErrorFromPOSIXerror(errno), "setting size");
	} else if (mInternals->mFD != -1) {
		// file
		if (::ftruncate(mInternals->mFD, byteCount) == 0)
			// Success
			return OV<SError>();
		else
			// Error
			CFileWriterReportErrorAndReturnError(SErrorFromPOSIXerror(errno), "setting size");
	} else
		// File not open!
		CFileWriterReportErrorAndReturnError(CFile::mNotOpenError, "setting size");
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFileWriter::flush() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check open mode
	if (mInternals->mFILE != nil) {
		// FILE
		if (::fflush(mInternals->mFILE) == 0)
			// Success
			return OV<SError>();
		else
			// Error
			CFileWriterReportErrorAndReturnError(SErrorFromPOSIXerror(errno), "flushing");
	} else if (mInternals->mFD != -1) {
		// file
		if (::fsync(mInternals->mFD) == 0)
			// Success
			return OV<SError>();
		else
			// Error
			CFileWriterReportErrorAndReturnError(SErrorFromPOSIXerror(errno), "flushing");
	} else
		// File not open!
		CFileWriterReportErrorAndReturnError(CFile::mNotOpenError, "flushing");
}

//----------------------------------------------------------------------------------------------------------------------
OV<SError> CFileWriter::close() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->close();
}
