#include "renderer.h"

using namespace DirectX;

Renderer& Renderer::GetInstance() {
  static Renderer rendererInstance;
  return rendererInstance;
}

HRESULT Renderer::InitDevice(const HWND& g_hWnd) {
  HRESULT hr = S_OK;

  RECT rc;
  GetClientRect(g_hWnd, &rc);
  UINT width = rc.right - rc.left;
  UINT height = rc.bottom - rc.top;

  UINT createDeviceFlags = 0;
#ifdef _DEBUG
  createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

  D3D_DRIVER_TYPE driverTypes[] =
  {
      D3D_DRIVER_TYPE_HARDWARE,
      D3D_DRIVER_TYPE_WARP,
      D3D_DRIVER_TYPE_REFERENCE,
  };
  UINT numDriverTypes = ARRAYSIZE(driverTypes);

  D3D_FEATURE_LEVEL featureLevels[] =
  {
      D3D_FEATURE_LEVEL_11_1,
      D3D_FEATURE_LEVEL_11_0,
      D3D_FEATURE_LEVEL_10_1,
      D3D_FEATURE_LEVEL_10_0,
  };
  UINT numFeatureLevels = ARRAYSIZE(featureLevels);

  for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
  {
    g_driverType = driverTypes[driverTypeIndex];
    hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, featureLevels, numFeatureLevels,
      D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);

    if (hr == E_INVALIDARG)
    {
      // DirectX 11.0 platforms will not recognize D3D_FEATURE_LEVEL_11_1 so we need to retry without it
      hr = D3D11CreateDevice(nullptr, g_driverType, nullptr, createDeviceFlags, &featureLevels[1], numFeatureLevels - 1,
        D3D11_SDK_VERSION, &g_pd3dDevice, &g_featureLevel, &g_pImmediateContext);
    }

    if (SUCCEEDED(hr))
      break;
  }
  if (FAILED(hr))
    return hr;

  // Obtain DXGI factory from device (since we used nullptr for pAdapter above)
  IDXGIFactory1* dxgiFactory = nullptr;
  {
    IDXGIDevice* dxgiDevice = nullptr;
    hr = g_pd3dDevice->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void**>(&dxgiDevice));
    if (SUCCEEDED(hr))
    {
      IDXGIAdapter* adapter = nullptr;
      hr = dxgiDevice->GetAdapter(&adapter);
      if (SUCCEEDED(hr))
      {
        hr = adapter->GetParent(__uuidof(IDXGIFactory1), reinterpret_cast<void**>(&dxgiFactory));
        adapter->Release();
      }
      dxgiDevice->Release();
    }
  }
  if (FAILED(hr))
    return hr;

  // Create swap chain
  IDXGIFactory2* dxgiFactory2 = nullptr;
  hr = dxgiFactory->QueryInterface(__uuidof(IDXGIFactory2), reinterpret_cast<void**>(&dxgiFactory2));
  if (dxgiFactory2)
  {
    // DirectX 11.1 or later
    hr = g_pd3dDevice->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&g_pd3dDevice1));
    if (SUCCEEDED(hr))
    {
      (void)g_pImmediateContext->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&g_pImmediateContext1));
    }

    DXGI_SWAP_CHAIN_DESC1 sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.Width = width;
    sd.Height = height;
    sd.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    sd.BufferCount = 2;

    hr = dxgiFactory2->CreateSwapChainForHwnd(g_pd3dDevice, g_hWnd, &sd, nullptr, nullptr, &g_pSwapChain1);
    if (SUCCEEDED(hr))
    {
      hr = g_pSwapChain1->QueryInterface(__uuidof(IDXGISwapChain), reinterpret_cast<void**>(&g_pSwapChain));
    }

    dxgiFactory2->Release();
  }
  else
  {
    // DirectX 11.0 systems
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = g_hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
    sd.Windowed = TRUE;

    hr = dxgiFactory->CreateSwapChain(g_pd3dDevice, &sd, &g_pSwapChain);
  }

  // Note this tutorial doesn't handle full-screen swapchains so we block the ALT+ENTER shortcut
  dxgiFactory->MakeWindowAssociation(g_hWnd, DXGI_MWA_NO_ALT_ENTER);

  dxgiFactory->Release();

  if (FAILED(hr))
    return hr;

  // Setup the viewport
  D3D11_VIEWPORT vp;
  vp.Width = (FLOAT)width;
  vp.Height = (FLOAT)height;
  vp.MinDepth = 0.0f;
  vp.MaxDepth = 1.0f;
  vp.TopLeftX = 0;
  vp.TopLeftY = 0;
  g_pImmediateContext->RSSetViewports(1, &vp);

  // Init back buffer
  hr = InitBackBuffer();
  if (FAILED(hr))
    return hr;

  // init skybox and scene
  sc.Init(g_pd3dDevice, g_pImmediateContext, width, height);
  sb.Init(g_pd3dDevice, g_pImmediateContext, width, height);

  return S_OK;
}

