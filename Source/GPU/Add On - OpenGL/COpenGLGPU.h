//----------------------------------------------------------------------------------------------------------------------
//	COpenGLGPU.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CGPU.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPU::Procs

struct CGPU::Procs {
	// Procs
	typedef	void				(*AcquireContextProc)(void* userData);
	typedef	bool				(*TryAcquireContextProc)(void* userData);
	typedef	void				(*ReleaseContextProc)(void* userData);
	typedef S2DSizeU16			(*GetSizeProc)(void* userData);
	typedef	Float32				(*GetScaleProc)(void* userData);
#if defined(TARGET_OS_IOS)
	typedef	void*				(*GetRenderBufferStorageContextProc)(void* userData);
	typedef	CVEAGLContext		(*GetContextProc)(void* userData);
#endif
#if defined(TARGET_OS_MACOS)
	typedef	CGLContextObj		(*GetContextProc)(void* userData);
	typedef	CGLPixelFormatObj	(*GetPixelFormatProc)(void* userData);
#endif

						// Lifecycle methods
						Procs(AcquireContextProc acquireContextProc,
								TryAcquireContextProc tryAcquireContextProc, ReleaseContextProc releaseContextProc,
								GetSizeProc getSizeProc, GetScaleProc getScaleProc,
#if defined(TARGET_OS_IOS)
								GetRenderBufferStorageContextProc getRenderBufferStorageContextProc,
								GetContextProc getContextProc,
#endif
#if defined(TARGET_OS_MACOS)
								GetContextProc getContextProc, GetPixelFormatProc getPixelFormatProc,
#endif
								void* userData) :
							mAcquireContextProc(acquireContextProc), mTryAcquireContextProc(tryAcquireContextProc),
									mReleaseContextProc(releaseContextProc), mGetSizeProc(getSizeProc),
									mGetScaleProc(getScaleProc),
#if defined(TARGET_OS_IOS)
									mGetRenderBufferStorageContextProc(getRenderBufferStorageContextProc),
									mGetContextProc(getContextProc),
#endif
#if defined(TARGET_OS_MACOS)
									mGetContextProc(getContextProc), mGetPixelFormatProc(getPixelFormatProc),
#endif
									mUserData(userData)
							{}
						Procs(const Procs& other) :
							mAcquireContextProc(other.mAcquireContextProc),
									mTryAcquireContextProc(other.mTryAcquireContextProc),
									mReleaseContextProc(other.mReleaseContextProc), mGetSizeProc(other.mGetSizeProc),
									mGetScaleProc(other.mGetScaleProc),
#if defined(TARGET_OS_IOS)
									mGetRenderBufferStorageContextProc(other.mGetRenderBufferStorageContextProc),
									mGetContextProc(other.mGetContextProc),
#endif
#if defined(TARGET_OS_MACOS)
									mGetContextProc(other.mGetContextProc),
									mGetPixelFormatProc(other.mGetPixelFormatProc),
#endif
									mUserData(other.mUserData)
							{}

						// Instance methods
	void				acquireContext() const
							{ mAcquireContextProc(mUserData); }
	bool				tryAcquireContext() const
							{ return mTryAcquireContextProc(mUserData); }
	void				releaseContext() const
							{ mReleaseContextProc(mUserData); }
	S2DSizeU16			getSize() const
							{ return mGetSizeProc(mUserData); }
	Float32				getScale() const
							{ return mGetScaleProc(mUserData); }
#if defined(TARGET_OS_IOS)
	void*				getRenderBufferStorageContext() const
								{ return mGetRenderBufferStorageContextProc(mUserData); }
	CVEAGLContext		getContext() const
							{ return mGetContextProc(mUserData); }
#endif
#if defined(TARGET_OS_MACOS)
	CGLContextObj		getContext() const
							{ return mGetContextProc(mUserData); }
	CGLPixelFormatObj	getPixelFormat() const
							{ return mGetPixelFormatProc(mUserData); }
#endif

	// Properties
	private:
		AcquireContextProc					mAcquireContextProc;
		TryAcquireContextProc				mTryAcquireContextProc;
		ReleaseContextProc					mReleaseContextProc;
		GetSizeProc							mGetSizeProc;
		GetScaleProc						mGetScaleProc;
#if defined(TARGET_OS_IOS)
		GetRenderBufferStorageContextProc	mGetRenderBufferStorageContextProc;
		GetContextProc						mGetContextProc;
#endif
#if defined(TARGET_OS_MACOS)
		GetContextProc						mGetContextProc;
		GetPixelFormatProc					mGetPixelFormatProc;
#endif
		void*								mUserData;
};
