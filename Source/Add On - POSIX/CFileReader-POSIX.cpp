//----------------------------------------------------------------------------------------------------------------------
//	CFileReader-POSIX.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFileReader.h"

#include "SError-POSIX.h"

#include <sys/mman.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: Macros

#define	CFileReaderReportError(error, message)												\
				{																			\
					CLogServices::logError(error, message, __FILE__, __func__, __LINE__);	\
					mInternals->mFile.logAsError(CString::mSpaceX4);						\
				}
#define	CFileReaderReportErrorAndReturnError(error, message)								\
				{																			\
					CLogServices::logError(error, message, __FILE__, __func__, __LINE__);	\
					mInternals->mFile.logAsError(CString::mSpaceX4);						\
																							\
					return error;															\
				}
#define	CFileReaderReportErrorAndReturnValue(error, message, value)							\
				{																			\
					CLogServices::logError(error, message, __FILE__, __func__, __LINE__);	\
					mInternals->mFile.logAsError(CString::mSpaceX4);						\
																							\
					return value;															\
				}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - SFileMemoryMapSetupInfo

struct SFileMemoryMapSetupInfo {
	// Lifecycle methods
	SFileMemoryMapSetupInfo(CFileReaderInternals* fileReaderInternals) :
		mFileReaderInternals(fileReaderInternals), mBytePtr(nil), mByteCount(0)
		{}

	// Properties
	CFileReaderInternals*	mFileReaderInternals;
	void*					mBytePtr;
	UInt64					mByteCount;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFileReaderInternals

class CFileReaderInternals : public TCopyOnWriteReferenceCountable<CFileReaderInternals> {
	public:
					CFileReaderInternals(const CFile& file) :
						TCopyOnWriteReferenceCountable(),
								mFile(file), mFILE(nil), mFD(-1)
						{}
					CFileReaderInternals(const CFileReaderInternals& other) :
						TCopyOnWriteReferenceCountable(),
								mFile(other.mFile), mFILE(nil), mFD(-1)
						{}
					~CFileReaderInternals()
						{ close(); }

		OI<SError>	close()
						{
							if (mFILE != nil)
								::fclose(mFILE);
							if (mFD != -1)
								::close(mFD);

							return OI<SError>();
						}

		CFile	mFile;

		FILE*	mFILE;
		SInt32	mFD;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFileReader

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CFileReader::CFileReader(const CFile& file)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CFileReaderInternals(file);
}

//----------------------------------------------------------------------------------------------------------------------
CFileReader::CFileReader(const CFileReader& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CFileReader::~CFileReader()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
const CFile& CFileReader::getFile() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mFile;
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CFileReader::open(bool buffered)
//----------------------------------------------------------------------------------------------------------------------
{
	// Prepare for write
	mInternals = mInternals->prepareForWrite();

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
							"rb");

			if (mInternals->mFILE != nil)
				// Success
				return OI<SError>();
			else
				// Unable to open
				CFileReaderReportErrorAndReturnError(SErrorFromPOSIXerror(errno), "opening buffered");
		} else
			// Already open, reset to beginning of file
			return setPos(kPositionFromBeginning, 0);
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
							O_RDONLY, 0);
			if (mInternals->mFD != -1)
				// Success
				return OI<SError>();
			else
				// Unable to open
				CFileReaderReportErrorAndReturnError(SErrorFromPOSIXerror(errno), "opening buffered");
		} else
			// Already open, reset to beginning of file
			return setPos(kPositionFromBeginning, 0);
	}
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CFileReader::readData(void* buffer, UInt64 byteCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	// Check open mode
	if (mInternals->mFILE != nil) {
		// Read from FILE
		ssize_t	bytesRead = ::fread(buffer, 1, (size_t) byteCount, mInternals->mFILE);
		if (bytesRead == (ssize_t) byteCount)
			// Success
			return OI<SError>();
		else if (::feof(mInternals->mFILE))
			// EOF
			return OI<SError>(SError::mEndOfData);
		else
			// Unable to read
			CFileReaderReportErrorAndReturnError(CFile::mUnableToReadError, "reading data");
	} else if (mInternals->mFD != -1) {
		// Read from file
		ssize_t bytesRead = ::read(mInternals->mFD, buffer, (size_t) byteCount);
		if (bytesRead != -1)
			// Success
			return OI<SError>();
		else if (bytesRead == 0)
			// EOF
			return OI<SError>(SError::mEndOfData);
		else
			// Unable to read
			CFileReaderReportErrorAndReturnError(SErrorFromPOSIXerror(errno), "reading data");
	} else
		// Not open
		CFileReaderReportErrorAndReturnError(CFile::mNotOpenError, "reading data");
}

