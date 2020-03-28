//----------------------------------------------------------------------------------------------------------------------
//	COpenGLBuiltIns.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "COpenGLProgram.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: COpenGLBuiltIns

class COpenGLBuiltIns {
	public:
		// MARK: Class methods
		static	const	COpenGLVertexShader&	getBasicVertexShader();

		static	const	COpenGLFragmentShader&	getOpacityFragmentShader();

		static	const	COpenGLProgram&			getOpacityProgram();
};
