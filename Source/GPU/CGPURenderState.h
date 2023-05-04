//----------------------------------------------------------------------------------------------------------------------
//	CGPURenderState.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CGPUShader.h"
#include "CGPUTexture.h"
#include "CMatrix.h"
#include "SGPUBuffer.h"
#include "TWrappers.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPURenderState

class CGPURenderState {
	// Enums
	public:
		enum Mode {
			kMode2D,
			kMode3D,
		};

	// Structs
	public:
		struct CommitInfo;

	// Classes
	private:
		class Internals;

	// Methods
	public:
															// Lifecycle methods
															CGPURenderState(Mode mode, CGPUVertexShader& vertexShader,
																	CGPUFragmentShader& fragmentShader);
															~CGPURenderState();

															// Instance methods
				void										setModelMatrix(const SMatrix4x4_32& modelMatrix);
				void										setVertexBuffer(const SGPUVertexBuffer& gpuVertexBuffer);
		const	OR<const SGPUBuffer>&						getIndexBuffer() const;
				void										setIndexBuffer(const SGPUBuffer& gpuIndexBuffer);
				void										setTextures(
																	const TArray<const I<CGPUTexture> >& gpuTextures);
		const	OR<const TArray<const I<CGPUTexture> > >	getTextures() const;

				Mode										getMode() const;
				void										commit(const CommitInfo& commitInfo);

	// Properties
	protected:
		Internals*	mInternals;
};
