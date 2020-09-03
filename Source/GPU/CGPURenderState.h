//----------------------------------------------------------------------------------------------------------------------
//	CGPURenderState.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CGPUShader.h"
#include "CGPUTexture.h"
#include "CMatrix.h"
#include "SGPUVertexBuffer.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: EGPURenderMode

enum EGPURenderMode {
	kGPURenderMode2D,
	kGPURenderMode3D,
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPURenderState

class CGPURenderStateInternals;
struct SGPURenderStateCommitInfo;

class CGPURenderState {
	// Methods
	public:
						// Lifecycle methods
						CGPURenderState(EGPURenderMode renderMode, CGPUVertexShader& vertexShader,
								CGPUFragmentShader& fragmentShader);
						~CGPURenderState();

						// Instance methods
		void			setViewMatrix(const SMatrix4x4_32& viewMatrix);
		void			setModelMatrix(const SMatrix4x4_32& modelMatrix);
		void			setVertexTextureInfo(const SGPUVertexBuffer& gpuVertexBuffer,
								const TArray<const CGPUTexture>& gpuTextures);

		EGPURenderMode	getRenderMode() const;
		void			commit(const SGPURenderStateCommitInfo& renderStateCommitInfo);

	// Properties
	protected:
		CGPURenderStateInternals*	mInternals;
};
