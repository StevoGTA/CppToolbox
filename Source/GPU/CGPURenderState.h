//----------------------------------------------------------------------------------------------------------------------
//	CGPURenderState.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CGPUShader.h"
#include "CGPUTexture.h"
#include "CMatrix.h"
#include "SGPUBuffer.h"

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
				CGPURenderState(CGPUVertexShader& vertexShader, CGPUFragmentShader& fragmentShader);
				~CGPURenderState();

				// Instance methods
		void	setProjectionMatrix(const SMatrix4x4_32& projectionMatrix);
		void	setViewMatrix(const SMatrix4x4_32& viewMatrix);
		void	setModelMatrix(const SMatrix4x4_32& modelMatrix);

		void	setVertexTextureInfo(const SGPUVertexBuffer& gpuVertexBuffer, UInt32 triangleOffset,
						const TArray<const CGPUTexture>& gpuTextures);
		UInt32	getTriangleOffset() const;

		void	commit(const SGPURenderStateCommitInfo& renderStateCommitInfo);

	// Properties
	protected:
		CGPURenderStateInternals*	mInternals;
};
