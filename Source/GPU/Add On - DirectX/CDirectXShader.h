//----------------------------------------------------------------------------------------------------------------------
//	CDirectXShader.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CFilesystemPath.h"
#include "CGPUShader.h"

#undef Delete
#include <d3d11_3.h>
#include <DirectXMath.h>
#define Delete(x)	{ delete x; x = nil; }

using namespace DirectX;

//----------------------------------------------------------------------------------------------------------------------
// MARK: CDirectXVertexShader

class CDirectXVertexShader : public CGPUVertexShader {
	// Classes
	private:
		class Internals;

	// Methods
	public:
									// Lifecycle methods
									CDirectXVertexShader(const CFilesystemPath& filesystemPath);
									~CDirectXVertexShader();

									// Instance methods
		virtual			void		setup(ID3D11Device& d3dDevice, ID3D11DeviceContext3& d3dDeviceContext,
											const XMFLOAT4X4& projectionMatrix, const XMFLOAT4X4& viewMatrix);

						void		setModelMatrix(const SMatrix4x4_32& modelMatrix);
				const	XMFLOAT4X4&	getModelMatrix() const;

									// Subclass methods
		virtual			void		createResources(ID3D11Device& d3dDevice, const CData& shaderData) = 0;

	// Properties
	private:
		Internals* mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDirectXPixelShader

class CDirectXPixelShader : public CGPUFragmentShader {
	// Classes
	private:
		class Internals;

	// Methods
	public:
						// Lifecycle methods
						CDirectXPixelShader(const CFilesystemPath& filesystemPath);
						~CDirectXPixelShader();

						// Instance methods
		virtual	void	setup(ID3D11Device& d3dDevice, ID3D11DeviceContext3& d3dDeviceContext);

						// Subclass methods
		virtual	void	createResources(ID3D11Device& d3dDevice, const CData& shaderData)
							{}

	// Properties
	private:
		Internals* mInternals;
};
