//----------------------------------------------------------------------------------------------------------------------
//	CThread.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CThread.h"

#include "CLogServices.h"
#include "CppToolboxAssert.h"
#include "CppToolboxError.h"

#include <pthread.h>

//----------------------------------------------------------------------------------------------------------------------
// MARK: CThreadInternals

class CThreadInternals {
	public:
		CThreadInternals(const CThread& thread, CThreadProc proc, void* userData, const CString& name) :
			mThread(thread), mThreadProc(proc), mThreadProcUserData(userData), mThreadName(name)
			{}
		~CThreadInternals() {}

				CThreadProc	mThreadProc;
				void*		mThreadProcUserData;
				CString		mThreadName;
		const	CThread&	mThread;

				pthread_t	mPThread;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc declarations

static	void*	sThreadProc(void* userData);

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

	// Create thread attributes
	int				result;
	pthread_attr_t	attr;
	result = ::pthread_attr_init(&attr);
	if (result != 0) {
		LogIfError(MAKE_UError(kPOSIXErrorDomain, result), "initing pthread attrs");
	}
	AssertFailIf(result != 0);

	result = ::pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	if (result != 0) {
		LogIfError(MAKE_UError(kPOSIXErrorDomain, result), "setting pthread detached state");
		::pthread_attr_destroy(&attr);
	}

	// Create thread
	result = ::pthread_create(&mInternals->mPThread, &attr, sThreadProc, mInternals);
	::pthread_attr_destroy(&attr);
	if (result != 0) {
		LogIfError(MAKE_UError(kPOSIXErrorDomain, result), "creating pthread");
	}
}

//----------------------------------------------------------------------------------------------------------------------
CThread::~CThread()
//----------------------------------------------------------------------------------------------------------------------
{
	DisposeOf(mInternals);
}

// MARK: Instance methods

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
CThreadRef CThread::getThreadRef() const
{
	return mInternals->mPThread;
}

// MARK: Class methods

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
CThreadRef CThread::getCurrentThreadRef()
{
	return pthread_self();
}

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - Local proc definitions
//----------------------------------------------------------------------------------------------------------------------
void* sThreadProc(void* userData)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CThreadInternals*	threadInternals = (CThreadInternals*) userData;

	// Set name
	::pthread_setname_np(threadInternals->mThreadName.getCString());

	// Call proc
	threadInternals->mThreadProc(threadInternals->mThread, threadInternals->mThreadProcUserData);

	return nil;
}
