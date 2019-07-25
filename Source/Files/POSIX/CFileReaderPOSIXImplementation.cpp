//----------------------------------------------------------------------------------------------------------------------
//	CFileReaderPOSIXImplementation.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFileReader.h"

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
	SFileMemoryMapSetupInfo(CFileReaderInternals* fileReaderInternals) :
		mFileReaderInternals(fileReaderInternals), mBytePtr(nil), mByteCount(0)
		{}

	CFileReaderInternals*	mFileReaderInternals;
	void*					mBytePtr;
	UInt64					mByteCount;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CFileReaderInternals

class CFileReaderInternals {
	public:
								CFileReaderInternals(const CFile& file) :
									mFile(file), mReferenceCount(1), mFILE(nil), mFD(-1)
									{}
								~CFileReaderInternals()
									{
										close();
									}

		CFileReaderInternals*	addReference()
									{ mReferenceCount++; return this; }
		void					removeReference()
									{
										// Remove reference and see if we are the last one
										if (--mReferenceCount == 0) {
											// Last one
											CFileReaderInternals*	THIS = this;
											DisposeOf(THIS);
										}
									}
		CFileReaderInternals*	prepareForWrite()
									{
										// Check reference count.  If there is more than 1 reference, we
										//	implement a "copy on write".  So we will clone ourselves so we
										//	have a personal buffer that can be changed while leaving the
										//	exiting buffer as-is for the other references.
										if (mReferenceCount > 1) {
											// Multiple references, copy
											CFileReaderInternals*	fileReaderInternals =
																			new CFileReaderInternals(mFile);

											// One less reference
											mReferenceCount--;

											return fileReaderInternals;
										} else
											// Only a single reference
											return this;
									}

		UError					read(void* buffer, UInt64 byteCount)
									{
										// Check open mode
										if (mFILE != nil) {
											// Read from FILE
											ssize_t	bytesRead = ::fread(buffer, 1, (size_t) byteCount, mFILE);
											if (bytesRead == (ssize_t) byteCount)
												// Success
												return kNoError;
											else if (::feof(mFILE))
												// EOF
												return kFileEOFError;
											else
												// Unable to read
												return kFileUnableToReadError;
										} else if (mFD != -1) {
											// Read from file
											ssize_t bytesRead = ::read(mFD, buffer, (size_t) byteCount);
											if (bytesRead != -1)
												// Success
												return kNoError;
											else if (bytesRead == 0)
												// EOF
												return kFileEOFError;
											else
												// Unable to read
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
UError CFileReader::open(bool buffered)
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
					::fopen(mInternals->mFile.getFilesystemPath().getString().getCString(kStringEncodingUTF8), "rb");

			if (mInternals->mFILE != nil)
				// Success
				return kNoError;
			else
				// Unable to open
				CFileReaderReportErrorAndReturnError(MAKE_UError(kPOSIXErrorDomain, errno), "opening buffered");
		} else
			// Already open, reset to beginning of file
			return setPos(kFileReaderPositionModeFromBeginning, 0);
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
							O_RDONLY | O_EXLOCK, 0);
			if (mInternals->mFD != -1)
				// Success
				return kNoError;
			else
				// Unable to open
				CFileReaderReportErrorAndReturnError(MAKE_UError(kPOSIXErrorDomain, errno), "opening buffered");
		} else
			// Already open, reset to beginning of file
			return setPos(kFileReaderPositionModeFromBeginning, 0);
	}
}

//----------------------------------------------------------------------------------------------------------------------
UError CFileReader::readData(void* buffer, UInt64 byteCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	UError	error = mInternals->read(buffer, byteCount);
	if ((error != kNoError) && (error != kFileEOFError))
		CFileReaderReportError(error, "reading data");

	return error;
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
			CFileReaderReportErrorAndReturnValue(MAKE_UError(kPOSIXErrorDomain, errno), "getting position", 0);
	} else
		// File not open!
		CFileReaderReportErrorAndReturnValue(kFileNotOpenError, "getting position", 0);
}

//----------------------------------------------------------------------------------------------------------------------
UError CFileReader::setPos(EFileReaderPositionMode mode, SInt64 newPos) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check open mode
	if (mInternals->mFILE != nil) {
		// FILE
		int	posMode;
		switch (mode) {
			case kFileReaderPositionModeFromBeginning:	posMode = SEEK_SET;	break;
			case kFileReaderPositionModeFromCurrent:	posMode = SEEK_CUR;	break;
			case kFileReaderPositionModeFromEnd:		posMode = SEEK_END;	break;
		}

		// Set position
		off_t	offset = ::fseeko(mInternals->mFILE, newPos, posMode);
		if (offset != -1)
			// Success
			return kNoError;
		else
			// Error
			CFileReaderReportErrorAndReturnError(MAKE_UError(kPOSIXErrorDomain, errno), "setting position");
	} else if (mInternals->mFD != -1) {
		// file
		SInt32	posMode;
		switch (mode) {
			case kFileReaderPositionModeFromBeginning:	posMode = SEEK_SET;	break;
			case kFileReaderPositionModeFromCurrent:	posMode = SEEK_CUR;	break;
			case kFileReaderPositionModeFromEnd:		posMode = SEEK_END;	break;
		}

		// Set position
		off_t	offset = ::lseek(mInternals->mFD, newPos, posMode);
		if (offset != -1)
			// Success
			return kNoError;
		else
			// Error
			CFileReaderReportErrorAndReturnError(MAKE_UError(kPOSIXErrorDomain, errno), "setting position");
	} else
		// File not open!
		CFileReaderReportErrorAndReturnError(kFileNotOpenError, "setting position");
}

//----------------------------------------------------------------------------------------------------------------------
CFileMemoryMap CFileReader::getFileMemoryMap(UInt64 byteOffset, UInt64 byteCount, UError& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SFileMemoryMapSetupInfo	fileMemoryMapSetupInfo(mInternals->addReference());

	// Is the file open?
	if (mInternals->mFD == -1) {
		// File not open!
		outError = kFileNotOpenError;
		CFileReaderReportErrorAndReturnValue(outError, "mapping data", CFileMemoryMap(fileMemoryMapSetupInfo));
	}

	// Limit to bytes remaining
	byteCount = std::min<UInt64>(byteCount, mInternals->mFile.getSize() - byteOffset);
	void*	bytePtr = ::mmap(nil, (size_t) byteCount, PROT_READ, MAP_FILE | MAP_PRIVATE, mInternals->mFD, byteOffset);

	// Check for failure
	if (bytePtr == (void*) -1) {
		// Failed
		outError = MAKE_UError(kPOSIXErrorDomain, errno);
		CFileReaderReportErrorAndReturnValue(outError, "mapping data", CFileMemoryMap(fileMemoryMapSetupInfo));
	}

	// All good
	fileMemoryMapSetupInfo.mBytePtr = bytePtr;
	fileMemoryMapSetupInfo.mByteCount = byteCount;
	outError = kNoError;

	return CFileMemoryMap(fileMemoryMapSetupInfo);
}

//----------------------------------------------------------------------------------------------------------------------
UError CFileReader::close() const
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
												DisposeOf(THIS);
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
