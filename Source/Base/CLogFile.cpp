//----------------------------------------------------------------------------------------------------------------------
//	CLogFile.cpp			©2023 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CLogFile.h"

#include "CFileWriter.h"
#include "ConcurrencyPrimitives.h"
#include "CLogServices.h"
#include "CThread.h"
#include "TLockingArray.h"

#if defined(TARGET_OS_WINDOWS)
	#undef Delete
	#include <Windows.h>
	#define Delete(x)		{ delete x; x = nil; }
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CLogFile::Internals

class CLogFile::Internals {
	public:
						Internals(CFile& file) :
							mFileWriter(file),
									mIsActive(true),
									mWriteThread((CThread::ThreadProc) write, this, CString(OSSTR("CLogFile Writer")),
											CThread::kOptionsAutoStart)
							{
								// Setup
								CLogServices::addLogMessageProc((CLogServices::LogProc) logMessage, this);
								CLogServices::addLogWarningProc((CLogServices::LogProc) logWarning, this);
								CLogServices::addLogErrorProc((CLogServices::LogProc) logError, this);

								// Open
								OV<SError>	error = mFileWriter.open();
								if (error.hasValue())
									// Error
									fprintf(stderr, "Unable to open log file at %s\n",
											*file.getFilesystemPath().getString().getCString());
							}
						~Internals()
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
								// Add
								mStrings += string;

								// Signal
								mWriteSemaphore.signal();
							}

		static	void	logMessage(const CString& string, Internals* internals)
							{ internals->queue(string); }
		static	void	logWarning(const CString& string, Internals* internals)
							{ internals->queue(string); }
		static	void	logError(const CString& string, Internals* internals)
							{ internals->queue(string); }
		static	void	write(CThread& thread, Internals* internals)
							{
								// While active
								while (internals->mIsActive || !internals->mStrings.isEmpty()) {
									// Check if have any strings
									if (!internals->mStrings.isEmpty()) {
										// Write any pending strings
										while (!internals->mStrings.isEmpty())
											// Write first
											internals->mFileWriter.write(internals->mStrings.popFirst());

										// Flush
										internals->mFileWriter.flush();
									} else
										// Wait
										internals->mWriteSemaphore.waitFor();
								}
							}

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
CLogFile::CLogFile(const CFilesystemPath& filesystemPath) : CFile(filesystemPath)
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if exists
	if (doesExist())
		// Remove
		remove();

	// Setup Internals
	mInternals = new Internals(*this);
}

//----------------------------------------------------------------------------------------------------------------------
CLogFile::~CLogFile()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}
