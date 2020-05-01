//----------------------------------------------------------------------------------------------------------------------
//	CGPU.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CGPUProgram.h"
#include "CGPUTexture.h"
#include "SGPUBuffer.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPU

struct CGPUProcsInfo;
class CGPUInternals;
class CGPU {
	// Methods
	public:
								// Lifecycle methods
								CGPU(const CGPUProcsInfo& procsInfo);
								~CGPU();

								// Instance methods
		void					setup(const S2DSizeF32& size, void* extraData = nil);

		SGPUTextureReference	registerTexture(const CData& data, EGPUTextureDataFormat gpuTextureDataFormat,
										const S2DSizeU16& size);
		void					unregisterTexture(SGPUTextureReference& gpuTexture);

		SGPUVertexBuffer		allocateVertexBuffer(const SGPUVertexBufferInfo& gpuVertexBufferInfo,
										UInt32 vertexCount);
		void					disposeBuffer(const SGPUBuffer& buffer);

		void					renderStart() const;
		void					setViewMatrix(const SMatrix4x4_32& viewMatrix);
		void					renderTriangleStrip(CGPUProgram& program, const SMatrix4x4_32& modelMatrix,
										UInt32 triangleCount);
		void					renderEnd() const;

	// Properties
	private:
		CGPUInternals*	mInternals;
};
