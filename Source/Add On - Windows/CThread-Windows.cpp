//----------------------------------------------------------------------------------------------------------------------
//	CThread-Windows.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CThread.h"

#include "CLogServices.h"
#include "CppToolboxAssert.h"
#include "CppToolboxError.h"

#undef Delete
#include <Windows.h>
#define Delete(x)		{ delete x; x = nil; }

//----------------------------------------------------------------------------------------------------------------------
// MARK: CThreadInternals

class CThreadInternals {
public:
	CThreadInternals(const CThread& thread, CThreadProc proc, void* userData, const CString& name) :
		mThreadProc(proc), mThreadProcUserData(userData), mThreadName(name), mThread(thread),
				mWindowsThreadHandle(NULL)
		{}
	~CThreadInternals() {}

			CThreadProc	mThreadProc;
			void*		mThreadProcUserData;
			CString		mThreadName;
	const	CThread&	mThread;

			HANDLE		mWindowsThreadHandle;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc declarations

static	DWORD sThreadProc(void* userData);

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CThreadInternals

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CThread::CThread(CThreadProc proc, void* userData, const CString& name)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup internals
	mInternals = new CThreadInternals(*this, proc, userData, name);

	// Create thread
	mInternals->mWindowsThreadHandle = CreateThread(NULL, 0, sThreadProc, mInternals, 0, NULL);
}

//----------------------------------------------------------------------------------------------------------------------
CThread::~CThread()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
CThreadRef CThread::getThreadRef() const
{
	return mInternals->mWindowsThreadHandle;
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
////----------------------------------------------------------------------------------------------------------------------
//void CThread::sleepFor(UniversalTimeInterval timeInterval)
////----------------------------------------------------------------------------------------------------------------------
//{
//	::usleep((UInt32) (timeInterval * 1000000.0));
//}

//----------------------------------------------------------------------------------------------------------------------
CThreadRef CThread::getCurrentThreadRef()
//----------------------------------------------------------------------------------------------------------------------
{
	return GetCurrentThread();
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions
//----------------------------------------------------------------------------------------------------------------------
DWORD sThreadProc(void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CThreadInternals* threadInternals = (CThreadInternals*)userData;

	// Call proc
	threadInternals->mThreadProc(threadInternals->mThread, threadInternals->mThreadProcUserData);

	return 0;
}
