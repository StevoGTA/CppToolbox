//----------------------------------------------------------------------------------------------------------------------
//	CDirectXGPU.cpp			©2020 Stevo Brock	All rights reserved.
//----------------------------------------------------------------------------------------------------------------------

#include "CDirectXGPU.h"

#include "CDirectXRenderState.h"
#include "CDirectXTexture.h"
#include "ConcurrencyPrimitives.h"

#undef Delete

#include <agile.h>
#include <DirectXColors.h>
#include <dxgi1_4.h>
#include <wincodec.h>
#include <wrl/client.h>
#include <d2d1_3.h>
#include <dwrite_3.h>

#define Delete(x)	{ delete x; x = nil; }

using namespace D2D1;
using namespace Microsoft::WRL;
using namespace Platform;

//----------------------------------------------------------------------------------------------------------------------
// MARK: CGPUInternals

class CGPUInternals {
	public:
							CGPUInternals(const SGPUProcsInfo& procsInfo) :
								mProcsInfo(procsInfo),
										mD3DFeatureLevel(D3D_FEATURE_LEVEL_11_0),
										mD3DRenderTargetSize(),
										mOutputSize(),
										mLogicalSize(),
										mNativeOrientation(Windows::Graphics::Display::DisplayOrientations::None),
										mCurrentOrientation(Windows::Graphics::Display::DisplayOrientations::None),
										mDPI(-1.0f),
										mRenderDPI(-1.0f),

										mD3DBlendState(NULL),

										m_text(L"")
								{
									// Clear
									::memset(&m_textMetrics, 0, sizeof(DWRITE_TEXT_METRICS));

									// Setup stuffs
									createDeviceIndependentResources();
									createDeviceResources();
								}
							~CGPUInternals() {}

		void				createDeviceIndependentResources()
								{
									// Setup
									HRESULT	result;

									// Initialize Direct2D resources.
									D2D1_FACTORY_OPTIONS options;
									::memset(&options, 0, sizeof(D2D1_FACTORY_OPTIONS));
#if defined(DEBUG)
									// If the project is in a debug build, enable Direct2D debugging via SDK Layers.
									options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

									// Initialize the Direct2D Factory.
									result =
											D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED,
													__uuidof(ID2D1Factory3), &options, &mD2DFactoryComPtr);
									AssertFailIf(FAILED(result));

