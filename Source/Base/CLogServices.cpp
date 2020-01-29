//----------------------------------------------------------------------------------------------------------------------
//	CLogServices.cpp			©2003 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CLogServices.h"

#if 1

#import "CFilesystemPath.h"

//----------------------------------------------------------------------------------------------------------------------
void CLogServices::logMessage(const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
#if defined(DEBUG)
	fprintf(stdout, "%s\n", string.getCString());
#endif
}

//----------------------------------------------------------------------------------------------------------------------
void CLogServices::logError(const CString& error, const CString& when, const char* file, const char* proc, UInt32 line)
//----------------------------------------------------------------------------------------------------------------------
{
	logError(CFilesystemPath(CString(file)).getLastComponent() + CString(" - error ") +
			CString("\"") + error + CString("\"") +
			(!when.isEmpty() ? CString(" when ") + when : CString::mEmpty) +
			CString(", in ") + CString(proc) + CString("()") +
			((line != 0) ? CString(", line: ") + CString(line) : CString::mEmpty));
}

//----------------------------------------------------------------------------------------------------------------------
void CLogServices::logError(const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
#if defined(DEBUG)
	fprintf(stdout, "\n*** ERROR ***\n%s\n", string.getCString());
#endif
}

#else

#include "CCoreServices.h"
#include "CFileX.h"
#include "CURL.h"

#include <pthread.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	CFileX*		sLogFile = nil;
static	CLogProc	sLogMessageProc = nil;
static	void*		sLogMessageProcUserData = nil;
static	CLogProc	sLogWarningProc = nil;
static	void*		sLogWarningProcUserData = nil;
static	CLogProc	sLogErrorProc = nil;
static	void*		sLogErrorProcUserData = nil;
static	bool		sSilentWarningsAndErrors = false;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc declarations

static	void	sLog(const CString& string);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CLogServices

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
const CFileX& CLogServices::setup(const CString& filename, const CString& productAndVersion, bool startNewFile)
//----------------------------------------------------------------------------------------------------------------------
{
	// Get folder
	CFolderX	folder;

	CFolderX	testFolder = CFolderX::userHomeFolder();
	if (testFolder.doesExist()) {
		// Found user folder (good, means, like, we have a user!)
		// At least put the log here
		folder = testFolder;

		// Check for Library folder
		testFolder = CFolderX(folder, CString("Library"));
		if (!testFolder.doesExist())
			// Not found, create
			testFolder.create();

		// Is it there
		if (testFolder.doesExist()) {
			// Yes, use it
			folder = testFolder;

			// Check for Logs folder
			testFolder = CFolderX(folder, CString("Logs"));
			if (!testFolder.doesExist())
				// Not found, create
				testFolder.create();

			// Is it there?
			if (testFolder.doesExist())
				// Yes, use it
				folder = testFolder;
		}
	}

	// Create file object
	sLogFile = new CFileX(folder, filename + CString(".txt"));

	// Open
	CString	string;
	if (startNewFile) {
		// Reset
		sLogFile->open(kFileOpenModeWriteBuffered);
		sLogFile->close();
	} else
		// Append something to break up the file
		string = CString("\n******************************************************************************\n");

	// Write our first message
	string +=
			CString("Logging started ") +
					CTimeInfo::getStringForUniversalTime(CTimeInfo::getCurrentUniversalTime(),
							kTimeInfoStringStyleLong, kTimeInfoStringStyleLong) +
			CString("\n");
	string += CString("Product: ") + productAndVersion + CString("\n");

	// Log system info
	string += CString("System Info\n");

	// OS Version
	string += CString("\tSystem Version: ") + CCoreServices::getSystemVersion().getString() + CString("\n");

	// Processor Count
	string +=
			CString("\tRunning ") + CString(CCoreServices::getTotalProcessorCoresCount()) +
					CString(" CPUs/Cores - ") + CCoreServices::getProcessorInfo() + CString("\n");

	// Physical RAM Size
	string +=
			CString("\tPhysical RAM: ") +
					CString(CCoreServices::getPhysicalMemoryByteCount(),
							(EStringSpecialFormattingOptions) (kStringSpecialFormattingOptionsBytesBinary |
									kStringSpecialFormattingOptionsBytesBinaryDoEasyRead |
									kStringSpecialFormattingOptionsBytesBinaryDoOrAddExactUseCommas|
									kStringSpecialFormattingOptionsBytesBinaryDoOrAddExactAddLabel)) +
					CString("\n");

	// Core Audio Version
	string += CString("\tCore Audio Version: ") + CCoreServices::getCoreAudioVersion().getString() + CString("\n");

	// Done
	string += CString("\n\n");

	// Write
	CLogServices::logMessage(string);

	return *sLogFile;
}

//----------------------------------------------------------------------------------------------------------------------
const CFileX& CLogServices::getLogFile()
//----------------------------------------------------------------------------------------------------------------------
{
	return *sLogFile;
}

//----------------------------------------------------------------------------------------------------------------------
void CLogServices::logMessage(const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	// Log to log file
	sLog(string);

	// Call proc
	if (sLogMessageProc != nil)
		sLogMessageProc(string + CString("\n"), sSilentWarningsAndErrors, sLogMessageProcUserData);
}

