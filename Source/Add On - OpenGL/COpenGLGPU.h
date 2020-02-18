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
		void				setup(const S2DSize32& size);

		SGPUTextureInfo		registerTexture(const CGPUTexture& gpuTexture);
		void				unregisterTexture(const SGPUTextureInfo& gpuTextureInfo);

		SGPUVertexBuffer	allocateVertexBuffer(EGPUVertexBufferType gpuVertexBufferType, UInt32 vertexCount);
		void				disposeBuffer(const SGPUBuffer& buffer);

		void				renderStart() const;
		void				renderSetClipPlane(Float32 clipPlane[4]);
		void				renderClearClipPlane();
		void				renderTriangleStrip(const SGPUVertexBuffer& gpuVertexBuffer, UInt32 vertexCount,
									const SGPUTextureInfo& gpuTextureInfo, OV<Float32> alpha = OV<Float32>());
		void				renderEnd() const;

	// Properties
	private:
		COpenGLGPUInternals*	mInternals;
};
