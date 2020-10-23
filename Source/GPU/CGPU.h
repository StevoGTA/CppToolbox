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

				SGPUVertexBuffer		allocateVertexBuffer(UInt32 perVertexByteCount, const CData& data);
				SGPUBuffer				allocateIndexBuffer(const CData& data);
				void					disposeBuffer(const SGPUBuffer& buffer);

				void					renderStart(const S2DSizeF32& size2D, Float32 fieldOfViewAngle3D = 0.0f,
												Float32 aspectRatio3D = 1.0f, Float32 nearZ3D = 0.01f,
												Float32 farZ3D = 100.0f, const S3DPointF32& camera3D = S3DPointF32(),
												const S3DPointF32& target3D = S3DPointF32(),
												const S3DVectorF32& up3D = S3DVectorF32()) const;
				void					render(CGPURenderState& renderState, EGPURenderType type, UInt32 count,
												UInt32 offset);
				void					renderIndexed(CGPURenderState& renderState, EGPURenderType type, UInt32 count,
												UInt32 offset);
				void					renderEnd() const;

	// Properties
	private:
		CGPUInternals*	mInternals;
};