//----------------------------------------------------------------------------------------------------------------------
void CLogServices::logWarning(const CString& warning, const CString& when, const char* file, const char* proc,
		UInt32 line)
//----------------------------------------------------------------------------------------------------------------------
{
	// Compose and log
	logWarning(CURL::getFilesystemLastPathComponent(CString(file)) + CString(" - warning ") +
			CString("\"") + warning + CString("\"") +
			(!when.isEmpty() ? CString(" when ") + when : CString::mEmpty) +
			CString(", in ") + CString(proc) + CString("()") +
			((line != 0) ? CString(", line: ") + CString(line) : CString::mEmpty));
}

//----------------------------------------------------------------------------------------------------------------------
void CLogServices::logWarning(const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	// Log to log file
	sLog(CString("\n*** WARNING ***\n"));
	sLog(string);

	// Call proc
	if (sLogWarningProc != nil)
		sLogWarningProc(string + CString("\n"), sSilentWarningsAndErrors, sLogWarningProcUserData);
}

//----------------------------------------------------------------------------------------------------------------------
void CLogServices::logError(const CString& error, const CString& when, const char* file, const char* proc,
		UInt32 line)
//----------------------------------------------------------------------------------------------------------------------
{
	// Compose and log
	logError(CURL::getFilesystemLastPathComponent(CString(file)) + CString(" - error ") +
			CString("\"") + error + CString("\"") +
			(!when.isEmpty() ? CString(" when ") + when : CString::mEmpty) +
			CString(", in ") + CString(proc) + CString("()") +
			((line != 0) ? CString(", line: ") + CString(line) : CString::mEmpty));
}

//----------------------------------------------------------------------------------------------------------------------
void CLogServices::logError(const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	// Log to log file
	sLog(CString("\n*** ERROR ***\n"));
	sLog(string);

	// Call proc
	if (sLogErrorProc != nil)
		sLogErrorProc(string + CString("\n"), sSilentWarningsAndErrors, sLogErrorProcUserData);
}

//----------------------------------------------------------------------------------------------------------------------
void CLogServices::log(const CFileX& file)
//----------------------------------------------------------------------------------------------------------------------
{
	logMessage(CString("    File: ") + file.getDescription());
}

//----------------------------------------------------------------------------------------------------------------------
void CLogServices::log(const CFolderX& folder)
//----------------------------------------------------------------------------------------------------------------------
{
	logMessage(CString("    Folder: ") + folder.getDescription());
}

//----------------------------------------------------------------------------------------------------------------------
void CLogServices::setLogMessageProc(CLogProc logProc, void* logProcUserData)
//----------------------------------------------------------------------------------------------------------------------
{
	sLogMessageProc = logProc;
	sLogMessageProcUserData = logProcUserData;
}

//----------------------------------------------------------------------------------------------------------------------
void CLogServices::setLogWarningProc(CLogProc logProc, void* logProcUserData)
//----------------------------------------------------------------------------------------------------------------------
{
	sLogWarningProc = logProc;
	sLogWarningProcUserData = logProcUserData;
}

//----------------------------------------------------------------------------------------------------------------------
void CLogServices::setLogErrorProc(CLogProc logProc, void* logProcUserData)
//----------------------------------------------------------------------------------------------------------------------
{
	sLogErrorProc = logProc;
	sLogErrorProcUserData = logProcUserData;
}

//----------------------------------------------------------------------------------------------------------------------
bool CLogServices::getSilentWarningsAndErrors()
//----------------------------------------------------------------------------------------------------------------------
{
	return sSilentWarningsAndErrors;
}

//----------------------------------------------------------------------------------------------------------------------
void CLogServices::setSilentWarningsAndErrors(bool silentWarningsAndErrors)
//----------------------------------------------------------------------------------------------------------------------
{
	sSilentWarningsAndErrors = silentWarningsAndErrors;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions

//----------------------------------------------------------------------------------------------------------------------
void sLog(const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	static	bool			sMutexInitted = false;
	static	pthread_t		sActiveThread = nil;
	static	pthread_mutex_t	sMutex;

	// Send to other receivers
#if defined(DEBUG)
	fprintf(stdout, "%s", (string + CString("\n")).getCString());
#endif

	// Ensure things are set up
	if (sLogFile == nil) {
		// Not setup
		fprintf(stdout, "Log file not setup yet...\n");

		return;
	}

	// Abort logging if re-entrant on same thread
	if (sActiveThread == ::pthread_self())
		return;

	// One at a time please
	if (!sMutexInitted) {
		::pthread_mutex_init(&sMutex, nil);
		sMutexInitted = true;
	}

	::pthread_mutex_lock(&sMutex);
	sActiveThread = ::pthread_self();

	// Log
	if (sLogFile->open(kFileOpenModeAppendBuffered) == kNoError) {
		// Go to end
		if (sLogFile->setPos(kFilePositionModeFromEnd, 0) == kNoError) {
			// Write message
			sLogFile->write(string + CString("\n"));

			// Close
			sLogFile->close();
		}
	}

	// All done
	sActiveThread = nil;
	::pthread_mutex_unlock(&sMutex);
}

#endif
