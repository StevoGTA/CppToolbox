//----------------------------------------------------------------------------------------------------------------------
//	CGPURenderEngine.cpp			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CGPURenderEngine.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPURenderEngineInternals

class CGPURenderEngineInternals {
	public:
		CGPURenderEngineInternals(const CGPURenderEngineProcsInfo& procsInfo) : mProcsInfo(procsInfo) {}
		~CGPURenderEngineInternals() {}

		const	CGPURenderEngineProcsInfo	mProcsInfo;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPURenderEngine

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CGPURenderEngine::CGPURenderEngine(const CGPURenderEngineProcsInfo& procsInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CGPURenderEngineInternals(procsInfo);
}

//----------------------------------------------------------------------------------------------------------------------
CGPURenderEngine::~CGPURenderEngine()
//----------------------------------------------------------------------------------------------------------------------
{
	DisposeOf(mInternals);
}

// MARK: Subclass methods

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderEngine::acquireContext() const
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mProcsInfo.mAcquireContextProc(mInternals->mProcsInfo.mUserData);
}

//----------------------------------------------------------------------------------------------------------------------
void CGPURenderEngine::releaseContext() const
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals->mProcsInfo.mReleaseContextProc(mInternals->mProcsInfo.mUserData);
}
