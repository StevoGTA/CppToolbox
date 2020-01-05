//----------------------------------------------------------------------------------------------------------------------
//	CGPURenderEngine.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CGPUTexture.h"
#include "SGPUBuffer.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPURenderEngineProcsInfo

typedef	void	(*GPURenderEngineAcquireContextProc)(void* userData);
typedef	void	(*GPURenderEngineReleaseContextProc)(void* userData);

struct CGPURenderEngineProcsInfo {
	// Lifecycle methods
	CGPURenderEngineProcsInfo(GPURenderEngineAcquireContextProc acquireContextProc,
			GPURenderEngineReleaseContextProc releaseContextProc, void* userData) :
		mAcquireContextProc(acquireContextProc), mReleaseContextProc(releaseContextProc), mUserData(userData)
		{}
	CGPURenderEngineProcsInfo(const CGPURenderEngineProcsInfo& other) :
		mAcquireContextProc(other.mAcquireContextProc), mReleaseContextProc(other.mReleaseContextProc),
				mUserData(other.mUserData)
		{}

	// Properties
	GPURenderEngineAcquireContextProc	mAcquireContextProc;
	GPURenderEngineReleaseContextProc	mReleaseContextProc;
	void*								mUserData;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: SGPURenderEngineTextureInfo

struct SGPURenderEngineTextureInfo {
	// Lifecycle methods
	SGPURenderEngineTextureInfo(const SGPUTextureSize& gpuTextureSize, Float32 maxU, Float32 maxV,
			void* internalReference) :
		mGPUTextureSize(gpuTextureSize), mMaxU(maxU), mMaxV(maxV), mInternalReference(internalReference)
		{}
	SGPURenderEngineTextureInfo(const SGPURenderEngineTextureInfo& other) :
		mGPUTextureSize(other.mGPUTextureSize), mMaxU(other.mMaxU), mMaxV(other.mMaxV),
				mInternalReference(other.mInternalReference)
		{}
	SGPURenderEngineTextureInfo() : mGPUTextureSize(), mMaxU(0.0), mMaxV(0.0), mInternalReference(nil) {}

	// Properties
	SGPUTextureSize	mGPUTextureSize;
	Float32			mMaxU;
	Float32			mMaxV;
	void*			mInternalReference;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPURenderEngine

class CGPURenderEngineInternals;
class CGPURenderEngine {
	// Methods
	public:
											// Lifecycle methods
											CGPURenderEngine(const CGPURenderEngineProcsInfo& procsInfo);
		virtual								~CGPURenderEngine();

											// Instance methods
		virtual	void						setup(const S2DSize32& size) = 0;

		virtual	SGPURenderEngineTextureInfo	registerTexture(const CGPUTexture& gpuTexture) = 0;
		virtual	void						unregisterTexture(
													const SGPURenderEngineTextureInfo& gpuRenderEngineTextureInfo) = 0;

		virtual	SGPUVertexBuffer			allocateVertexBuffer(EGPUVertexBufferType gpuVertexBufferType,
													UInt32 vertexCount) = 0;
		virtual	void						disposeBuffer(const SGPUBuffer& buffer) = 0;

		virtual	void						renderStart() const = 0;
		virtual	void						renderTriangleStrip(const SGPUVertexBuffer& gpuVertexBuffer,
													UInt32 vertexCount,
													const SGPURenderEngineTextureInfo& gpuRenderEngineTextureInfo,
													Float32* alphaOrNil = nil) = 0;

		virtual	void						renderEnd() const = 0;

											// Subclass methods
	protected:
				void						acquireContext() const;
				void						releaseContext() const;

	// Properties
	private:
		CGPURenderEngineInternals*	mInternals;
};
