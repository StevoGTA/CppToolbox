//----------------------------------------------------------------------------------------------------------------------
//	CThread-POSIX.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CThread.h"

#include "CLogServices.h"
#include "CppToolboxAssert.h"
#include "SError-POSIX.h"

#include <pthread.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CThread::Internals

class CThread::Internals {
	public:
						Internals(CThread& thread, CThread::ThreadProc threadProc, void* userData,
								const CString& name) :
							mIsRunning(true), mThreadProc(threadProc), mThreadProcUserData(userData), mThreadName(name),
									mThread(thread),
									mPThread(nil)
							{}

		static	void*	threadProc(Internals* internals)
							{
								// Check if have name
								if (!internals->mThreadName.isEmpty())
									// Set name
									::pthread_setname_np(*internals->mThreadName.getUTF8String());

								// Call proc
								internals->mThreadProc(internals->mThread, internals->mThreadProcUserData);

								// Not running
								internals->mIsRunning = false;

								return nil;
							}

		bool				mIsRunning;
		CThread::ThreadProc	mThreadProc;
		void*				mThreadProcUserData;
		CString				mThreadName;
		CThread&			mThread;

		pthread_t			mPThread;
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
	mInternals = new Internals(*this, CThread::runThreadProc, nil, name);

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
	return mInternals->mPThread;
}

//----------------------------------------------------------------------------------------------------------------------
void CThread::start()
//----------------------------------------------------------------------------------------------------------------------
{
	// Create thread attributes
	int				result;
	pthread_attr_t	attr;
	result = ::pthread_attr_init(&attr);
	if (result != 0)
		LogError(SErrorFromPOSIXerror(result), CString(OSSTR("initing pthread attrs")));
	AssertFailIf(result != 0);

	result = ::pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if (result != 0) {
		LogError(SErrorFromPOSIXerror(result), CString(OSSTR("setting pthread detached state")));
		::pthread_attr_destroy(&attr);
	}

	// Create thread
	result = ::pthread_create(&mInternals->mPThread, &attr, (void* (*)(void*)) Internals::threadProc, mInternals);
	::pthread_attr_destroy(&attr);
	if (result != 0)
		LogError(SErrorFromPOSIXerror(result), CString(OSSTR("creating pthread")));
}

//----------------------------------------------------------------------------------------------------------------------
bool CThread::isRunning() const
//----------------------------------------------------------------------------------------------------------------------
{
	return mInternals->mIsRunning;
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
CThread::Ref CThread::getCurrentRef()
//----------------------------------------------------------------------------------------------------------------------
{
	return pthread_self();
}

//----------------------------------------------------------------------------------------------------------------------
void CThread::sleepFor(UniversalTimeInterval universalTimeInterval)
//----------------------------------------------------------------------------------------------------------------------
{
	::usleep((useconds_t) (universalTimeInterval * 1000000.0));
}