									// Initialize the DirectWrite Factory.
									result =
											DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory3),
													&mDWriteFactoryComPtr);
									AssertFailIf(FAILED(result));

									// Initialize the Windows Imaging Component (WIC) Factory.
									result =
											CoCreateInstance(CLSID_WICImagingFactory2, NULL, CLSCTX_INPROC_SERVER,
													IID_PPV_ARGS(&mWICImagingFactoryComPtr));
									AssertFailIf(FAILED(result));

									// Create device independent resources
									ComPtr<IDWriteTextFormat> textFormat;
									result =
											mDWriteFactoryComPtr->CreateTextFormat(L"Segoe UI", NULL,
													DWRITE_FONT_WEIGHT_LIGHT, DWRITE_FONT_STYLE_NORMAL,
													DWRITE_FONT_STRETCH_NORMAL, 32.0f, L"en-US", &textFormat);
									AssertFailIf(FAILED(result));

									AssertFailIf(FAILED(textFormat.As(&m_textFormat)));

									result = m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_NEAR);
									AssertFailIf(FAILED(result));

									AssertFailIf(FAILED(mD2DFactoryComPtr->CreateDrawingStateBlock(&m_stateBlock)));
								}
		void				createDeviceResources()
								{
									// Setup
									HRESULT	result;

									// This flag adds support for surfaces with a different color channel ordering
									// than the API default. It is required for compatibility with Direct2D.
									UINT	creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

#if defined(DEBUG)
									result =
											D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_NULL, 0, D3D11_CREATE_DEVICE_DEBUG,
													NULL, 0, D3D11_SDK_VERSION, NULL, NULL, NULL);
									if (SUCCEEDED(result))
										// If the project is in a debug build, enable debugging via SDK Layers with this
										//	flag.
										creationFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

									// This array defines the set of DirectX hardware feature levels this app will
									//	support.
									// Note the ordering should be preserved.
									// Don't forget to declare your application's minimum required feature level in its
									//	description.  All applications are assumed to support 9.1 unless otherwise
									//	stated.
									D3D_FEATURE_LEVEL	featureLevels[] =
																{
																	D3D_FEATURE_LEVEL_12_1,
																	D3D_FEATURE_LEVEL_12_0,
																	D3D_FEATURE_LEVEL_11_1,
																	D3D_FEATURE_LEVEL_11_0,
																	//D3D_FEATURE_LEVEL_10_1,
																	//D3D_FEATURE_LEVEL_10_0,
																	//D3D_FEATURE_LEVEL_9_3,
																	//D3D_FEATURE_LEVEL_9_2,
																	//D3D_FEATURE_LEVEL_9_1,
																};

									// Create the Direct3D 11 API device object and a corresponding context.
									ComPtr<ID3D11Device>		device;
									ComPtr<ID3D11DeviceContext> deviceContext;
									result =
											D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, 0, creationFlags,
													featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION, &device,
													&mD3DFeatureLevel, &deviceContext);

									if (FAILED(result)) {
										// If the initialization fails, fall back to the WARP device.
										// For more information on WARP, see: 
										// https://go.microsoft.com/fwlink/?LinkId=286690
										result =
												D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_WARP, 0, creationFlags,
														featureLevels, ARRAYSIZE(featureLevels), D3D11_SDK_VERSION,
														&device, &mD3DFeatureLevel, &deviceContext);
										AssertFailIf(FAILED(result));
									}

									// Store pointers to the Direct3D 11.3 API device and immediate context.
									AssertFailIf(FAILED(device.As(&mD3DDeviceComPtr)));
									AssertFailIf(FAILED(deviceContext.As(&mD3DDeviceContextComPtr)));

									// Create the Direct2D device object and a corresponding context.
									ComPtr<IDXGIDevice3> dxgiDevice;
									AssertFailIf(FAILED(mD3DDeviceComPtr.As(&dxgiDevice)));

									result = mD2DFactoryComPtr->CreateDevice(dxgiDevice.Get(), &mD2DDeviceComPtr);
									AssertFailIf(FAILED(result));

									result =
											mD2DDeviceComPtr->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE,
													&mD2DDeviceContextComPtr);
									AssertFailIf(FAILED(result));

									result =
											mD2DDeviceContextComPtr->CreateSolidColorBrush(ColorF(ColorF::White),
													&m_whiteBrush);
									AssertFailIf(FAILED(result));

									// Create the Blend State
									D3D11_BLEND_DESC	blendDescription = {0};
									blendDescription.RenderTarget[0].BlendEnable = TRUE;
									blendDescription.RenderTarget[0].RenderTargetWriteMask =
											D3D11_COLOR_WRITE_ENABLE_ALL;
									blendDescription.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
									blendDescription.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
									blendDescription.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
									blendDescription.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_INV_DEST_ALPHA;
									blendDescription.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
									blendDescription.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;

									mD3DDeviceComPtr->CreateBlendState(&blendDescription, &mD3DBlendState);

									// Create the Rasterizer States
								    D3D11_RASTERIZER_DESC	rasterizerDescription =
																	CD3D11_RASTERIZER_DESC(CD3D11_DEFAULT());

									rasterizerDescription.CullMode = D3D11_CULL_NONE;
									mD3DDeviceComPtr->CreateRasterizerState(&rasterizerDescription,
											&mD3DRasterizerState2D);

									rasterizerDescription.CullMode = D3D11_CULL_BACK;
									mD3DDeviceComPtr->CreateRasterizerState(&rasterizerDescription,
											&mD3DRasterizerState3D);
								}
		void				createWindowSizeDependentResources()
								{
									// Setup
									HRESULT	result;

									// Clear the previous window size specific context.
									ID3D11RenderTargetView*	nullViews[] = {NULL};
									mD3DDeviceContextComPtr->OMSetRenderTargets(ARRAYSIZE(nullViews), nullViews, NULL);
									mD3DRenderTargetViewComPtr = nullptr;
									mD2DDeviceContextComPtr->SetTarget(NULL);
									mD2DTargetBitmapComPtr = nullptr;
									mD3DDeptStencilViewComPtr = nullptr;
									mD3DDeviceContextComPtr->Flush1(D3D11_CONTEXT_TYPE_ALL, NULL);

									// Update render target size
									updateRenderTargetSize();

									// The width and height of the swap chain must be based on the window's
									// natively-oriented width and height. If the window is not in the native
									// orientation, the dimensions must be reversed.
									DXGI_MODE_ROTATION	displayRotation = computeDisplayRotation();

									bool	swapDimensions =
													(displayRotation == DXGI_MODE_ROTATION_ROTATE90) ||
															(displayRotation == DXGI_MODE_ROTATION_ROTATE270);
									mD3DRenderTargetSize.Width =
											swapDimensions ? mOutputSize.Height : mOutputSize.Width;
									mD3DRenderTargetSize.Height =
											swapDimensions ? mOutputSize.Width : mOutputSize.Height;

									if (mDXGISwapChainComPtr != NULL) {
										// If the swap chain already exists, resize it.
										result =
												mDXGISwapChainComPtr->ResizeBuffers(2,
														lround(mD3DRenderTargetSize.Width),
														lround(mD3DRenderTargetSize.Height), DXGI_FORMAT_B8G8R8A8_UNORM,
														0);

										if ((result == DXGI_ERROR_DEVICE_REMOVED) ||
												(result == DXGI_ERROR_DEVICE_RESET)) {
											// If the device was removed for any reason, a new device and swap chain
											//	will need to be created.
											handleDeviceLost();

											// Everything is set up now. Do not continue execution of this method.
											//	HandleDeviceLost will reenter this method and correctly set up the new
											//	device.
											return;
										} else
											AssertFailIf(FAILED(result));
									} else {
										// Otherwise, create a new one using the same adapter as the existing Direct3D
										//	device.
										SDirectXDisplaySupportInfo	displaySupportInfo =
																			mProcsInfo.getDisplaySupportInfo();
										DXGI_SCALING				scaling =
																			displaySupportInfo.mAlwaysSupportHighResolutions
																					? DXGI_SCALING_NONE :
																					DXGI_SCALING_STRETCH;
										DXGI_SWAP_CHAIN_DESC1	swapChainDesc = {0};

										swapChainDesc.Width = lround(mD3DRenderTargetSize.Width);		// Match the size of the window.
										swapChainDesc.Height = lround(mD3DRenderTargetSize.Height);
										swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;				// This is the most common swap chain format.
										swapChainDesc.Stereo = false;
										swapChainDesc.SampleDesc.Count = 1;								// Don't use multi-sampling.
										swapChainDesc.SampleDesc.Quality = 0;
										swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
										swapChainDesc.BufferCount = 2;									// Use double-buffering to minimize latency.
										swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;	// All Microsoft Store apps must use this SwapEffect.
										swapChainDesc.Flags = 0;
										swapChainDesc.Scaling = scaling;
										swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;

										// This sequence obtains the DXGI factory that was used to create the Direct3D device above.
										ComPtr<IDXGIDevice3> dxgiDevice;
										AssertFailIf(FAILED(mD3DDeviceComPtr.As(&dxgiDevice)));

										ComPtr<IDXGIAdapter> dxgiAdapter;
										AssertFailIf(FAILED(dxgiDevice->GetAdapter(&dxgiAdapter)));

										ComPtr<IDXGIFactory4> dxgiFactory;
										AssertFailIf(FAILED(dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory))));

										ComPtr<IDXGISwapChain1> swapChain;
										result =
												dxgiFactory->CreateSwapChainForCoreWindow(mD3DDeviceComPtr.Get(),
														reinterpret_cast<IUnknown*>(mCoreWindowAgile.Get()),
														&swapChainDesc, NULL, &swapChain);
										AssertFailIf(FAILED(result));
										AssertFailIf(FAILED(swapChain.As(&mDXGISwapChainComPtr)));

										// Ensure that DXGI does not queue more than one frame at a time. This both
										//	reduces latency and ensures that the application will only render after each
										//	VSync, minimizing power consumption.
										AssertFailIf(FAILED(dxgiDevice->SetMaximumFrameLatency(1)));
									}

									// Set the proper orientation for the swap chain, and generate 2D and
									// 3D matrix transformations for rendering to the rotated swap chain.
									// Note the rotation angle for the 2D and 3D transforms are different.
									// This is due to the difference in coordinate spaces.  Additionally,
									// the 3D matrix is specified explicitly to avoid rounding errors.
									switch (displayRotation) {
										case DXGI_MODE_ROTATION_IDENTITY:
											// No rotation
											mOrientationTransform2D = Matrix3x2F::Identity();
											mOrientationTransform3D =
													XMFLOAT4X4(
																1.0f, 0.0f, 0.0f, 0.0f,
																0.0f, 1.0f, 0.0f, 0.0f,
																0.0f, 0.0f, 1.0f, 0.0f,
																0.0f, 0.0f, 0.0f, 1.0f
															  );
											break;

										case DXGI_MODE_ROTATION_ROTATE90:
											// Rotate 90
											mOrientationTransform2D = 
													Matrix3x2F::Rotation(90.0f) *
													Matrix3x2F::Translation(mLogicalSize.mHeight, 0.0f);
											mOrientationTransform3D =
													XMFLOAT4X4(
																0.0f, -1.0f, 0.0f, 0.0f,
																1.0f, 0.0f, 0.0f, 0.0f,
																0.0f, 0.0f, 1.0f, 0.0f,
																0.0f, 0.0f, 0.0f, 1.0f
															  );
											break;

										case DXGI_MODE_ROTATION_ROTATE180:
											// Rotate 180
											mOrientationTransform2D = 
													Matrix3x2F::Rotation(180.0f) *
													Matrix3x2F::Translation(mLogicalSize.mWidth, mLogicalSize.mHeight);
											mOrientationTransform3D =
													XMFLOAT4X4(
																-1.0f, 0.0f, 0.0f, 0.0f,
																0.0f, -1.0f, 0.0f, 0.0f,
																0.0f, 0.0f, 1.0f, 0.0f,
																0.0f, 0.0f, 0.0f, 1.0f
															  );
											break;

										case DXGI_MODE_ROTATION_ROTATE270:
											// Rotate 270
											mOrientationTransform2D = 
													Matrix3x2F::Rotation(270.0f) *
													Matrix3x2F::Translation(0.0f, mLogicalSize.mWidth);
											mOrientationTransform3D =
													XMFLOAT4X4(
																0.0f, 1.0f, 0.0f, 0.0f,
																-1.0f, 0.0f, 0.0f, 0.0f,
																0.0f, 0.0f, 1.0f, 0.0f,
																0.0f, 0.0f, 0.0f, 1.0f
															  );
											break;

										default:
											// Unspecified
											throw ref new FailureException();
									}

									AssertFailIf(FAILED(mDXGISwapChainComPtr->SetRotation(displayRotation)));

									// Create a render target view of the swap chain back buffer.
									ComPtr<ID3D11Texture2D1> backBuffer;
									AssertFailIf(FAILED(mDXGISwapChainComPtr->GetBuffer(0, IID_PPV_ARGS(&backBuffer))));

									result =
											mD3DDeviceComPtr->CreateRenderTargetView1(backBuffer.Get(), NULL,
													&mD3DRenderTargetViewComPtr);
									AssertFailIf(FAILED(result));

									// Create a depth stencil view for use with 3D rendering if needed.
									CD3D11_TEXTURE2D_DESC1	depthStencilDesc(DXGI_FORMAT_D24_UNORM_S8_UINT,
																	lround(mD3DRenderTargetSize.Width),
																	lround(mD3DRenderTargetSize.Height), 1, 1,
																	D3D11_BIND_DEPTH_STENCIL);

									ComPtr<ID3D11Texture2D1> depthStencil;
									result =
											mD3DDeviceComPtr->CreateTexture2D1(&depthStencilDesc, NULL, &depthStencil);
									AssertFailIf(FAILED(result));

									CD3D11_DEPTH_STENCIL_VIEW_DESC	depthStencilViewDesc(D3D11_DSV_DIMENSION_TEXTURE2D);
									result =
											mD3DDeviceComPtr->CreateDepthStencilView(depthStencil.Get(),
													&depthStencilViewDesc, &mD3DDeptStencilViewComPtr);
									AssertFailIf(FAILED(result));
	
									// Set the 3D rendering viewport to target the entire window.
									mD3DScreenViewport =
											CD3D11_VIEWPORT(0.0f, 0.0f, mD3DRenderTargetSize.Width,
													mD3DRenderTargetSize.Height);
									mD3DDeviceContextComPtr->RSSetViewports(1, &mD3DScreenViewport);

									// Create a Direct2D target bitmap associated with the
									// swap chain back buffer and set it as the current target.
									D2D1_BITMAP_PROPERTIES1	bitmapProperties = 
											D2D1::BitmapProperties1(
													D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
													D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM,
															D2D1_ALPHA_MODE_PREMULTIPLIED),
													mDPI, mDPI);

									ComPtr<IDXGISurface2> dxgiBackBuffer;
									AssertFailIf(FAILED(mDXGISwapChainComPtr->GetBuffer(0,
											IID_PPV_ARGS(&dxgiBackBuffer))));

									result =
											mD2DDeviceContextComPtr->CreateBitmapFromDxgiSurface(dxgiBackBuffer.Get(),
													&bitmapProperties, &mD2DTargetBitmapComPtr);
									AssertFailIf(FAILED(result));

									mD2DDeviceContextComPtr->SetTarget(mD2DTargetBitmapComPtr.Get());
									mD2DDeviceContextComPtr->SetDpi(mRenderDPI, mRenderDPI);

									// Grayscale text anti-aliasing is recommended for all Microsoft Store apps.
									mD2DDeviceContextComPtr->SetTextAntialiasMode(D2D1_TEXT_ANTIALIAS_MODE_GRAYSCALE);
								}
		void				updateRenderTargetSize()
								{
									// Calculate render DPI
									mRenderDPI =
											mProcsInfo.getDisplaySupportInfo()
													.getRenderDPI(mDPI,
															S2DSizeF32(mLogicalSize.mWidth, mLogicalSize.mHeight));

									// Calculate the necessary render target size in pixels.
									mOutputSize.Width =
											SDirectXDisplaySupportInfo::pixelsFromDIPS(mLogicalSize.mWidth, mRenderDPI);
									mOutputSize.Height =
											SDirectXDisplaySupportInfo::pixelsFromDIPS(mLogicalSize.mHeight, mRenderDPI);

									// Prevent zero size DirectX content from being created.
									mOutputSize.Width = max(mOutputSize.Width, 1.0f);
									mOutputSize.Height = max(mOutputSize.Height, 1.0f);
								}
		DXGI_MODE_ROTATION	computeDisplayRotation()
								{
									// Note: NativeOrientation can only be Landscape or Portrait even though the
									//	DisplayOrientations enum has other values.
									switch (mNativeOrientation) {
										case DisplayOrientations::Landscape:
											// Landscape
											switch (mCurrentOrientation) {
												case DisplayOrientations::Landscape:
													// Landscape
													return DXGI_MODE_ROTATION_IDENTITY;

												case DisplayOrientations::Portrait:
													// Portrait
													return DXGI_MODE_ROTATION_ROTATE270;

												case DisplayOrientations::LandscapeFlipped:
													// Landscape flipped
													return DXGI_MODE_ROTATION_ROTATE180;

												case DisplayOrientations::PortraitFlipped:
													// Portrait flipped
													return DXGI_MODE_ROTATION_ROTATE90;
											}
											break;

										case DisplayOrientations::Portrait:
											// Portrait
											switch (mCurrentOrientation) {
												case DisplayOrientations::Landscape:
													// Landscape
													return DXGI_MODE_ROTATION_ROTATE90;

												case DisplayOrientations::Portrait:
													// Portrait
													return DXGI_MODE_ROTATION_IDENTITY;

												case DisplayOrientations::LandscapeFlipped:
													// Landscape flipped
													return DXGI_MODE_ROTATION_ROTATE270;

												case DisplayOrientations::PortraitFlipped:
													// Portrait flipped
													return DXGI_MODE_ROTATION_ROTATE180;
											}
											break;
									}

									return DXGI_MODE_ROTATION_UNSPECIFIED;
								}
		void				handleDeviceLost()
								{
									// Reset
									mDXGISwapChainComPtr = nullptr;

									// Notify on device lost
									//m_whiteBrush.Reset();

									createDeviceResources();
									mD2DDeviceContextComPtr->SetDpi(mDPI, mDPI);
									createWindowSizeDependentResources();
								}

		void				validateDevice()
								{
									// The D3D Device is no longer valid if the default adapter changed since the device
									// was created or if the device has been removed.

									// First, get the information for the default adapter from when the device was created.
									ComPtr<IDXGIDevice3> dxgiDevice;
									AssertFailIf(FAILED(mD3DDeviceComPtr.As(&dxgiDevice)));

									ComPtr<IDXGIAdapter> deviceAdapter;
									AssertFailIf(FAILED(dxgiDevice->GetAdapter(&deviceAdapter)));

									ComPtr<IDXGIFactory4> deviceFactory;
									AssertFailIf(FAILED(deviceAdapter->GetParent(IID_PPV_ARGS(&deviceFactory))));

									ComPtr<IDXGIAdapter1> previousDefaultAdapter;
									AssertFailIf(FAILED(deviceFactory->EnumAdapters1(0, &previousDefaultAdapter)));

									DXGI_ADAPTER_DESC1 previousDesc;
									AssertFailIf(FAILED(previousDefaultAdapter->GetDesc1(&previousDesc)));

									// Next, get the information for the current default adapter.
									ComPtr<IDXGIFactory4> currentFactory;
									AssertFailIf(FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&currentFactory))));

									ComPtr<IDXGIAdapter1> currentDefaultAdapter;
									AssertFailIf(FAILED(currentFactory->EnumAdapters1(0, &currentDefaultAdapter)));

									DXGI_ADAPTER_DESC1 currentDesc;
									AssertFailIf(FAILED(currentDefaultAdapter->GetDesc1(&currentDesc)));

									// If the adapter LUIDs don't match, or if the device reports that it has been removed,
									// a new D3D device must be created.
									if (previousDesc.AdapterLuid.LowPart != currentDesc.AdapterLuid.LowPart ||
											previousDesc.AdapterLuid.HighPart != currentDesc.AdapterLuid.HighPart ||
											FAILED(mD3DDeviceComPtr->GetDeviceRemovedReason())) {
										// Release references to resources related to the old device.
										dxgiDevice = nullptr;
										deviceAdapter = nullptr;
										deviceFactory = nullptr;
										previousDefaultAdapter = nullptr;

										// Create a new device and swap chain.
										handleDeviceLost();
									}
								}
		void				present() 
								{
									// The first argument instructs DXGI to block until VSync, putting the application
									// to sleep until the next VSync. This ensures we don't waste any cycles rendering
									// frames that will never be displayed to the screen.
									DXGI_PRESENT_PARAMETERS	parameters = {0};
									HRESULT					hr = mDXGISwapChainComPtr->Present1(1, 0, &parameters);

									// Discard the contents of the render target.
									// This is a valid operation only when the existing contents will be entirely
									// overwritten. If dirty or scroll rects are used, this call should be removed.
									mD3DDeviceContextComPtr->DiscardView1(mD3DRenderTargetViewComPtr.Get(), NULL, 0);

									// Discard the contents of the depth stencil.
									mD3DDeviceContextComPtr->DiscardView1(mD3DDeptStencilViewComPtr.Get(), NULL, 0);

									// If the device was removed either by a disconnection or a driver upgrade, we 
									// must recreate all device resources.
									if (hr == DXGI_ERROR_DEVICE_REMOVED || hr == DXGI_ERROR_DEVICE_RESET)
										handleDeviceLost();
									else
										AssertFailIf(FAILED(hr));
								}

		SGPUProcsInfo									mProcsInfo;

		// Direct3D objects.
		ComPtr<ID3D11Device3>							mD3DDeviceComPtr;
		ComPtr<ID3D11DeviceContext3>					mD3DDeviceContextComPtr;
		CLock											mD3DDeviceContextLock;
		ComPtr<IDXGISwapChain3>							mDXGISwapChainComPtr;

		// Direct3D rendering objects. Required for 3D.
		ComPtr<ID3D11RenderTargetView1>					mD3DRenderTargetViewComPtr;
		ComPtr<ID3D11DepthStencilView>					mD3DDeptStencilViewComPtr;
		D3D11_VIEWPORT									mD3DScreenViewport;
		XMFLOAT4X4										mOrientationTransform3D;

		// Direct2D drawing components.
		ComPtr<ID2D1Factory3>							mD2DFactoryComPtr;
		ComPtr<ID2D1Device2>							mD2DDeviceComPtr;
		ComPtr<ID2D1DeviceContext2>						mD2DDeviceContextComPtr;
		ComPtr<ID2D1Bitmap1>							mD2DTargetBitmapComPtr;
		Matrix3x2F										mOrientationTransform2D;

		// DirectWrite drawing components.
		ComPtr<IDWriteFactory3>							mDWriteFactoryComPtr;
		ComPtr<IWICImagingFactory2>						mWICImagingFactoryComPtr;

		// Cached reference to the Window.
		Agile<CoreWindow>								mCoreWindowAgile;

		// Cached device properties.
		D3D_FEATURE_LEVEL								mD3DFeatureLevel;
		Windows::Foundation::Size						mD3DRenderTargetSize;
		Windows::Foundation::Size						mOutputSize;
		S2DSizeF32										mLogicalSize;
		DisplayOrientations								mNativeOrientation;
		DisplayOrientations								mCurrentOrientation;
		Float32											mDPI;

		// This is the DPI that will be reported back to the app. It takes into account whether the app supports high
		//	resolution screens or not.
		Float32											mRenderDPI;

		// Matrices
		XMFLOAT4X4										mViewMatrix2D;
		XMFLOAT4X4										mProjectionMatrix2D;
		XMFLOAT4X4										mViewMatrix3D;
		XMFLOAT4X4										mProjectionMatrix3D;

		// Blend state
		ID3D11BlendState*								mD3DBlendState;

		// Rasterizer State
		ID3D11RasterizerState*							mD3DRasterizerState2D;
		ID3D11RasterizerState*							mD3DRasterizerState3D;

		// Other stuffs
		std::wstring									m_text;
		DWRITE_TEXT_METRICS								m_textMetrics;
		ComPtr<ID2D1SolidColorBrush>					m_whiteBrush;
		ComPtr<ID2D1DrawingStateBlock1>					m_stateBlock;
		ComPtr<IDWriteTextLayout3>						m_textLayout;
		ComPtr<IDWriteTextFormat2>						m_textFormat;
};

