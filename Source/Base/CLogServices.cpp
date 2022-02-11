//----------------------------------------------------------------------------------------------------------------------
//	CLogServices.cpp			©2003 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CLogServices.h"

#include "CFileWriter.h"
#include "ConcurrencyPrimitives.h"
#include "CThread.h"
#include "TLockingArray.h"

#if defined(TARGET_OS_WINDOWS)
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
							TReferenceCountable(), mFile(file), mFileWriter(mFile),
									mIsActive(true),
									mWriteThread((CThread::ThreadProc) write, this, CString(OSSTR("CLogFile Writer")),
											CThread::kOptionsAutoStart)
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
						~CLogFileInternals()
							{
								// Stop writer thread
								mIsActive = false;

								// Signal
								mWriteSemaphore.signal();

								// Wait
								mWriteThread.waitUntilFinished();
							}

				void	queue(const CString& string)
							{
								// Add string
								mStrings +=
										SGregorianDate().getString() + CString(OSSTR(": ")) + string +
												CString::mPlatformDefaultNewline;

								// Signal
								mWriteSemaphore.signal();
							}
				void	queue(const TArray<CString>& strings)
							{
								// Prepare strings
								TNArray<CString>	stringsWithDates;
								for (TIteratorD<CString> iterator = strings.getIterator(); iterator.hasValue();
										iterator.advance())
									// Add string with date
									stringsWithDates +=
											SGregorianDate().getString() + CString(OSSTR(": ")) + *iterator +
													CString::mPlatformDefaultNewline;

								// Add
								mStrings += stringsWithDates;

								// Signal
								mWriteSemaphore.signal();
							}

		static	void	write(CThread& thread, CLogFileInternals* logFileInternals)
							{
								// While active
								while (logFileInternals->mIsActive || !logFileInternals->mStrings.isEmpty()) {
									// Check if have any strings
									if (!logFileInternals->mStrings.isEmpty()) {
										// Write any pending strings
										while (!logFileInternals->mStrings.isEmpty())
											// Write first
											logFileInternals->mFileWriter.write(logFileInternals->mStrings.popFirst());

										// Flush
										logFileInternals->mFileWriter.flush();
									} else
										// Wait
										logFileInternals->mWriteSemaphore.waitFor();
								}
							}

		CFile					mFile;
		CFileWriter				mFileWriter;

		bool					mIsActive;
		CThread					mWriteThread;
		TNLockingArray<CString>	mStrings;
		CSemaphore				mWriteSemaphore;
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
	mInternals->queue(string);
}

//----------------------------------------------------------------------------------------------------------------------
void CLogFile::logMessages(const TArray<CString>& strings) const
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->queue(strings);
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
	// Setup
	static	CString	special(OSSTR("*** WARNING ***"));

	// Log
	CString	strings[] = { special, string };
	mInternals->queue(TSArray<CString>(strings, 2));
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
	// Setup
	static	CString	special(OSSTR("*** ERROR ***"));

	// Log
	CString	strings[] = { CString::mPlatformDefaultNewline, special, string, CString::mPlatformDefaultNewline };
	mInternals->queue(TSArray<CString>(strings, 4));
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
	// Check if have primary log file
	if (sPrimaryLogFile.hasInstance())
		// Pass to primary log file
		(*sPrimaryLogFile).logMessage(string);

	// Setup
	CString	stringWithDate = SGregorianDate().getString() + CString(OSSTR(": ")) + string;

	// Check if passing to output/console
#if defined(DEBUG)
	// Pass to output/console
	sLogToConsoleOutput(stringWithDate);
#else
	if (!sPrimaryLogFile.hasInstance())
		// Pass to output/console
		sLogToConsoleOutput(stringWithDate);
#endif

	// Check if have procs
	if (sLogMessageProcInfos != nil)
		// Call procs
		for (TIteratorD<SLogProcInfo> iterator = sLogMessageProcInfos->getIterator(); iterator.hasValue();
				iterator.advance())
			// Call proc
			iterator.getValue().callProc(stringWithDate);
}

//----------------------------------------------------------------------------------------------------------------------
void CLogServices::logMessages(const TArray<CString>& strings)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if have primary log file
	if (sPrimaryLogFile.hasInstance())
		// Pass to primary log file
		(*sPrimaryLogFile).logMessages(strings);

	// Setup
	CString	stringsWithDates;
	for (TIteratorD<CString> iterator = strings.getIterator(); iterator.hasValue();
			iterator.advance(), stringsWithDates += CString::mPlatformDefaultNewline)
		// Update strings with dates
		stringsWithDates += SGregorianDate().getString() + CString(OSSTR(": ")) + *iterator;

	// Check if passing to output/console
#if defined(DEBUG)
	// Pass to output/console
	sLogToConsoleOutput(stringsWithDates);
#else
	if (!sPrimaryLogFile.hasInstance())
		// Pass to output/console
		sLogToConsoleOutput(stringsWithDates);
#endif

	// Check if have procs
	if (sLogMessageProcInfos != nil)
		// Call procs
		for (TIteratorD<SLogProcInfo> iterator = sLogMessageProcInfos->getIterator(); iterator.hasValue();
				iterator.advance())
			// Call proc
			iterator.getValue().callProc(stringsWithDates);
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
#if defined(DEBUG)
	// Pass to output/console
	dateString = SGregorianDate().getString();
	sLogToConsoleOutput(CString::mEmpty);
	sLogToConsoleOutput(*dateString + CString(OSSTR(": *** WARNING ***")));
	sLogToConsoleOutput(*dateString + CString(OSSTR(": ")) + string);
	sLogToConsoleOutput(CString::mEmpty);
#else
	else {
		// Pass to output/console
		dateString = SGregorianDate().getString();
		sLogToConsoleOutput(CString::mEmpty);
		sLogToConsoleOutput(*dateString + CString(OSSTR(": *** WARNING ***")));
		sLogToConsoleOutput(*dateString + CString(OSSTR(": ")) + string);
		sLogToConsoleOutput(CString::mEmpty);
	}
#endif

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
			iterator.getValue().callProc(*dateString + CString(OSSTR(": ")) + string);
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
#if defined(DEBUG)
	// Pass to output/console
	dateString = SGregorianDate().getString();
	sLogToConsoleOutput(CString::mEmpty);
	sLogToConsoleOutput(*dateString + CString(OSSTR(": *** ERROR ***")));
	sLogToConsoleOutput(*dateString + CString(OSSTR(": ")) + string);
	sLogToConsoleOutput(CString::mEmpty);
#else
	else {
		// Pass to output/console
		dateString = SGregorianDate().getString();
		sLogToConsoleOutput(CString::mEmpty);
		sLogToConsoleOutput(*dateString + CString(OSSTR(": *** ERROR ***")));
		sLogToConsoleOutput(*dateString + CString(OSSTR(": ")) + string);
		sLogToConsoleOutput(CString::mEmpty);
	}
#endif

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
			iterator.getValue().callProc(*dateString + CString(OSSTR(": ")) + string);
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
#if defined(TARGET_OS_WINDOWS)
	OutputDebugString((string + CString::mNewline).getOSString());
#else
	fprintf(stdout, "%s\n", *string.getCString());
#endif
}
