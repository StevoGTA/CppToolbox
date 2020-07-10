//----------------------------------------------------------------------------------------------------------------------
//	COpenGLGPU.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CGPU.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPUProcsInfo

typedef	void		(*COpenGLGPUAcquireContextProc)(void* userData);
typedef	bool		(*COpenGLGPUTryAcquireContextProc)(void* userData);
typedef	void		(*COpenGLGPUReleaseContextProc)(void* userData);
typedef S2DSizeU16	(*COpenGLGPUGetSizeProc)(void* userData);
typedef	Float32		(*COpenGLGPUGetScaleProc)(void* userData);
#if TARGET_OS_IOS
typedef	void*		(*COpenGLGPUGetRenderBufferStorageContextProc)(void* userData);
#endif

struct CGPUProcsInfo {
			// Lifecycle methods
			CGPUProcsInfo(COpenGLGPUAcquireContextProc acquireContextProc,
					COpenGLGPUTryAcquireContextProc tryAcquireContextProc,
					COpenGLGPUReleaseContextProc releaseContextProc, COpenGLGPUGetSizeProc getSizeProc,
					COpenGLGPUGetScaleProc getScaleProc,
#if TARGET_OS_IOS
					COpenGLGPUGetRenderBufferStorageContextProc getRenderBufferStorageContextProc,
#endif
					void* userData) :
				mAcquireContextProc(acquireContextProc), mTryAcquireContextProc(tryAcquireContextProc),
						mReleaseContextProc(releaseContextProc), mGetSizeProc(getSizeProc), mGetScaleProc(getScaleProc),
#if TARGET_OS_IOS
						mGetRenderBufferStorageContextProc(getRenderBufferStorageContextProc),
#endif
						mUserData(userData)
				{}
			CGPUProcsInfo(const CGPUProcsInfo& other) :
				mAcquireContextProc(other.mAcquireContextProc), mTryAcquireContextProc(other.mTryAcquireContextProc),
						mReleaseContextProc(other.mReleaseContextProc), mGetSizeProc(other.mGetSizeProc),
						mGetScaleProc(other.mGetScaleProc),
#if TARGET_OS_IOS
						mGetRenderBufferStorageContextProc(other.mGetRenderBufferStorageContextProc),
#endif
						mUserData(other.mUserData)
				{}

				// Instance methods
	void		acquireContext() const
					{ mAcquireContextProc(mUserData); }
	bool		tryAcquireContext() const
					{ return mTryAcquireContextProc(mUserData); }
	void		releaseContext() const
					{ mReleaseContextProc(mUserData); }
	S2DSizeU16	getSize() const
					{ return mGetSizeProc(mUserData); }
	Float32		getScale() const
					{ return mGetScaleProc(mUserData); }
#if TARGET_OS_IOS
	void*		getRenderBufferStorageContext() const
					{ return mGetRenderBufferStorageContextProc(mUserData); }
#endif

	// Properties
	private:
		COpenGLGPUAcquireContextProc				mAcquireContextProc;
		COpenGLGPUTryAcquireContextProc				mTryAcquireContextProc;
		COpenGLGPUReleaseContextProc				mReleaseContextProc;
		COpenGLGPUGetSizeProc						mGetSizeProc;
		COpenGLGPUGetScaleProc						mGetScaleProc;
#if TARGET_OS_IOS
		COpenGLGPUGetRenderBufferStorageContextProc	mGetRenderBufferStorageContextProc;
#endif
		void*										mUserData;
};
