#pragma once

#include <d3d11_1.h>
#include <directxcolors.h>

#include <ctime>

// Make renderer class
class Renderer {
public:
  // Make class singleton
  static Renderer& GetInstance();
  Renderer(const Renderer&) = delete;
  Renderer(Renderer&&) = delete;

  // Initialization device method
  HRESULT InitDevice(const HWND& g_hWnd);

  // Scene render method
  void Render();

  // Device cleaner method
  void CleanupDevice();

  // Window resize method
  void ResizeWindow(const HWND& g_hWnd);

private:
  // Private constructor (for singleton)
  Renderer() = default;

  // DirectX11 variables
  D3D_DRIVER_TYPE         g_driverType = D3D_DRIVER_TYPE_NULL;
  D3D_FEATURE_LEVEL       g_featureLevel = D3D_FEATURE_LEVEL_11_0;
  ID3D11Device*           g_pd3dDevice = nullptr;
  ID3D11Device1*          g_pd3dDevice1 = nullptr;
  ID3D11DeviceContext*    g_pImmediateContext = nullptr;
  ID3D11DeviceContext1*   g_pImmediateContext1 = nullptr;
  IDXGISwapChain*         g_pSwapChain = nullptr;
  IDXGISwapChain1*        g_pSwapChain1 = nullptr;
  ID3D11RenderTargetView* g_pRenderTargetView = nullptr;

  // initialization clock
  std::clock_t init_time;
};