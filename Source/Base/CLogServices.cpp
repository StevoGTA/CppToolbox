//----------------------------------------------------------------------------------------------------------------------
//	CLogServices.cpp			©2003 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CLogServices.h"

#include "CFilesystemPath.h"
#include "TimeAndDate.h"

#if defined(TARGET_OS_WINDOWS)
	#undef Delete
	#include <Windows.h>
	#define Delete(x)		{ delete x; x = nil; }
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

static	bool					sConsoleLoggingEnabled = false;
static	TNArray<SLogProcInfo>*	sLogMessageProcInfos = nil;
static	TNArray<SLogProcInfo>*	sLogWarningProcInfos = nil;
static	TNArray<SLogProcInfo>*	sLogErrorProcInfos = nil;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc declarations

static	CString	sStringWithDate(const CString& string);
static	void	sLogToConsoleOutput(const CString& string);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CLogServices

//----------------------------------------------------------------------------------------------------------------------
void CLogServices::logMessage(const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CString	stringWithDate = sStringWithDate(string);

	// Check if passing to output/console
	if (sConsoleLoggingEnabled)
		// Log to Console
		sLogToConsoleOutput(stringWithDate);

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
	// Setup
	TNArray<CString>	stringsWithDates(strings, (TNArray<CString>::MapProc) sStringWithDate);
	CString				stringsWithDatesString(stringsWithDates, CString::mPlatformDefaultNewline);

	// Check if passing to output/console
	if (sConsoleLoggingEnabled)
		// Log to Console
		sLogToConsoleOutput(stringsWithDatesString);

	// Check if have procs
	if (sLogMessageProcInfos != nil)
		// Call procs
		for (TIteratorD<SLogProcInfo> iterator = sLogMessageProcInfos->getIterator(); iterator.hasValue();
				iterator.advance())
			// Call proc
			iterator.getValue().callProc(stringsWithDatesString);
}

//----------------------------------------------------------------------------------------------------------------------
void CLogServices::logDebugMessage(const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if passing to output/console
	if (sConsoleLoggingEnabled)
		// Log to Console
		sLogToConsoleOutput(sStringWithDate(string));
}

//----------------------------------------------------------------------------------------------------------------------
void CLogServices::logWarning(const CString& warning, const CString& when, const char* file, const char* proc,
		UInt32 line)
//----------------------------------------------------------------------------------------------------------------------
{
	logWarning(*CFilesystemPath(CString(file)).getLastComponent() + CString(OSSTR(" - warning ")) +
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
	CString				dateString = SGregorianDate().getString();

	TNArray<CString>	strings;
	strings += CString::mEmpty;
	strings += dateString + CString(OSSTR(": *** WARNING ***"));
	strings += dateString + CString(OSSTR(": ")) + string;
	strings += CString::mEmpty;

	CString	compositeString(strings, CString::mPlatformDefaultNewline);

	// Check if passing to output/console
	if (sConsoleLoggingEnabled)
		// Log to Console
		sLogToConsoleOutput(compositeString);

	// Check if have procs
	if (sLogWarningProcInfos != nil)
		// Call procs
		for (TIteratorD<SLogProcInfo> iterator = sLogWarningProcInfos->getIterator(); iterator.hasValue();
				iterator.advance())
			// Call proc
			iterator.getValue().callProc(compositeString);
}

//----------------------------------------------------------------------------------------------------------------------
void CLogServices::logError(const CString& error, const CString& when, const char* file, const char* proc, UInt32 line)
//----------------------------------------------------------------------------------------------------------------------
{
	logError(*CFilesystemPath(CString(file)).getLastComponent() + CString(OSSTR(" - error ")) +
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
	CString				dateString = SGregorianDate().getString();

	TNArray<CString>	strings;
	strings += CString::mEmpty;
	strings += dateString + CString(OSSTR(": *** ERROR ***"));
	strings += dateString + CString(OSSTR(": ")) + string;
	strings += CString::mEmpty;

	CString	compositeString(strings, CString::mPlatformDefaultNewline);

	// Check if passing to output/console
	if (sConsoleLoggingEnabled)
		// Log to Console
		sLogToConsoleOutput(compositeString);

	// Check if have procs
	if (sLogErrorProcInfos != nil)
		// Call procs
		for (TIteratorD<SLogProcInfo> iterator = sLogErrorProcInfos->getIterator(); iterator.hasValue();
				iterator.advance())
			// Call proc
			iterator.getValue().callProc(compositeString);
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
void CLogServices::enableConsoleLogging()
//----------------------------------------------------------------------------------------------------------------------
{
	sConsoleLoggingEnabled = true;
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions

//----------------------------------------------------------------------------------------------------------------------
CString	sStringWithDate(const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	return SGregorianDate().getString() + CString(OSSTR(": ")) + string;
}

//----------------------------------------------------------------------------------------------------------------------
void sLogToConsoleOutput(const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	// Output to console
#if defined(TARGET_OS_WINDOWS)
	OutputDebugString((string + CString::mNewline).getOSString());
#else
	fprintf(stdout, "%s\n", *string.getCString());
#endif
}