//----------------------------------------------------------------------------------------------------------------------
SInt64 CFileReader::getPos() const
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
			// Error
			CFileReaderReportErrorAndReturnValue(SErrorFromPOSIXerror(errno), "getting position", 0);
	} else
		// File not open!
		CFileReaderReportErrorAndReturnValue(CFile::mNotOpenError, "getting position", 0);
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CFileReader::setPos(Position position, SInt64 newPos) const
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
			return OI<SError>();
		else
			// Error
			CFileReaderReportErrorAndReturnError(SErrorFromPOSIXerror(errno), "setting position");
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
			return OI<SError>();
		else
			// Error
			CFileReaderReportErrorAndReturnError(SErrorFromPOSIXerror(errno), "setting position");
	} else
		// File not open!
		CFileReaderReportErrorAndReturnError(CFile::mNotOpenError, "setting position");
}

//----------------------------------------------------------------------------------------------------------------------
CFileMemoryMap CFileReader::getFileMemoryMap(UInt64 byteOffset, UInt64 byteCount, OI<SError>& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SFileMemoryMapSetupInfo	fileMemoryMapSetupInfo(mInternals->addReference());

	// Is the file open?
	if (mInternals->mFD == -1) {
		// File not open!
		outError = OI<SError>(CFile::mNotOpenError);
		CFileReaderReportErrorAndReturnValue(*outError, "mapping data", CFileMemoryMap(fileMemoryMapSetupInfo));
	}

	// Limit to bytes remaining
	byteCount = std::min<UInt64>(byteCount, mInternals->mFile.getSize() - byteOffset);

	// Create map
	void*	bytePtr = ::mmap(nil, (size_t) byteCount, PROT_READ, MAP_FILE | MAP_PRIVATE, mInternals->mFD, byteOffset);

	// Check for failure
	if (bytePtr == (void*) -1) {
		// Failed
		outError = SErrorFromPOSIXerror(errno);
		CFileReaderReportErrorAndReturnValue(*outError, "mapping data", CFileMemoryMap(fileMemoryMapSetupInfo));
	}

	// All good
	fileMemoryMapSetupInfo.mBytePtr = bytePtr;
	fileMemoryMapSetupInfo.mByteCount = byteCount;

	return CFileMemoryMap(fileMemoryMapSetupInfo);
}

//----------------------------------------------------------------------------------------------------------------------
OI<SError> CFileReader::close() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->close();
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFileMemoryMapInternals

class CFileMemoryMapInternals {
	public:
									CFileMemoryMapInternals(SFileMemoryMapSetupInfo& fileMemoryMapSetupInfo) :
										mFileReaderInternals(fileMemoryMapSetupInfo.mFileReaderInternals),
												mReferenceCount(1), mBytePtr(fileMemoryMapSetupInfo.mBytePtr),
												mByteCount(fileMemoryMapSetupInfo.mByteCount)
										{}
									~CFileMemoryMapInternals()
										{
											// Check if need to cleanup
											if (mBytePtr != nil)
												// Clean up
												::munmap(mBytePtr, (size_t) mByteCount);

											// Remove reference from file reader
											mFileReaderInternals->removeReference();
										}

		CFileMemoryMapInternals*	addReference()
										{ mReferenceCount++; return this; }
		void						removeReference()
										{
											// Decrement reference count and check if we are the last one
											if (--mReferenceCount == 0) {
												// We going away
												CFileMemoryMapInternals*	THIS = this;
												Delete(THIS);
											}
										}

		CFileReaderInternals*	mFileReaderInternals;
		UInt32					mReferenceCount;
		void*					mBytePtr;
		UInt64					mByteCount;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFileMemoryMap

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CFileMemoryMap::CFileMemoryMap(SFileMemoryMapSetupInfo& fileMemoryMapSetupInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CFileMemoryMapInternals(fileMemoryMapSetupInfo);
}

//----------------------------------------------------------------------------------------------------------------------
CFileMemoryMap::CFileMemoryMap(const CFileMemoryMap& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CFileMemoryMap::~CFileMemoryMap()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
const void* CFileMemoryMap::getBytePtr() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mBytePtr;
}

//----------------------------------------------------------------------------------------------------------------------
UInt64 CFileMemoryMap::getByteCount() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mByteCount;
}
