//----------------------------------------------------------------------------------------------------------------------
//	CDirectXRenderState.h			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#pragma once

#undef Delete
#include <d3d11_3.h>
#include <DirectXMath.h>
#define Delete(x)	{ delete x; x = nil; }

using namespace DirectX;

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPURenderState::CommitInfo

struct CGPURenderState::CommitInfo {
	// Lifecycle methods
	CommitInfo(ID3D11Device& d3dDevice, ID3D11DeviceContext3& d3dDeviceContext,
			ID3D11BlendState& d3dBlendState, const XMFLOAT4X4& projectionMatrix, const XMFLOAT4X4& viewMatrix) :
		mD3DDevice(d3dDevice), mD3DDeviceContext(d3dDeviceContext), mD3DBlendState(d3dBlendState),
				mProjectionMatrix(projectionMatrix), mViewMatrix(viewMatrix)
		{}

	// Properties
			ID3D11Device&			mD3DDevice;
			ID3D11DeviceContext3&	mD3DDeviceContext;
			ID3D11BlendState&		mD3DBlendState;
	const	XMFLOAT4X4&				mProjectionMatrix;
	const	XMFLOAT4X4&				mViewMatrix;
};
