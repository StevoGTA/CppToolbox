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
// MARK: CThread::Internals

class CThread::Internals {
	public:
						Internals(CThread& thread, CThread::ThreadProc threadProc, void* userData,
								const CString& name) :
							mIsRunning(true), mThreadProc(threadProc), mThreadProcUserData(userData), mThreadName(name),
									mThread(thread),
									mThreadRef(::GetCurrentThreadId()), mThreadHandle(nullptr)
							{}

		static	DWORD	threadProc(void* userData)
							{
								// Setup
								Internals&	internals = *((Internals*) userData);

								// Call proc
								internals.mThreadProc(internals.mThread, internals.mThreadProcUserData);

								// Not running
								internals.mIsRunning = false;

								return 0;
							}

		bool				mIsRunning;
		CThread::ThreadProc	mThreadProc;
		void*				mThreadProcUserData;
		CString				mThreadName;
		CThread&			mThread;

		Ref					mThreadRef;
		HANDLE				mThreadHandle;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CThread

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CThread::CThread(ThreadProc threadProc, void* userData, const CString& name, Options options)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup internals
	mInternals = new Internals(*this, threadProc, userData, name);

	// Check options
	if (options & kOptionsAutoStart)
		// Start
		start();
}

//----------------------------------------------------------------------------------------------------------------------
CThread::CThread(const CString& name, Options options)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup internals
	mInternals = new Internals(*this, CThread::runThreadProc, NULL, name);

	// Check options
	if (options & kOptionsAutoStart)
		// Start
		start();
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
	return mInternals->mThreadRef;
}

//----------------------------------------------------------------------------------------------------------------------
bool CThread::isRunning() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mIsRunning;
}

//----------------------------------------------------------------------------------------------------------------------
void CThread::start()
//----------------------------------------------------------------------------------------------------------------------
{
	// Create thread
	mInternals->mThreadHandle = ::CreateThread(NULL, 0, Internals::threadProc, mInternals, 0, NULL);
	::SetThreadDescription(mInternals->mThreadHandle, mInternals->mThreadName.getOSString());
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CThread::Ref CThread::getCurrentRef()
//----------------------------------------------------------------------------------------------------------------------
{
	return ::GetCurrentThreadId();
}

//----------------------------------------------------------------------------------------------------------------------
void CThread::sleepFor(UniversalTimeInterval universalTimeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	::Sleep((DWORD) (universalTimeInterval * 1000.0));
}
