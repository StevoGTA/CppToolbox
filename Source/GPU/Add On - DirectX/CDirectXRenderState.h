//----------------------------------------------------------------------------------------------------------------------
//	CDirectXRenderState.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#undef Delete
#include <d3d11_3.h>
#include <DirectXMath.h>

using namespace DirectX;

//----------------------------------------------------------------------------------------------------------------------
// MARK: SGPURenderStateCommitInfo

struct SGPURenderStateCommitInfo {
	// Lifecycle methods
	SGPURenderStateCommitInfo(ID3D11Device& d3dDevice, ID3D11DeviceContext3& d3dDeviceContext,
			const XMFLOAT4X4& projectionMatrix, const XMFLOAT4X4& viewMatrix) :
		mD3DDevice(d3dDevice), mD3DDeviceContext(d3dDeviceContext), mProjectionMatrix(projectionMatrix),
				mViewMatrix(viewMatrix)
		{}

	// Properties
			ID3D11Device&			mD3DDevice;
			ID3D11DeviceContext3&	mD3DDeviceContext;
	const	XMFLOAT4X4&				mProjectionMatrix;
	const	XMFLOAT4X4&				mViewMatrix;
};