//----------------------------------------------------------------------------------------------------------------------
//----------------------------------------------------------------------------------------------------------------------
// MARK: - CGPU

// MARK: Lifecycle methods

//----------------------------------------------------------------------------------------------------------------------
CGPU::CGPU(const SGPUProcsInfo& procsInfo)
//----------------------------------------------------------------------------------------------------------------------
{
	mInternals = new CGPUInternals(procsInfo);
}

//----------------------------------------------------------------------------------------------------------------------
CGPU::~CGPU()
//----------------------------------------------------------------------------------------------------------------------
{
	Delete(mInternals);
}

// MARK: CGPU methods

//----------------------------------------------------------------------------------------------------------------------
CVideoFrame::Compatibility CGPU::getVideoFrameCompatibility() const
//----------------------------------------------------------------------------------------------------------------------
{
	return CVideoFrame::kCompatibilityNotApplicable;
}

//----------------------------------------------------------------------------------------------------------------------
I<CGPUTexture> CGPU::registerTexture(const CData& data, CGPUTexture::DataFormat dataFormat, const S2DSizeU16& size)
//----------------------------------------------------------------------------------------------------------------------
{
	// Register texture
	mInternals->mD3DDeviceContextLock.lock();
	CGPUTexture*	gpuTexture =
							new CDirectXTexture(*mInternals->mD3DDeviceComPtr.Get(),
									*mInternals->mD3DDeviceContextComPtr.Get(), data, DXGI_FORMAT_R8G8B8A8_UNORM, size);
	mInternals->mD3DDeviceContextLock.unlock();

	return I<CGPUTexture>(gpuTexture);
}

