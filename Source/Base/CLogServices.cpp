//----------------------------------------------------------------------------------------------------------------------
//	CLogServices.cpp			©2003 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CLogServices.h"

#include "CFileWriter.h"
#include "ConcurrencyPrimitives.h"

#if TARGET_OS_WINDOWS
	#undef Delete
	#include <Windows.h>
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: SLogProcInfo

struct SLogProcInfo {
			// Lifecycle methods
			SLogProcInfo(CLogServices::LogProc logProc, void* userData) : mLogProc(logProc), mUserData(userData) {}
			SLogProcInfo(const SLogProcInfo& other) : mLogProc(other.mLogProc), mUserData(other.mUserData) {}

			// Instance methods
	void	callProc(const CString& string) const
				{ mLogProc(string, mUserData); }

	// Properties
	CLogServices::LogProc	mLogProc;
	void*					mUserData;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	OI<CLogFile>			sPrimaryLogFile;
static	TNArray<SLogProcInfo>*	sLogMessageProcInfos = nil;
static	TNArray<SLogProcInfo>*	sLogWarningProcInfos = nil;
static	TNArray<SLogProcInfo>*	sLogErrorProcInfos = nil;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc declarations

static	void	sLogToConsoleOutput(const CString& string);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CLogFileInternals

class CLogFileInternals : public TReferenceCountable<CLogFileInternals> {
	public:
		CLogFileInternals(const CFile& file) :
			TReferenceCountable(), mFile(file), mFileWriter(mFile)
			{
				// Check if exists
				if (mFile.doesExist())
					// Remove
					mFile.remove();

				// Open
				OI<SError>	error = mFileWriter.open();
				if (error.hasInstance())
					// Error
					fprintf(stderr, "Unable to open log file at %s\n",
							*mFile.getFilesystemPath().getString().getCString());
			}

		CFile		mFile;
		CFileWriter	mFileWriter;
		CLock		mLock;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CLogFile

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CLogFile::CLogFile(const CFile& file)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CLogFileInternals(file);
}

//----------------------------------------------------------------------------------------------------------------------
CLogFile::CLogFile(const CLogFile& other)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = other.mInternals->addReference();
}

//----------------------------------------------------------------------------------------------------------------------
CLogFile::~CLogFile()
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->removeReference();
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
const CFile& CLogFile::getFile() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mFile;
}

//----------------------------------------------------------------------------------------------------------------------
void CLogFile::logMessage(const CString& string) const
//----------------------------------------------------------------------------------------------------------------------
{
	// One at a time please
	mInternals->mLock.lock();
	mInternals->mFileWriter.write(
			SGregorianDate().getString() + CString::mSpace + string + CString::mPlatformDefaultNewline);
	mInternals->mFileWriter.flush();
	mInternals->mLock.unlock();
}

//----------------------------------------------------------------------------------------------------------------------
void CLogFile::logWarning(const CString& warning, const CString& when, const char* file, const char* proc, UInt32 line)
		const
//----------------------------------------------------------------------------------------------------------------------
{
	logWarning(CFilesystemPath(CString(file)).getLastComponent() + CString(OSSTR(" - warning ")) +
			CString(OSSTR("\"")) + warning + CString(OSSTR("\"")) +
			(!when.isEmpty() ? CString(OSSTR(" when ")) + when : CString::mEmpty) +
			CString(OSSTR(", in ")) + CString(proc) + CString(OSSTR("()")) +
			((line != 0) ? CString(OSSTR(", line: ")) + CString(line) : CString::mEmpty));
}

//----------------------------------------------------------------------------------------------------------------------
void CLogFile::logWarning(const CString& string) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Log
	logMessage(CString(OSSTR("*** WARNING ***")));
	logMessage(string);
}

//----------------------------------------------------------------------------------------------------------------------
void CLogFile::logError(const CString& error, const CString& when, const char* file, const char* proc, UInt32 line)
		const
