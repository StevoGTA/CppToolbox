//----------------------------------------------------------------------------------------------------------------------
//	CGPURenderState.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CGPUShader.h"
#include "CGPUTexture.h"
#include "CMatrix.h"
#include "SGPUVertexBuffer.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: Render Mode

enum EGPURenderStateMode {
	kGPURenderStateMode2D,
	kGPURenderStateMode3D,
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: SGPURenderStateCommitInfo

struct SGPURenderStateCommitInfo;

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPURenderState

class CGPURenderStateInternals;
class CGPURenderState {
	// Methods
	public:
							// Lifecycle methods
							CGPURenderState(EGPURenderStateMode mode, CGPUVertexShader& vertexShader,
									CGPUFragmentShader& fragmentShader);
							~CGPURenderState();

							// Instance methods
		EGPURenderStateMode	getMode() const;

		void				setModelMatrix(const SMatrix4x4_32& modelMatrix);
		void				setVertexTextureInfo(const SGPUVertexBuffer& gpuVertexBuffer,
									const TArray<const CGPUTexture>& gpuTextures);
		void				commit(const SGPURenderStateCommitInfo& renderStateCommitInfo);

	// Properties
	protected:
		CGPURenderStateInternals*	mInternals;
};
