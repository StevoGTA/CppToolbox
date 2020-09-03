//----------------------------------------------------------------------------------------------------------------------
//	CGPU.h			©2019 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CGPURenderState.h"
#include "CGPUTexture.h"
#include "SGPUBuffer.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: - EGPURenderType

enum EGPURenderType {
	kGPURenderTypeTriangleList,
	kGPURenderTypeTriangleStrip
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPU

struct SGPUProcsInfo;
class CGPUInternals;
class CGPU {
	// Methods
	public:
										// Lifecycle methods
										CGPU(const SGPUProcsInfo& procsInfo);
										~CGPU();

										// Instance methods
				SGPUTextureReference	registerTexture(const CData& data, EGPUTextureDataFormat gpuTextureDataFormat,
												const S2DSizeU16& size);
				void					unregisterTexture(SGPUTextureReference& gpuTexture);

				SGPUVertexBuffer		allocateVertexBuffer(const SGPUVertexBufferInfo& gpuVertexBufferInfo,
												const CData& data);
				void					disposeBuffer(const SGPUBuffer& buffer);

				void					renderStart(const S2DSizeF32& size, const S3DPoint32& camera = S3DPoint32(),
												const S3DPoint32& target = S3DPoint32()) const;
				void					render(CGPURenderState& renderState, EGPURenderType type, UInt32 count,
												UInt32 offset);
				void					renderIndexed(CGPURenderState& renderState, EGPURenderType type, UInt32 count,
												UInt32 offset);
				void					renderEnd() const;

	// Properties
	private:
		CGPUInternals*	mInternals;
};
