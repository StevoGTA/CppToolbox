//----------------------------------------------------------------------------------------------------------------------
//	CGPU.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CGPURenderState.h"
#include "CGPUTexture.h"
#include "SGPUBuffer.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPU

struct SGPUProcsInfo;
class CGPUInternals;
class CGPU {
	// Methods
	public:
										// Lifecycle methods
										CGPU(const SGPUProcsInfo& procsInfo);
										~CGPU();

										// Instance methods
				void					setViewMatrix(const SMatrix4x4_32& viewMatrix);
		const	SMatrix4x4_32&			getViewMatrix() const;

				SGPUTextureReference	registerTexture(const CData& data, EGPUTextureDataFormat gpuTextureDataFormat,
												const S2DSizeU16& size);
				void					unregisterTexture(SGPUTextureReference& gpuTexture);

				SGPUVertexBuffer		allocateVertexBuffer(const SGPUVertexBufferInfo& gpuVertexBufferInfo,
												const CData& data);
				void					disposeBuffer(const SGPUBuffer& buffer);

				void					renderStart() const;
				void					renderTriangleStrip(CGPURenderState& renderState, UInt32 triangleCount);
				void					renderEnd() const;

	// Properties
	private:
		CGPUInternals*	mInternals;
};
