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
										~CMetalVertexShader();

										// Instance methods
						void			setModelMatrix(const SMatrix4x4_32& modelMatrix);
				const	SMatrix4x4_32&	getModelMatrix() const;

										// Subclass methods
		virtual			CString			getName() const = 0;

		virtual			void			setup(id<MTLRenderCommandEncoder> renderCommandEncoder,
												MetalBufferCache* metalBufferCache) const = 0;

	protected:
						// Lifecycle methods
						CMetalVertexShader();

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
						~CMetalFragmentShader();

						// Instance methods
		virtual	CString	getName() const = 0;

		virtual	void	setup(id<MTLRenderCommandEncoder> renderCommandEncoder, MetalBufferCache* metalBufferCache)
								const = 0;

	protected:
						// Lifecycle methods
						CMetalFragmentShader();

	// Properties
	private:
		CMetalFragmentShaderInternals*	mInternals;
};
