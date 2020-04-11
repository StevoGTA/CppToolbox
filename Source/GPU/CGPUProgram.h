//----------------------------------------------------------------------------------------------------------------------
//	CGPUProgram.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CGPUTexture.h"
#include "CMatrix.h"
#include "SGPUBuffer.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPUVertexShader

class CGPUVertexShader {
	// Methods
	public:
				// Lifecycle methods
				CGPUVertexShader() {}
		virtual	~CGPUVertexShader() {}
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUFragmentShader

class CGPUFragmentShader {
	// Methods
	public:
				// Lifecycle methods
				CGPUFragmentShader() {}
		virtual	~CGPUFragmentShader() {}
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPUProgram

class CGPUProgramInternals;
class CGPUProgram {
	// Methods
	public:
										// Lifecycle methods
										CGPUProgram(const CGPUVertexShader& vertexShader,
												const CGPUFragmentShader& fragmentShader);
		virtual							~CGPUProgram();

										// Instance methods
						void			setup(const SMatrix4x4_32& viewMatrix, const SMatrix4x4_32& projectionMatrix);
				const	SMatrix4x4_32&	getViewMatrix() const;
				const	SMatrix4x4_32&	getProjectionMatrix() const;

										// Subclass methods
		virtual			void			setModelMatrix(const SMatrix4x4_32& modelMatrix) = 0;

		virtual			void			willUse() const;
		virtual			void			didFinish() const;

	// Properties
	protected:
		CGPUProgramInternals*	mGPUProgramInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUTextureProgram

class CGPUTextureProgramInternals;
class CGPUTextureProgram : public CGPUProgram {
	// Methods
	public:
				// Lifecycle methods
				CGPUTextureProgram(const CGPUVertexShader& vertexShader, const CGPUFragmentShader& fragmentShader);
				~CGPUTextureProgram();

				// CGPUProgram methods
		void	willUse() const;
		void	didFinish() const;

				// Instance methods
		void	setupVertexTextureInfo(const SGPUVertexBuffer& gpuVertexBuffer, UInt32 triangleCount,
						const SGPUTextureInfo& gpuTextureInfo);

	// Properties
	private:
		CGPUTextureProgramInternals*	mInternals;
};
