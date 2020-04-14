//----------------------------------------------------------------------------------------------------------------------
//	CGPUProgramBuiltins.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CGPUProgram.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPUOpaqueProgram

class CGPUOpaqueProgramInternals;
class CGPUOpaqueProgram : public CGPUTextureProgram {
	// Methods
	public:
									// Lifecycle methods
									~CGPUOpaqueProgram();

									// CGPUProgram methods
				void				setModelMatrix(const SMatrix4x4_32& modelMatrix);

									// Class methods
		static	CGPUOpaqueProgram&	getProgram();

	private:
									// Lifecycle methods
									CGPUOpaqueProgram();

	// Properties
	private:
		CGPUOpaqueProgramInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPUOpacityProgram

class CGPUOpacityProgramInternals;
class CGPUOpacityProgram : public CGPUTextureProgram {
	// Methods
	public:
									// Lifecycle methods
									~CGPUOpacityProgram();

									// CGPUProgram methods
				void				setModelMatrix(const SMatrix4x4_32& modelMatrix);

									// Instance methods
				void				setupVertexTextureInfo(const SGPUVertexBuffer& gpuVertexBuffer,
											UInt32 triangleCount,
											const TArray<const SGPUTextureInfo>& gpuTextureInfos, Float32 opacity);

									// Class methods
		static	CGPUOpacityProgram&	getProgram();

	private:
									// Lifecycle methods
									CGPUOpacityProgram();

	// Properties
	private:
		CGPUOpacityProgramInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUClipOpacityProgram

class CGPUClipOpacityProgramInternals;
class CGPUClipOpacityProgram : public CGPUTextureProgram {
	// Methods
	public:
										// Lifecycle methods
										~CGPUClipOpacityProgram();

										// CGPUProgram methods
				void					setModelMatrix(const SMatrix4x4_32& modelMatrix);

				void					willUse() const;
				void					didFinish() const;

										// Instance methods
				void					setupVertexTextureInfo(const SGPUVertexBuffer& gpuVertexBuffer,
												UInt32 triangleCount,
												const TArray<const SGPUTextureInfo>& gpuTextureInfos,
												Float32 opacity);
				void					setClipPlane(const SMatrix4x1_32& clipPlane);

										// Class methods
		static	CGPUClipOpacityProgram&	getProgram();

	private:
										// Lifecycle methods
										CGPUClipOpacityProgram();

	// Properties
	private:
		CGPUClipOpacityProgramInternals*	mInternals;
};
