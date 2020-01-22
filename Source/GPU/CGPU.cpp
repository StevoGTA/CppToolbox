//----------------------------------------------------------------------------------------------------------------------
//	CGPU.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CGPU.h"

#include "CThread.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUInternals

class CGPUInternals {
	public:
		CGPUInternals(const CGPUProcsInfo& procsInfo) :
			mProcsInfo(procsInfo), mContextThreadRef(nil), mContextLockCount(0)
			{}
		~CGPUInternals() {}

		const	CGPUProcsInfo	mProcsInfo;
				CThreadRef		mContextThreadRef;
				UInt32			mContextLockCount;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPU

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CGPU::CGPU(const CGPUProcsInfo& procsInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CGPUInternals(procsInfo);
}

//----------------------------------------------------------------------------------------------------------------------
CGPU::~CGPU()
//----------------------------------------------------------------------------------------------------------------------
{
	DisposeOf(mInternals);
}

// MARK: Subclass methods

//----------------------------------------------------------------------------------------------------------------------
void CGPU::acquireContext() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Check if already have the context locked on the current thread
	if (mInternals->mContextThreadRef != CThread::getCurrentThreadRef()) {
		// Do not
		mInternals->mProcsInfo.mAcquireContextProc(mInternals->mProcsInfo.mUserData);
		mInternals->mContextThreadRef = CThread::getCurrentThreadRef();
	}

	// One more
	mInternals->mContextLockCount++;
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::releaseContext() const
//----------------------------------------------------------------------------------------------------------------------
{
	// One less needing the lock
	if (--mInternals->mContextLockCount == 0) {
		// Release!
		mInternals->mContextThreadRef = nil;
		mInternals->mProcsInfo.mReleaseContextProc(mInternals->mProcsInfo.mUserData);
	}
}
