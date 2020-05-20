//----------------------------------------------------------------------------------------------------------------------
//	CGPUShader.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CMatrix.h"
#include "CUUID.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPUVertexShader

class CGPUVertexShaderInternals;
class CGPUVertexShader {
	// Methods
	public:
											// Lifecycle methods
		virtual								~CGPUVertexShader();

											// Instance methods
				const	CUUID&				getUUID() const;

											// Class methods
		static			CGPUVertexShader&	getBasic();
		static			CGPUVertexShader&	getClip(const SMatrix4x1_32& clipPlane);

	protected:
											// Lifecycle methods
											CGPUVertexShader();

	// Properties
	private:
		CGPUVertexShaderInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUFragmentShader

class CGPUFragmentShaderInternals;
class CGPUFragmentShader {
	// Methods
	public:
											// Lifecycle methods
		virtual								~CGPUFragmentShader();

											// Instance methods
				const	CUUID&				getUUID() const;

											// Class methods
		static			CGPUFragmentShader&	getBasic();
		static			CGPUFragmentShader&	getOpacity(Float32 opacity);

	protected:
											// Lifecycle methods
											CGPUFragmentShader();

	// Properties
	private:
		CGPUFragmentShaderInternals*	mInternals;
};
