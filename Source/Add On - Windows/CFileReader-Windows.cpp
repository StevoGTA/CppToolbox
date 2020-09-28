//----------------------------------------------------------------------------------------------------------------------
//	CFileReader-Windows.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CFileReader.h"

#undef Delete
#include <Windows.h>
#define Delete(x)		{ delete x; x = nil; }
#undef THIS

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
							mFile(file)
					{}
				CFileReaderInternals(const CFileReaderInternals& other) :
					TCopyOnWriteReferenceCountable(),
							mFile(other.mFile)
					{}
				~CFileReaderInternals()
					{
						close();
					}

		UError	close()
					{
						CloseHandle(mFileMappingHandle);
						mFileMappingHandle = NULL;

						CloseHandle(mFileHandle);
						mFileHandle = NULL;

						return kNoError;
					}

		CFile	mFile;
		HANDLE	mFileHandle;
		HANDLE	mFileMappingHandle;
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
	// Copy on change
	mInternals = mInternals->prepareForWrite();

	// Open
	CREATEFILE2_EXTENDED_PARAMETERS	extendedParameters = {0};
	extendedParameters.dwSize = sizeof(CREATEFILE2_EXTENDED_PARAMETERS);
	extendedParameters.dwFileAttributes = FILE_ATTRIBUTE_READONLY;
	extendedParameters.dwFileFlags = FILE_FLAG_RANDOM_ACCESS;
	mInternals->mFileHandle =
			CreateFile2(mInternals->mFile.getFilesystemPath().getString().getOSString(), GENERIC_READ, FILE_SHARE_READ,
					OPEN_EXISTING, &extendedParameters);

	// Handle results
	if (mInternals->mFileHandle != NULL)
		// Success
		return kNoError;
	else
		// Unable to open
		CFileReaderReportErrorAndReturnError(MAKE_UError(kWindowsErrorDomain, GetLastError()), "opening buffered");
}

//----------------------------------------------------------------------------------------------------------------------
UError CFileReader::readData(void* buffer, UInt64 byteCount) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Read
	BOOL	result = ReadFile(mInternals->mFileHandle, buffer, (DWORD) byteCount, NULL, NULL);
	if (!result)
		CFileReaderReportErrorAndReturnError(MAKE_UError(kWindowsErrorDomain, GetLastError()), "reading data");

	return kNoError;
}

//----------------------------------------------------------------------------------------------------------------------
SInt64 CFileReader::getPos() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	LARGE_INTEGER	position = {0};

	// "Set position" to current to query the current position
	position.LowPart = SetFilePointer(mInternals->mFileHandle, position.LowPart, &position.HighPart, FILE_CURRENT);
	if (position.LowPart == INVALID_SET_FILE_POINTER)
		CFileReaderReportErrorAndReturnValue(MAKE_UError(kWindowsErrorDomain, GetLastError()), "getting position", 0);

	return position.QuadPart;
}

//----------------------------------------------------------------------------------------------------------------------
UError CFileReader::setPos(EFileReaderPositionMode mode, SInt64 newPos) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	LARGE_INTEGER	position;
	position.QuadPart = newPos;

	DWORD	moveMethod;
	switch (mode) {
		case kFileReaderPositionModeFromBeginning:	moveMethod = FILE_BEGIN;	break;
		case kFileReaderPositionModeFromCurrent:	moveMethod = FILE_CURRENT;	break;
		case kFileReaderPositionModeFromEnd:		moveMethod = FILE_END;		break;
	}

	// Set position
	DWORD	newPositionLow = SetFilePointer(mInternals->mFileHandle, position.LowPart, &position.HighPart, moveMethod);
	if (newPositionLow == INVALID_SET_FILE_POINTER)
		CFileReaderReportError(MAKE_UError(kWindowsErrorDomain, GetLastError()), "setting position");

	return kNoError;
}

//----------------------------------------------------------------------------------------------------------------------
CFileMemoryMap CFileReader::getFileMemoryMap(UInt64 byteOffset, UInt64 byteCount, UError& outError) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	SFileMemoryMapSetupInfo	fileMemoryMapSetupInfo(mInternals->addReference());

	// Is the file open?
	if (mInternals->mFileHandle == NULL) {
		// File not open!
		outError = kFileNotOpenError;
		CFileReaderReportErrorAndReturnValue(outError, "mapping data", CFileMemoryMap(fileMemoryMapSetupInfo));
	}

	// Check for file mapping handle
	if (mInternals->mFileMappingHandle == NULL) {
		// Create file mapping handle
		mInternals->mFileMappingHandle =
				CreateFileMapping(mInternals->mFileHandle, NULL, PAGE_READONLY, 0, 0, NULL);
		if (mInternals->mFileMappingHandle == NULL) {
			// Error
			outError = MAKE_UError(kWindowsErrorDomain, GetLastError());
			CFileReaderReportErrorAndReturnValue(outError, "creating file mapping",
					CFileMemoryMap(fileMemoryMapSetupInfo));
		}
	}

	// Limit to bytes remaining
	byteCount = std::min<UInt64>(byteCount, mInternals->mFile.getSize() - byteOffset);

	// Create map
	ULARGE_INTEGER	fileOffset;
	fileOffset.QuadPart = byteOffset;

	void*	bytePtr =
					MapViewOfFile(mInternals->mFileMappingHandle, FILE_MAP_READ, fileOffset.HighPart,
							fileOffset.LowPart, byteCount);

	// Check for failure
	if (bytePtr == NULL) {
		// Failed
		outError = MAKE_UError(kWindowsErrorDomain, GetLastError());
		CFileReaderReportErrorAndReturnValue(outError, "creating file view", CFileMemoryMap(fileMemoryMapSetupInfo));
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
											UnmapViewOfFile(mBytePtr);

										// Remove reference from file reader
										mFileReaderInternals->removeReference();
									}

	CFileMemoryMapInternals*	addReference()
									{
										mReferenceCount++; return this;
									}
	void						removeReference()
									{
										// Decrement reference count and check if we are the last one
										if (--mReferenceCount == 0) {
											// We going away
											CFileMemoryMapInternals* THIS = this;
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