//----------------------------------------------------------------------------------------------------------------------
TArray<I<CGPUTexture> > CGPU::registerTextures(const CVideoFrame& videoFrame)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	IMFSample*	sample = videoFrame.getSample();

	IMFMediaBuffer*	mediaBuffer;
	sample->GetBufferByIndex(0, &mediaBuffer);

	BYTE*	buffer;
	mediaBuffer->Lock(&buffer, NULL, NULL);

	DWORD	currentLength;
	mediaBuffer->GetCurrentLength(&currentLength);

	// Register texture
	mInternals->mD3DDeviceContextLock.lock();
	CGPUTexture*	gpuTexture =
							new CDirectXTexture(*mInternals->mD3DDeviceComPtr.Get(),
									*mInternals->mD3DDeviceContextComPtr.Get(), CData(buffer, currentLength, false),
									DXGI_FORMAT_NV12, videoFrame.getFrameSize());
	mediaBuffer->Unlock();
	mInternals->mD3DDeviceContextLock.unlock();

	// Cleanup
	mediaBuffer->Release();

	return TNArray<I<CGPUTexture> >(I<CGPUTexture>(gpuTexture));
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::unregisterTexture(I<CGPUTexture>& gpuTexture)
//----------------------------------------------------------------------------------------------------------------------
{
}