//----------------------------------------------------------------------------------------------------------------------
{
	logError(CFilesystemPath(CString(file)).getLastComponent() + CString(OSSTR(" - error ")) +
			CString(OSSTR("\"")) + error + CString(OSSTR("\"")) +
			(!when.isEmpty() ? CString(OSSTR(" when ")) + when : CString::mEmpty) +
			CString(OSSTR(", in ")) + CString(proc) + CString(OSSTR("()")) +
			((line != 0) ? CString(OSSTR(", line: ")) + CString(line) : CString::mEmpty));
}

//----------------------------------------------------------------------------------------------------------------------
void CLogFile::logError(const CString& string) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Log
	logMessage(CString(OSSTR("*** ERROR ***")));
	logMessage(string);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CLogServices

//----------------------------------------------------------------------------------------------------------------------
void CLogServices::setPrimaryLogFile(const CLogFile& logFile)
//----------------------------------------------------------------------------------------------------------------------
{
	sPrimaryLogFile = OI<CLogFile>(logFile);
}

//----------------------------------------------------------------------------------------------------------------------
OI<CLogFile>& CLogServices::getPrimaryLogFile()
//----------------------------------------------------------------------------------------------------------------------
{
	return sPrimaryLogFile;
}

//----------------------------------------------------------------------------------------------------------------------
void CLogServices::logMessage(const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	OI<CString>	stringWithDate;

	// Check if have primary log file
	if (sPrimaryLogFile.hasInstance())
		// Pass to primary log file
		(*sPrimaryLogFile).logMessage(string);
#if defined(DEBUG)
	// Pass to output/console
	sLogToConsoleOutput(SGregorianDate().getString() + CString(OSSTR(": ")) + string);
#else
	else
		// Pass to output/console
		sLogToConsoleOutput(SGregorianDate().getString() + CString(OSSTR(": ")) + string);
#endif

	// Check if have procs
	if (sLogMessageProcInfos != nil)
		// Call procs
		for (TIteratorD<SLogProcInfo> iterator = sLogMessageProcInfos->getIterator(); iterator.hasValue();
				iterator.advance()) {
			// Check if need to finish setup
			if (!stringWithDate.hasInstance())
				// Finish setup
				stringWithDate = OI<CString>(SGregorianDate().getString() + CString::mSpace + string);

			// Call proc
			iterator.getValue().callProc(*stringWithDate);
		}
}

//----------------------------------------------------------------------------------------------------------------------
void CLogServices::logDebugMessage(const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	if (sPrimaryLogFile.hasInstance())
		// Pass to primary log file
		(*sPrimaryLogFile).logMessage(string);

#if defined(DEBUG)
	// Pass to output/console
	sLogToConsoleOutput(SGregorianDate().getString() + CString(OSSTR(": ")) + string);
#endif
}

//----------------------------------------------------------------------------------------------------------------------
void CLogServices::logWarning(const CString& warning, const CString& when, const char* file, const char* proc,
		UInt32 line)
//----------------------------------------------------------------------------------------------------------------------
{
	logWarning(CFilesystemPath(CString(file)).getLastComponent() + CString(OSSTR(" - warning ")) +
			CString(OSSTR("\"")) + warning + CString(OSSTR("\"")) +
			(!when.isEmpty() ? CString(OSSTR(" when ")) + when : CString::mEmpty) +
			CString(OSSTR(", in ")) + CString(proc) + CString(OSSTR("()")) +
			((line != 0) ? CString(OSSTR(", line: ")) + CString(line) : CString::mEmpty));
}

