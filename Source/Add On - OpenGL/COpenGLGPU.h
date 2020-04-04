//----------------------------------------------------------------------------------------------------------------------
//	COpenGLGPU.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CGPU.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPUProcsInfo

typedef	void	(*COpenGLGPUAcquireContextProc)(void* userData);
typedef	bool	(*COpenGLGPUTryAcquireContextProc)(void* userData);
typedef	void	(*COpenGLGPUReleaseContextProc)(void* userData);

struct CGPUProcsInfo {
			// Lifecycle methods
			CGPUProcsInfo(COpenGLGPUAcquireContextProc acquireContextProc,
					COpenGLGPUTryAcquireContextProc tryAcquireContextProc,
					COpenGLGPUReleaseContextProc releaseContextProc, void* userData) :
				mAcquireContextProc(acquireContextProc), mTryAcquireContextProc(tryAcquireContextProc),
						mReleaseContextProc(releaseContextProc), mUserData(userData)
				{}
			CGPUProcsInfo(const CGPUProcsInfo& other) :
				mAcquireContextProc(other.mAcquireContextProc), mTryAcquireContextProc(other.mTryAcquireContextProc),
						mReleaseContextProc(other.mReleaseContextProc), mUserData(other.mUserData)
				{}

			// Instance methods
	void	acquireContext() const
				{ mAcquireContextProc(mUserData); }
	bool	tryAcquireContext() const
				{ return mTryAcquireContextProc(mUserData); }
	void	releaseContext() const
				{ mReleaseContextProc(mUserData); }

	// Properties
	private:
		COpenGLGPUAcquireContextProc	mAcquireContextProc;
		COpenGLGPUTryAcquireContextProc	mTryAcquireContextProc;
		COpenGLGPUReleaseContextProc	mReleaseContextProc;
		void*							mUserData;
};

#if TARGET_OS_IOS
//----------------------------------------------------------------------------------------------------------------------
// MARK: SOpenGLESGPUSetupInfo

struct SOpenGLESGPUSetupInfo {
	// Lifecycle methods
	SOpenGLESGPUSetupInfo(Float32 scale, void* renderBufferStorageContext) :
		mScale(scale), mRenderBufferStorageContext(renderBufferStorageContext)
		{}

	// Properties
	Float32	mScale;
	void*	mRenderBufferStorageContext;
};
#endif

#if TARGET_OS_MACOS
//----------------------------------------------------------------------------------------------------------------------
// MARK: SOpenGLGPUSetupInfo

struct SOpenGLGPUSetupInfo {
	// Lifecycle methods
	SOpenGLGPUSetupInfo(Float32 scale) : mScale(scale) {}

	// Properties
	Float32	mScale;
};
#endif
