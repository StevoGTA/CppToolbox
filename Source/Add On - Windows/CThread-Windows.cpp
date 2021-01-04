//----------------------------------------------------------------------------------------------------------------------
//	CThread-Windows.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CThread.h"

#include "CLogServices.h"
#include "CppToolboxAssert.h"

#undef Delete
#include <Windows.h>
#define Delete(x)	{ delete x; x = nil; }

//----------------------------------------------------------------------------------------------------------------------
// MARK: CThreadInternals

class CThreadInternals {
	public:
						CThreadInternals(CThread& thread, CThread::ThreadProc threadProc, void* userData,
								const CString& name) :
							mIsRunning(true), mThreadProc(threadProc), mThreadProcUserData(userData), mThreadName(name),
									mThread(thread),
									mWindowsThreadHandle(NULL)
							{}

		static	DWORD	threadProc(void* userData)
							{
								// Setup
								CThreadInternals&	threadInternals = *((CThreadInternals*) userData);

								// Call proc
								threadInternals.mThreadProc(threadInternals.mThread,
										threadInternals.mThreadProcUserData);

								// Not running
								threadInternals.mIsRunning = false;

								return 0;
							}

		bool				mIsRunning;
		CThread::ThreadProc	mThreadProc;
		void*				mThreadProcUserData;
		CString				mThreadName;
		CThread&			mThread;

		HANDLE				mWindowsThreadHandle;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CThread

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CThread::CThread(ThreadProc threadProc, void* userData, const CString& name)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup internals
	mInternals = new CThreadInternals(*this, threadProc, userData, name);

	// Start
	start();
}

//----------------------------------------------------------------------------------------------------------------------
CThread::CThread(const CString& name)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup internals
	mInternals = new CThreadInternals(*this, CThread::runThreadProc, nil, name);
}

//----------------------------------------------------------------------------------------------------------------------
CThread::~CThread()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
CThread::Ref CThread::getRef() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mWindowsThreadHandle;
}

//----------------------------------------------------------------------------------------------------------------------
bool CThread::getIsRunning() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mIsRunning;
}

//----------------------------------------------------------------------------------------------------------------------
void CThread::start()
//----------------------------------------------------------------------------------------------------------------------
{
	// Create thread
	mInternals->mWindowsThreadHandle = CreateThread(NULL, 0, CThreadInternals::threadProc, mInternals, 0, NULL);
	SetThreadDescription(mInternals->mWindowsThreadHandle, mInternals->mThreadName.getOSString());
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CThread::Ref CThread::getCurrentRef()
//----------------------------------------------------------------------------------------------------------------------
{
	return GetCurrentThread();
}

//----------------------------------------------------------------------------------------------------------------------
void CThread::sleepFor(UniversalTimeInterval universalTimeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	Sleep((DWORD) (universalTimeInterval * 1000.0));
}
