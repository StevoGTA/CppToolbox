//----------------------------------------------------------------------------------------------------------------------
//	CGPU.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CGPUTexture.h"
#include "SGPUBuffer.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPUProcsInfo

struct CGPUProcsInfo {
	// Lifecycle methods
	CGPUProcsInfo() {}
	CGPUProcsInfo(const CGPUProcsInfo& other) {}

	// Properties
	void*	mUserData;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - SGPUTextureInfo

struct SGPUTextureInfo {
			// Lifecycle methods
			SGPUTextureInfo(const SGPUTextureSize& gpuTextureSize, Float32 maxU, Float32 maxV,
					void* internalReference) :
				mGPUTextureSize(gpuTextureSize), mMaxU(maxU), mMaxV(maxV), mInternalReference(internalReference)
				{}
			SGPUTextureInfo(const SGPUTextureInfo& other) :
				mGPUTextureSize(other.mGPUTextureSize), mMaxU(other.mMaxU), mMaxV(other.mMaxV),
						mInternalReference(other.mInternalReference)
				{}
			SGPUTextureInfo() : mGPUTextureSize(), mMaxU(0.0), mMaxV(0.0), mInternalReference(nil) {}

			// Instance methods
	bool	isValid() const
				{ return mMaxU != 0.0; }

	// Properties
	SGPUTextureSize	mGPUTextureSize;
	Float32			mMaxU;
	Float32			mMaxV;
	void*			mInternalReference;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPU

class CGPUInternals;
class CGPU {
	// Methods
	public:
									// Lifecycle methods
									CGPU(const CGPUProcsInfo& procsInfo);
		virtual						~CGPU();

									// Instance methods
		virtual	void				setup(const S2DSize32& size, void* extraData = nil) = 0;

		virtual	SGPUTextureInfo		registerTexture(const CGPUTexture& gpuTexture) = 0;
		virtual	void				unregisterTexture(const SGPUTextureInfo& gpuTextureInfo) = 0;

		virtual	SGPUVertexBuffer	allocateVertexBuffer(EGPUVertexBufferType gpuVertexBufferType,
													UInt32 vertexCount) = 0;
		virtual	void				disposeBuffer(const SGPUBuffer& buffer) = 0;

		virtual	void				renderStart() const = 0;
		virtual	void				renderSetClipPlane(Float32 clipPlane[4]) = 0;
		virtual	void				renderClearClipPlane() = 0;
		virtual	void				renderTriangleStrip(const SGPUVertexBuffer& gpuVertexBuffer, UInt32 vertexCount,
											const SGPUTextureInfo& gpuTextureInfo,
											OV<Float32> alpha = OV<Float32>()) = 0;
		virtual	void				renderEnd() const = 0;

	// Properties
	private:
		CGPUInternals*	mInternals;
};
