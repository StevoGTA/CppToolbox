//----------------------------------------------------------------------------------------------------------------------
//	CMetalShader.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CGPUShader.h"
#include "MetalBufferCache.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMetalVertexShader

class CMetalVertexShaderInternals;
class CMetalVertexShader : public CGPUVertexShader {
	// Methods
	public:
										// Lifecycle methods
										CMetalVertexShader(const CString& name);
										~CMetalVertexShader();

										// Instance methods
				const	CString&		getName() const;

						void			setModelMatrix(const SMatrix4x4_32& modelMatrix);
				const	SMatrix4x4_32&	getModelMatrix() const;

										// Subclass methods
		virtual			void			setup(id<MTLRenderCommandEncoder> renderCommandEncoder,
												MetalBufferCache* metalBufferCache) const = 0;

	// Properties
	private:
		CMetalVertexShaderInternals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMetalFragmentShader

class CMetalFragmentShaderInternals;
class CMetalFragmentShader : public CGPUFragmentShader {
	// Methods
	public:
									// Lifecycle methods
									CMetalFragmentShader(const CString& name);
									~CMetalFragmentShader();

									// Instance methods
				const	CString&	getName() const;

									// Subclass methods
		virtual			void		setup(id<MTLRenderCommandEncoder> renderCommandEncoder,
											MetalBufferCache* metalBufferCache) const = 0;

	// Properties
	private:
		CMetalFragmentShaderInternals*	mInternals;
};
