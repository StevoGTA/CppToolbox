//----------------------------------------------------------------------------------------------------------------------
//	CDirectXShader.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#include "CFilesystemPath.h"
#include "CGPUShader.h"

#undef Delete
#include <d3d11_3.h>
#include <DirectXMath.h>

using namespace DirectX;

//----------------------------------------------------------------------------------------------------------------------
// MARK: CDirectXVertexShader

class CDirectXVertexShaderInternals;
class CDirectXVertexShader : public CGPUVertexShader {
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
		virtual			void		setupInputLayout(ID3D11Device& d3dDevice, const CData& shaderData) = 0;

	// Properties
	private:
		CDirectXVertexShaderInternals* mInternals;
};

//----------------------------------------------------------------------------------------------------------------------
// MARK: - CDirectXPixelShader

class CDirectXPixelShaderInternals;
class CDirectXPixelShader : public CGPUFragmentShader {
	// Methods
	public:
						// Lifecycle methods
						CDirectXPixelShader(const CFilesystemPath& filesystemPath);
						~CDirectXPixelShader();

						// Instance methods
		virtual	void	setup(ID3D11Device& d3dDevice, ID3D11DeviceContext& d3dDeviceContext);

	// Properties
	private:
		CDirectXPixelShaderInternals* mInternals;
};