//----------------------------------------------------------------------------------------------------------------------
void CLogServices::logWarning(const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	OI<CString>	dateString;

	// Check if have primary log file
	if (sPrimaryLogFile.hasInstance())
		// Pass to primary log file
		(*sPrimaryLogFile).logWarning(string);
	else {
		// Pass to output/console
		dateString = SGregorianDate().getString();
		sLogToConsoleOutput(CString::mEmpty);
		sLogToConsoleOutput(*dateString + CString(OSSTR(": *** WARNING ***")));
		sLogToConsoleOutput(*dateString + CString(OSSTR(": ")) + string);
		sLogToConsoleOutput(CString::mEmpty);
	}

	// Check if have procs
	if (sLogWarningProcInfos != nil)
		// Call procs
		for (TIteratorD<SLogProcInfo> iterator = sLogWarningProcInfos->getIterator(); iterator.hasValue();
				iterator.advance()) {
			// Check if need to finish setup
			if (!dateString.hasInstance())
				// Finish setup
				dateString = SGregorianDate().getString();

			// Call proc
			iterator.getValue().callProc(*dateString + CString::mSpace + string);
		}
}

//----------------------------------------------------------------------------------------------------------------------
void CLogServices::logError(const CString& error, const CString& when, const char* file, const char* proc, UInt32 line)
//----------------------------------------------------------------------------------------------------------------------
{
	logError(CFilesystemPath(CString(file)).getLastComponent() + CString(OSSTR(" - error ")) +
			CString(OSSTR("\"")) + error + CString(OSSTR("\"")) +
			(!when.isEmpty() ? CString(OSSTR(" when ")) + when : CString::mEmpty) +
			CString(OSSTR(", in ")) + CString(proc) + CString(OSSTR("()")) +
			((line != 0) ? CString(OSSTR(", line: ")) + CString(line) : CString::mEmpty));
}

//----------------------------------------------------------------------------------------------------------------------
void CLogServices::logError(const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	OI<CString>	dateString;

	// Check if have primary log file
	if (sPrimaryLogFile.hasInstance())
		// Pass to primary log file
		(*sPrimaryLogFile).logError(string);
	else {
		// Pass to output/console
		dateString = SGregorianDate().getString();
		sLogToConsoleOutput(CString::mEmpty);
		sLogToConsoleOutput(*dateString + CString(OSSTR(": *** ERROR ***")));
		sLogToConsoleOutput(*dateString + CString(OSSTR(": ")) + string);
		sLogToConsoleOutput(CString::mEmpty);
	}

	// Check if have procs
	if (sLogErrorProcInfos != nil)
		// Call procs
		for (TIteratorD<SLogProcInfo> iterator = sLogErrorProcInfos->getIterator(); iterator.hasValue();
				iterator.advance()) {
		   // Check if need to finish setup
		   if (!dateString.hasInstance())
			   // Finish setup
			   dateString = SGregorianDate().getString();


			// Call proc
			iterator.getValue().callProc(string);
		}
}

//----------------------------------------------------------------------------------------------------------------------
void CLogServices::addLogMessageProc(LogProc logProc, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// See if it has been created
	if (sLogMessageProcInfos == nil)
		// Create
		sLogMessageProcInfos = new TNArray<SLogProcInfo>();

	// Add
	(*sLogMessageProcInfos) += SLogProcInfo(logProc, userData);
}

//----------------------------------------------------------------------------------------------------------------------
void CLogServices::addLogWarningProc(LogProc logProc, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// See if it has been created
	if (sLogWarningProcInfos == nil)
		// Create
		sLogWarningProcInfos = new TNArray<SLogProcInfo>();

	// Add
	(*sLogWarningProcInfos) += SLogProcInfo(logProc, userData);
}

//----------------------------------------------------------------------------------------------------------------------
void CLogServices::addLogErrorProc(LogProc logProc, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// See if it has been created
	if (sLogErrorProcInfos == nil)
		// Create
		sLogErrorProcInfos = new TNArray<SLogProcInfo>();

	// Add
	(*sLogErrorProcInfos) += SLogProcInfo(logProc, userData);
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions

//----------------------------------------------------------------------------------------------------------------------
void sLogToConsoleOutput(const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
#if TARGET_OS_WINDOWS
	OutputDebugString((string + CString::mNewline).getOSString());
#else
	fprintf(stdout, "%s\n", *string.getCString());
#endif
}
