//----------------------------------------------------------------------------------------------------------------------
//	CLogServices.cpp			©2003 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CLogServices.h"

#import "CFileWriter.h"
#import "CLock.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: SLogProcInfo

struct SLogProcInfo {
			// Lifecycle methods
			SLogProcInfo(CLogProc logProc, void* userData) : mLogProc(logProc), mUserData(userData) {}
			SLogProcInfo(const SLogProcInfo& other) : mLogProc(other.mLogProc), mUserData(other.mUserData) {}

			// Instance methods
	void	callProc(const CString& string) const
				{ mLogProc(string, mUserData); }

	// Properties
	CLogProc	mLogProc;
	void*		mUserData;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: Local data

static	OO<CLogFile>			sPrimaryLogFile;
static	TNArray<SLogProcInfo>	sLogMessageProcInfos;
static	TNArray<SLogProcInfo>	sLogWarningProcInfos;
static	TNArray<SLogProcInfo>	sLogErrorProcInfos;

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CLogFileInternals

class CLogFileInternals : public TReferenceCountable<CLogFileInternals> {
	public:
		CLogFileInternals(const CFile& file) :
			mFile(file), mFileWriter(mFile)
			{
				// Open
				mFile.remove();
				UError	error = mFileWriter.open();
				if (error != kNoError)
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
	mInternals->mFileWriter.write(SGregorianDate().getString() + CString::mSpaceCharacter + string);
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
	sPrimaryLogFile = OO<CLogFile>(logFile);
}

//----------------------------------------------------------------------------------------------------------------------
OO<CLogFile>& CLogServices::getPrimaryLogFile()
//----------------------------------------------------------------------------------------------------------------------
{
	return sPrimaryLogFile;
}

//----------------------------------------------------------------------------------------------------------------------
void CLogServices::logMessage(const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	OO<CString>	stringWithDate;

	// Check if have primary log file
	if (sPrimaryLogFile.hasObject())
		// Pass to primary log file
		(*sPrimaryLogFile).logMessage(string);
#if defined(DEBUG)
	// Pass to output/console
	fprintf(stdout, "%s: %s\n", *SGregorianDate().getString().getCString(), *string.getCString());
#else
	else {
		// Pass to output/console
		stringWithDate = OO<CString>(SGregorianDate().getString() + CString::mSpaceCharacter + string);
		fprintf(stdout, "%s\n", *(*stringWithDate).getCString());
	}
#endif

	// Call procs
	for (TIteratorD<SLogProcInfo> iterator = sLogMessageProcInfos.getIterator(); iterator.hasValue();
			iterator.advance()) {
		// Check if need to finish setup
		if (!stringWithDate.hasObject())
			// Finish setup
			stringWithDate = OO<CString>(SGregorianDate().getString() + CString::mSpaceCharacter + string);

		// Call proc
		iterator.getValue().callProc(*stringWithDate);
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CLogServices::logDebugMessage(const CString& string)
//----------------------------------------------------------------------------------------------------------------------
{
	if (sPrimaryLogFile.hasObject())
		// Pass to primary log file
		(*sPrimaryLogFile).logMessage(string);

#if defined(DEBUG)
	// Pass to output/console
	fprintf(stdout, "%s: %s\n", *SGregorianDate().getString().getCString(), *string.getCString());
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
	OO<CString>	dateString;

	// Check if have primary log file
	if (sPrimaryLogFile.hasObject())
		// Pass to primary log file
		(*sPrimaryLogFile).logWarning(string);
	else {
		// Pass to output/console
		dateString = SGregorianDate().getString();
		fprintf(stdout, "%s: \n*** WARNING ***\n", *(*dateString).getCString());
		fprintf(stdout, "%s: %s\n", *(*dateString).getCString(), *string.getCString());
	}

	// Call procs
	for (TIteratorD<SLogProcInfo> iterator = sLogWarningProcInfos.getIterator(); iterator.hasValue();
			iterator.advance()) {
		// Check if need to finish setup
		if (!dateString.hasObject())
			// Finish setup
			dateString = SGregorianDate().getString();

		// Call proc
		iterator.getValue().callProc(*dateString + CString::mSpaceCharacter + string);
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
	OO<CString>	dateString;

	// Check if have primary log file
	if (sPrimaryLogFile.hasObject())
		// Pass to primary log file
		(*sPrimaryLogFile).logError(string);
	else {
		// Pass to output/console
		dateString = SGregorianDate().getString();
		fprintf(stderr, "%s: \n*** ERROR ***\n", *(*dateString).getCString());
		fprintf(stderr, "%s: %s\n", *(*dateString).getCString(), *string.getCString());
	}

	// Call procs
	for (TIteratorD<SLogProcInfo> iterator = sLogErrorProcInfos.getIterator(); iterator.hasValue();
			iterator.advance()) {
	   // Check if need to finish setup
	   if (!dateString.hasObject())
		   // Finish setup
		   dateString = SGregorianDate().getString();


		// Call proc
		iterator.getValue().callProc(string);
	}
}

//----------------------------------------------------------------------------------------------------------------------
void CLogServices::addLogMessageProc(CLogProc logProc, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Add
	sLogMessageProcInfos += SLogProcInfo(logProc, userData);
}

//----------------------------------------------------------------------------------------------------------------------
void CLogServices::addLogWarningProc(CLogProc logProc, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Add
	sLogWarningProcInfos += SLogProcInfo(logProc, userData);
}

//----------------------------------------------------------------------------------------------------------------------
void CLogServices::addLogErrorProc(CLogProc logProc, void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Add
	sLogErrorProcInfos += SLogProcInfo(logProc, userData);
}
