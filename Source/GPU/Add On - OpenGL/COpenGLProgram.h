//----------------------------------------------------------------------------------------------------------------------
//	COpenGLProgram.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "COpenGLShader.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: COpenGLProgram

class COpenGLProgram {
	// Classes
	private:
		class Internals;

	// Methods
	public:
				// Lifecycle methods
				COpenGLProgram(COpenGLVertexShader& vertexShader, COpenGLFragmentShader& fragmentShader);
				COpenGLProgram(const COpenGLProgram& other);
				~COpenGLProgram();

				// Instance methods
		void	prepare(const SMatrix4x4_32& projectionMatrix, const SMatrix4x4_32& viewMatrix,
						const SMatrix4x4_32& modelMatrix) const;

	// Properties
	private:
		Internals*	mInternals;
};