//----------------------------------------------------------------------------------------------------------------------
SGPUVertexBuffer CGPU::allocateVertexBuffer(UInt32 perVertexByteCount, const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CD3D11_BUFFER_DESC		bufferDesc((UINT) data.getByteCount(), D3D11_BIND_VERTEX_BUFFER);
	D3D11_SUBRESOURCE_DATA	subresourceData = {data.getBytePtr(), 0, 0};
	ID3D11Buffer*			d3dBuffer = NULL;
	HRESULT					result =
									mInternals->mD3DDeviceComPtr->CreateBuffer(&bufferDesc, &subresourceData,
											&d3dBuffer);
	AssertFailIf(result != S_OK);

	return SGPUVertexBuffer(perVertexByteCount, d3dBuffer);
}

//----------------------------------------------------------------------------------------------------------------------
SGPUBuffer CGPU::allocateIndexBuffer(const CData& data)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	CD3D11_BUFFER_DESC		bufferDesc((UINT) data.getByteCount(), D3D11_BIND_INDEX_BUFFER);
	D3D11_SUBRESOURCE_DATA	subresourceData = {data.getBytePtr(), 0, 0};
	ID3D11Buffer*			d3dBuffer = NULL;
	HRESULT					result =
									mInternals->mD3DDeviceComPtr->CreateBuffer(&bufferDesc, &subresourceData,
											&d3dBuffer);
	AssertFailIf(result != S_OK);

	return SGPUBuffer(d3dBuffer);
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::disposeBuffer(const SGPUBuffer& buffer)
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	ID3D11Buffer*	d3dBuffer = (ID3D11Buffer*) buffer.mPlatformReference;

	// Cleanup
	d3dBuffer->Release();
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::renderStart(const S2DSizeF32& size2D, Float32 fieldOfViewAngle3D, Float32 aspectRatio3D, Float32 nearZ3D,
		Float32 farZ3D, const S3DPointF32& camera3D, const S3DPointF32& target3D, const S3DVectorF32& up3D) const