HRESULT Renderer::InitBackBuffer() {
  // Create a render target view
  ID3D11Texture2D* pBackBuffer = NULL;
  HRESULT hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
  if (FAILED(hr))
    return hr;

  hr = g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_pRenderTargetView);
  if (pBackBuffer) pBackBuffer->Release();
  if (FAILED(hr))
    return hr;

  g_pImmediateContext->OMSetRenderTargets(1, &g_pRenderTargetView, nullptr);

  if (g_pDepthBuffer) g_pDepthBuffer->Release();
  if (g_pDepthBufferDSV) g_pDepthBufferDSV->Release();
  
  D3D11_TEXTURE2D_DESC desc = {};
  desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
  desc.ArraySize = 1;
  desc.MipLevels = 1;
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.Height = input.GetHeight();
  desc.Width = input.GetWidth();
  desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
  desc.CPUAccessFlags = 0;
  desc.MiscFlags = 0;
  desc.SampleDesc.Count = 1;
  desc.SampleDesc.Quality = 0;

  hr = g_pd3dDevice->CreateTexture2D(&desc, NULL, &g_pDepthBuffer);
  if (FAILED(hr))
    return hr;

  hr = g_pd3dDevice->CreateDepthStencilView(g_pDepthBuffer, NULL, &g_pDepthBufferDSV);
  return hr;
}

HRESULT Renderer::Init(const HWND& g_hWnd, const HINSTANCE& g_hInstance, UINT screenWidth, UINT screenHeight) {
  HRESULT hr = input.InitInputs(g_hInstance, g_hWnd, screenWidth, screenHeight);
  if (FAILED(hr))
    return hr;

  hr = camera.InitCamera();
  if (FAILED(hr))
    return hr;

  hr = InitDevice(g_hWnd);
  if (FAILED(hr))
    return hr;

  return S_OK;
}

void Renderer::HandleInput() {
  // handle camera rotations
  XMFLOAT3 mouseMove = input.IsMouseUsed();
  camera.Move(mouseMove.x, mouseMove.y, mouseMove.z);

  // handle world matrix rotation
  //float sign = input.IsPlusMinusPressed();
  //angle_velocity += sign * 0.0001f;
  //angle_velocity = min(max(angle_velocity, 0.f), 30.f);
}

// Update frame method
bool Renderer::Frame() {
  // update inputs
  input.Frame();
  
  // update camera
  HandleInput();
  camera.Frame();

  // Get the view matrix
  XMMATRIX mView;
  camera.GetBaseViewMatrix(mView);
  // Get the projection matrix
  XMMATRIX mProjection = XMMatrixPerspectiveFovLH(XM_PIDIV2, (FLOAT)input.GetWidth() / (FLOAT)input.GetHeight(), 100.0f, 0.01f);
  
  // Get the view matrix
  HRESULT hr = sc.Frame(g_pImmediateContext, mView, mProjection, camera.GetPos());
  if (FAILED(hr))
    return SUCCEEDED(hr);

  hr = sb.Frame(g_pImmediateContext, mView, mProjection, camera.GetPos());
  if (FAILED(hr))
    return SUCCEEDED(hr);

  return SUCCEEDED(hr);
}

void Renderer::Render() {
  g_pImmediateContext->ClearState();

  ID3D11RenderTargetView* views[] = { g_pRenderTargetView };
  g_pImmediateContext->OMSetRenderTargets(1, views, g_pDepthBufferDSV);

  FLOAT BackColor[4] = { 0.1f, 0.1f, 0.1f, 1.0f };
  g_pImmediateContext->ClearRenderTargetView(g_pRenderTargetView, BackColor);
  g_pImmediateContext->ClearDepthStencilView(g_pDepthBufferDSV, D3D11_CLEAR_DEPTH, 0.0f, 0);

  D3D11_VIEWPORT viewport;
  viewport.TopLeftX = 0;
  viewport.TopLeftY = 0;
  viewport.Width = (FLOAT)input.GetWidth();
  viewport.Height = (FLOAT)input.GetHeight();
  viewport.MinDepth = 0.0f;
  viewport.MaxDepth = 1.0f;
  g_pImmediateContext->RSSetViewports(1, &viewport);

  D3D11_RECT rect;
  rect.left = 0;
  rect.top = 0;
  rect.right = input.GetWidth();
  rect.bottom = input.GetHeight();
  g_pImmediateContext->RSSetScissorRects(1, &rect);

  sc.Render(g_pImmediateContext);
  sb.Render(g_pImmediateContext);

  g_pSwapChain->Present(0, 0);
}

void Renderer::CleanupDevice() {
  camera.Realese();
  input.Realese();
  txt.Release();
  sb.Realese();
  sc.Realese();

  if (g_pImmediateContext) g_pImmediateContext->ClearState();

  if (g_pDepthBuffer) g_pDepthBuffer->Release();
  if (g_pDepthBufferDSV) g_pDepthBufferDSV->Release();
  if (g_pRenderTargetView) g_pRenderTargetView->Release();
  if (g_pSwapChain1) g_pSwapChain1->Release();
  if (g_pSwapChain) g_pSwapChain->Release();
  if (g_pImmediateContext1) g_pImmediateContext1->Release();
  if (g_pImmediateContext) g_pImmediateContext->Release();
  if (g_pd3dDevice1) g_pd3dDevice1->Release();
  if (g_pd3dDevice) g_pd3dDevice->Release();
}

void Renderer::ResizeWindow(const HWND& g_hWnd) {
  RECT rc;
  GetClientRect(g_hWnd, &rc);
  UINT width = rc.right - rc.left;
  UINT height = rc.bottom - rc.top;

  if (g_pSwapChain &&
    (width != input.GetWidth() || height != input.GetHeight())) {

    if (g_pRenderTargetView) g_pRenderTargetView->Release();

    HRESULT hr = g_pSwapChain->ResizeBuffers(2, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0);
    assert(SUCCEEDED(hr));
    if (SUCCEEDED(hr)) {
      input.Resize(width, height);
      
      hr = InitBackBuffer();
      input.Resize(width, height);
      sc.Resize(width, height);
      sb.Resize(width, height);
    }
  }
}
