//----------------------------------------------------------------------------------------------------------------------
//	CMetalShader.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CGPUShader.h"
#include "MetalBufferCache.h"

//----------------------------------------------------------------------------------------------------------------------
// MARK: CMetalVertexShader

class CMetalVertexShader : public CGPUVertexShader {
	// Classes
	private:
		class Internals;

	// Methods
	public:
												// Lifecycle methods
												CMetalVertexShader(const CString& name);
												~CMetalVertexShader();

												// Instance methods
				const	CString&				getName() const;

						void					setModelMatrix(const SMatrix4x4_32& modelMatrix);
				const	SMatrix4x4_32&			getModelMatrix() const;

												// Subclass methods
		virtual			bool					requiresDepthTest() const = 0;
		virtual			MTLVertexDescriptor*	getVertexDescriptor() const = 0;
		virtual			void					setup(id<MTLRenderCommandEncoder> renderCommandEncoder,
														id<MTLBuffer> vertexBuffer, MetalBufferCache* metalBufferCache)
														const = 0;

	// Properties
	private:
		Internals*	mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CMetalFragmentShader

class CMetalFragmentShader : public CGPUFragmentShader {
	// Classes
	private:
		class Internals;

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
		Internals*	mInternals;
};
