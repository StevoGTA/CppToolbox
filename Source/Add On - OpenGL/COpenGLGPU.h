//----------------------------------------------------------------------------------------------------------------------
//	COpenGLGPU.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CGPU.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: COpenGLGPUProcsInfo

typedef	void	(*COpenGLGPUAcquireContextProc)(void* userData);
typedef	bool	(*COpenGLGPUTryAcquireContextProc)(void* userData);
typedef	void	(*COpenGLGPUReleaseContextProc)(void* userData);

struct COpenGLGPUProcsInfo : CGPUProcsInfo {
			// Lifecycle methods
			COpenGLGPUProcsInfo(COpenGLGPUAcquireContextProc acquireContextProc,
					COpenGLGPUTryAcquireContextProc tryAcquireContextProc,
					COpenGLGPUReleaseContextProc releaseContextProc, void* userData) :
				CGPUProcsInfo(), mAcquireContextProc(acquireContextProc), mTryAcquireContextProc(tryAcquireContextProc),
						mReleaseContextProc(releaseContextProc), mUserData(userData)
				{}
			COpenGLGPUProcsInfo(const COpenGLGPUProcsInfo& other) :
				CGPUProcsInfo(other), mAcquireContextProc(other.mAcquireContextProc),
						mTryAcquireContextProc(other.mTryAcquireContextProc),
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

//----------------------------------------------------------------------------------------------------------------------
// MARK: - COpenGLGPU

class COpenGLGPUInternals;
class COpenGLGPU : public CGPU {
	// Methods
	public:
							// Lifecycle methods
							COpenGLGPU(const COpenGLGPUProcsInfo& procsInfo);
							~COpenGLGPU();

							// CGPU methods
		void				setup(const S2DSize32& size, void* extraData = nil);

		SGPUTextureInfo		registerTexture(const CGPUTexture& gpuTexture);
		void				unregisterTexture(const SGPUTextureInfo& gpuTextureInfo);

		SGPUVertexBuffer	allocateVertexBuffer(EGPUVertexBufferType gpuVertexBufferType, UInt32 vertexCount);
		void				disposeBuffer(const SGPUBuffer& buffer);

		void				renderStart() const;
		void				renderSetClipPlane(Float32 clipPlane[4]);
		void				renderClearClipPlane();
		void				renderTriangleStrip(const SGPUVertexBuffer& gpuVertexBuffer, UInt32 triangleCount,
									const SGPUTextureInfo& gpuTextureInfo, OV<Float32> alpha = OV<Float32>());
		void				renderEnd() const;

	// Properties
	private:
		COpenGLGPUInternals*	mInternals;
};