//----------------------------------------------------------------------------------------------------------------------
{
	// Setup
	mInternals->mD3DDeviceContextLock.lock();
	if (mInternals->mProcsInfo.requiresDeviceValidation()) {
		// Requires device validation
		mInternals->validateDevice();
		mInternals->mProcsInfo.handledDeviceValidation();
	}

	bool				createWindowSizeDependentResources = false;
	Float32				dpi = mInternals->mProcsInfo.getDPI();
	DisplayOrientations	orientation = mInternals->mProcsInfo.getOrientation();
	if (mInternals->mCoreWindowAgile.Get() == nullptr) {
		// Setup
		DisplayInformation^ currentDisplayInformation = DisplayInformation::GetForCurrentView();

		mInternals->mCoreWindowAgile = mInternals->mProcsInfo.getCoreWindow();
		mInternals->mLogicalSize =
				S2DSizeF32(mInternals->mCoreWindowAgile->Bounds.Width, mInternals->mCoreWindowAgile->Bounds.Height);
		mInternals->mNativeOrientation = currentDisplayInformation->NativeOrientation;
		mInternals->mCurrentOrientation = currentDisplayInformation->CurrentOrientation;
		mInternals->mDPI = currentDisplayInformation->LogicalDpi;
		mInternals->mD2DDeviceContextComPtr->SetDpi(mInternals->mDPI, mInternals->mDPI);

		createWindowSizeDependentResources = true;
	} else if ((dpi != 0.0) && (dpi != mInternals->mDPI)) {
		// Update DPI
		mInternals->mDPI = dpi;

		// When the display DPI changes, the logical size of the window (measured in Dips) also changes and needs to be updated.
		mInternals->mLogicalSize =
				S2DSizeF32(mInternals->mCoreWindowAgile->Bounds.Width, mInternals->mCoreWindowAgile->Bounds.Height);

		// Update device context
		mInternals->mD2DDeviceContextComPtr->SetDpi(mInternals->mDPI, mInternals->mDPI);

		createWindowSizeDependentResources = true;
	} else if (mInternals->mProcsInfo.getSize() != mInternals->mLogicalSize) {
		// Update size
		mInternals->mLogicalSize = mInternals->mProcsInfo.getSize();
		createWindowSizeDependentResources = true;
	} else if ((orientation != DisplayOrientations::None) && (orientation != mInternals->mCurrentOrientation)) {
		// Update orientation
		mInternals->mCurrentOrientation = orientation;
		createWindowSizeDependentResources = true;
	}

	if (createWindowSizeDependentResources)
		// Create window size dependent resources
		mInternals->createWindowSizeDependentResources();

	// Reset the viewport to target the whole screen.
	mInternals->mD3DDeviceContextComPtr->RSSetViewports(1, &mInternals->mD3DScreenViewport);

	// Reset render targets to the screen.
	ID3D11RenderTargetView *const targets[1] = { mInternals->mD3DRenderTargetViewComPtr.Get() };
	mInternals->mD3DDeviceContextComPtr->OMSetRenderTargets(1, targets, mInternals->mD3DDeptStencilViewComPtr.Get());

	// Clear the back buffer and depth stencil view.
	mInternals->mD3DDeviceContextComPtr->ClearRenderTargetView(mInternals->mD3DRenderTargetViewComPtr.Get(),
			DirectX::Colors::CornflowerBlue);
	mInternals->mD3DDeviceContextComPtr->ClearDepthStencilView(mInternals->mD3DDeptStencilViewComPtr.Get(),
			D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
	mInternals->mD3DDeviceContextLock.unlock();

	// Setup matrices
	XMStoreFloat4x4(&mInternals->mViewMatrix2D, DirectX::XMMatrixIdentity());
	mInternals->mProjectionMatrix2D =
			XMFLOAT4X4(
					2.0f / size2D.mWidth, 0.0, 0.0, -1.0,
					0.0, -2.0f / size2D.mHeight, 0.0, 1.0,
					0.0, 0.0, 0.5, 0.0,
					0.0, 0.0, 0.0, 1.0);

	XMVECTORF32	cameraVector = {camera3D.mX, camera3D.mY, camera3D.mZ, 0.0f};
	XMVECTORF32	targetVector = {target3D.mX, target3D.mY, target3D.mZ, 0.0f};
	XMVECTORF32	upVector = {up3D.mDX, up3D.mDY, up3D.mDZ, 0.0f};
	XMStoreFloat4x4(&mInternals->mViewMatrix3D,
			XMMatrixTranspose(XMMatrixLookAtRH(cameraVector, targetVector, upVector)));

	// Note that the OrientationTransform3D matrix is post-multiplied here
	// in order to correctly orient the scene to match the display orientation.
	// This post-multiplication step is required for any draw calls that are
	// made to the swap chain render target. For draw calls to other targets,
	// this transform should not be applied.
	XMMATRIX	perspectiveMatrix = XMMatrixPerspectiveFovRH(fieldOfViewAngle3D, aspectRatio3D, nearZ3D, farZ3D);
	XMMATRIX	orientationMatrix = XMLoadFloat4x4(&mInternals->mOrientationTransform3D);
	XMStoreFloat4x4(&mInternals->mProjectionMatrix3D, XMMatrixTranspose(perspectiveMatrix * orientationMatrix));
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::render(CGPURenderState& renderState, RenderType renderType, UInt32 count, UInt32 offset)
//----------------------------------------------------------------------------------------------------------------------
{
	// Reset render targets to the screen.
	ID3D11RenderTargetView *const targets[1] = { mInternals->mD3DRenderTargetViewComPtr.Get() };

	// Finalize render state
	switch (renderState.getMode()) {
		case CGPURenderState::kMode2D:
			// 2D
			mInternals->mD3DDeviceContextComPtr->RSSetState(mInternals->mD3DRasterizerState2D);
			mInternals->mD3DDeviceContextComPtr->OMSetRenderTargets(1, targets, NULL);
			renderState.commit(
					SGPURenderStateCommitInfo(*mInternals->mD3DDeviceComPtr.Get(),
							*mInternals->mD3DDeviceContextComPtr.Get(), *mInternals->mD3DBlendState,
							mInternals->mProjectionMatrix2D, mInternals->mViewMatrix2D));
			break;

		case CGPURenderState::kMode3D:
			// 3D
			mInternals->mD3DDeviceContextComPtr->RSSetState(mInternals->mD3DRasterizerState3D);
			mInternals->mD3DDeviceContextComPtr->OMSetRenderTargets(1, targets,
					mInternals->mD3DDeptStencilViewComPtr.Get());
			renderState.commit(
					SGPURenderStateCommitInfo(*mInternals->mD3DDeviceComPtr.Get(),
							*mInternals->mD3DDeviceContextComPtr.Get(), *mInternals->mD3DBlendState,
							mInternals->mProjectionMatrix3D, mInternals->mViewMatrix3D));
			break;
	}

	// Check type
	switch (renderType) {
		case kRenderTypeTriangleList:
			// Triangle list
			mInternals->mD3DDeviceContextComPtr->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			break;

		case kRenderTypeTriangleStrip:
			// Triangle strip
			mInternals->mD3DDeviceContextComPtr->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			break;
	}

	// Draw
	mInternals->mD3DDeviceContextComPtr->Draw(count, offset);
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::renderIndexed(CGPURenderState& renderState, RenderType renderType, UInt32 count, UInt32 offset)
//----------------------------------------------------------------------------------------------------------------------
{
	// Reset render targets to the screen.
	ID3D11RenderTargetView *const targets[1] = { mInternals->mD3DRenderTargetViewComPtr.Get() };

	// Finalize render state
	switch (renderState.getMode()) {
		case CGPURenderState::kMode2D:
			// 2D
			mInternals->mD3DDeviceContextComPtr->RSSetState(mInternals->mD3DRasterizerState2D);
			mInternals->mD3DDeviceContextComPtr->OMSetRenderTargets(1, targets, NULL);
			renderState.commit(
					SGPURenderStateCommitInfo(*mInternals->mD3DDeviceComPtr.Get(),
							*mInternals->mD3DDeviceContextComPtr.Get(), *mInternals->mD3DBlendState,
							mInternals->mProjectionMatrix2D, mInternals->mViewMatrix2D));
			break;

		case CGPURenderState::kMode3D:
			// 3D
			mInternals->mD3DDeviceContextComPtr->RSSetState(mInternals->mD3DRasterizerState3D);
			mInternals->mD3DDeviceContextComPtr->OMSetRenderTargets(1, targets,
					mInternals->mD3DDeptStencilViewComPtr.Get());
			renderState.commit(
					SGPURenderStateCommitInfo(*mInternals->mD3DDeviceComPtr.Get(),
							*mInternals->mD3DDeviceContextComPtr.Get(), *mInternals->mD3DBlendState,
							mInternals->mProjectionMatrix3D, mInternals->mViewMatrix3D));
			break;
	}

	// Check type
	switch (renderType) {
		case kRenderTypeTriangleList:
			// Triangle list
			mInternals->mD3DDeviceContextComPtr->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			break;

		case kRenderTypeTriangleStrip:
			// Triangle strip
			mInternals->mD3DDeviceContextComPtr->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
			break;
	}

	// Draw
	mInternals->mD3DDeviceContextComPtr->DrawIndexed(count, offset, 0);
}

//----------------------------------------------------------------------------------------------------------------------
void CGPU::renderEnd() const
//----------------------------------------------------------------------------------------------------------------------
{
	// Update display text.
	UInt32	fps = mInternals->mProcsInfo.getFPS();

	mInternals->m_text = (fps > 0) ? std::to_wstring(fps) + L" FPS" : L" - FPS";

	ComPtr<IDWriteTextLayout> textLayout;
	AssertFailIf(FAILED(
		mInternals->mDWriteFactoryComPtr->CreateTextLayout(
			mInternals->m_text.c_str(),
			(uint32) mInternals->m_text.length(),
			mInternals->m_textFormat.Get(),
			240.0f, // Max width of the input text.
			50.0f, // Max height of the input text.
			&textLayout
			)
		));

	AssertFailIf(FAILED(
		textLayout.As(&mInternals->m_textLayout)
		));

	AssertFailIf(FAILED(
		mInternals->m_textLayout->GetMetrics(&mInternals->m_textMetrics)
		));

	ID2D1DeviceContext* context = mInternals->mD2DDeviceContextComPtr.Get();

	context->SaveDrawingState(mInternals->m_stateBlock.Get());
	context->BeginDraw();

	// Position on the bottom right corner
	D2D1::Matrix3x2F screenTranslation = D2D1::Matrix3x2F::Translation(
		mInternals->mLogicalSize.mWidth - mInternals->m_textMetrics.layoutWidth,
		mInternals->mLogicalSize.mHeight - mInternals->m_textMetrics.height
		);

	context->SetTransform(screenTranslation * mInternals->mOrientationTransform2D);

	AssertFailIf(FAILED(
		mInternals->m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_TRAILING)
		));

	context->DrawTextLayout(
		D2D1::Point2F(0.f, 0.f),
		mInternals->m_textLayout.Get(),
		mInternals->m_whiteBrush.Get()
		);

	// Ignore D2DERR_RECREATE_TARGET here. This error indicates that the device
	// is lost. It will be handled during the next call to Present.
	HRESULT hr = context->EndDraw();
	if (hr != D2DERR_RECREATE_TARGET)
		AssertFailIf(FAILED(hr));

	context->RestoreDrawingState(mInternals->m_stateBlock.Get());

	// Present
	mInternals->present();
}
