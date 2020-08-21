//----------------------------------------------------------------------------------------------------------------------
//	CGPUShader.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CMatrix.h"
#include "CUUID.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPUVertexShader

class CGPUVertexShader {
	// Methods
	public:
									// Lifecycle methods
		virtual						~CGPUVertexShader() {}

									// Class methods
		static	CGPUVertexShader&	getBasic();
		static	CGPUVertexShader&	getClip(const SMatrix4x1_32& clipPlane);

	protected:
									// Lifecycle methods
									CGPUVertexShader() {}
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPUFragmentShader

class CGPUFragmentShader {
	// Methods
	public:
									// Lifecycle methods
		virtual						~CGPUFragmentShader() {}

									// Class methods
		static	CGPUFragmentShader&	getBasic();
		static	CGPUFragmentShader&	getOpacity(Float32 opacity);

	protected:
									// Lifecycle methods
									CGPUFragmentShader() {}
};
