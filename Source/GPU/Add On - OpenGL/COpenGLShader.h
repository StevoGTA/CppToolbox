//----------------------------------------------------------------------------------------------------------------------
//	COpenGLShader.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CDictionary.h"
#include "CGPUShader.h"
#include "CUUID.h"

#if defined(TARGET_OS_IOS)
	#include <OpenGLES/ES3/glext.h>
#endif

//#if defined(TARGET_OS_LINUX)
//	#include <GLES/gl.h>
//#endif

#if defined(TARGET_OS_MACOS)
	#include <OpenGL/gl3.h>
#endif

//----------------------------------------------------------------------------------------------------------------------
// MARK: COpenGLVertexShader

class COpenGLVertexShader : public CGPUVertexShader {
	// Classes
	private:
		class Internals;

	// Methods
	public:
											// Lifecycle methods
											COpenGLVertexShader(const CString& string,
													const TArray<CString>& attributeNames,
													const TArray<CString>& uniformNames);
											~COpenGLVertexShader();

											// Instance methods
						GLuint				getShader() const;
				const	CUUID&				getUUID() const;

				const	TArray<CString>&	getAttributeNames() const;
				const	TArray<CString>&	getUniformNames() const;

											// Subclass methods
		virtual			void				setAttibutes(const CDictionary& attributeInfo) = 0;
		virtual			void				setUniforms(const CDictionary& uniformInfo,
													const SMatrix4x4_32& projectionMatrix,
													const SMatrix4x4_32& viewMatrix, const SMatrix4x4_32& modelMatrix)
													= 0;

		virtual			void				configureGL() {}
		virtual			void				resetGL() {}

	// Properties
	private:
		Internals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - COpenGLFragmentShader

class COpenGLFragmentShader : public CGPUFragmentShader {
	// Classes
	private:
		class Internals;

	// Methods
	public:
											// Lifecycle methods
											COpenGLFragmentShader(const CString& string,
													const TArray<CString>& uniformNames);
											~COpenGLFragmentShader();

											// Instance methods
						GLuint				getShader() const;
				const	CUUID&				getUUID() const;

				const	TArray<CString>&	getUniformNames() const;

											// Subclass methods
		virtual			void				setUniforms(const CDictionary& uniformInfo) = 0;

	// Properties
	private:
		Internals*	mInternals;
};
