//----------------------------------------------------------------------------------------------------------------------
//	CThread-POSIX.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CThread.h"

#include "CLogServices.h"
#include "CppToolboxAssert.h"
#include "SError-POSIX.h"

#include <pthread.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CThreadInternals

class CThreadInternals {
	public:
						CThreadInternals(CThread& thread, CThread::ThreadProc threadProc, void* userData,
								const CString& name) :
							mIsRunning(true), mThreadProc(threadProc), mThreadProcUserData(userData), mThreadName(name),
									mThread(thread),
									mPThread(nil)
							{}

		static	void*	threadProc(void* userData)
							{
								// Setup
								CThreadInternals&	threadInternals = *((CThreadInternals*) userData);

								// Check if have name
								if (!threadInternals.mThreadName.isEmpty())
									// Set name
									::pthread_setname_np(*threadInternals.mThreadName.getCString());

								// Call proc
								threadInternals.mThreadProc(threadInternals.mThread,
										threadInternals.mThreadProcUserData);

								// Not running
								threadInternals.mIsRunning = false;

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
// MARK: - CThreadInternals

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
	return mInternals->mPThread;
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
	// Create thread attributes
	int				result;
	pthread_attr_t	attr;
	result = ::pthread_attr_init(&attr);
	if (result != 0)
		LogError(SErrorFromPOSIXerror(result), "initing pthread attrs");
	AssertFailIf(result != 0);

	result = ::pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if (result != 0) {
		LogError(SErrorFromPOSIXerror(result), "setting pthread detached state");
		::pthread_attr_destroy(&attr);
	}

	// Create thread
	result = ::pthread_create(&mInternals->mPThread, &attr, CThreadInternals::threadProc, mInternals);
	::pthread_attr_destroy(&attr);
	if (result != 0)
		LogError(SErrorFromPOSIXerror(result), "creating pthread");
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
