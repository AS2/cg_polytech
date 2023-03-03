#pragma once

#include <d3d11_1.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <directxcolors.h>
#include <directxmath.h>

#include <ctime>

#include "camera.h"
#include "input.h"
#include "texture.h"


struct SimpleVertex
{
  float x, y, z;      // positional coords
  float u, v;         // texture coords
};

struct WorldMatrixBuffer {
  XMMATRIX worldMatrix;
};

struct SceneMatrixBuffer {
  XMMATRIX viewProjectionMatrix;
};

// Make renderer class
class Renderer {
public:
  // Make class singleton
  static Renderer& GetInstance();
  Renderer(const Renderer&) = delete;
  Renderer(Renderer&&) = delete;

  // Init Renderer method
  HRESULT Init(const HWND& g_hWnd, const HINSTANCE& g_hInstance, UINT screenWidth, UINT screenHeight);

  // Update frame method
  bool Frame();

  // Scene render method
  void Render();

  // Device cleaner method
  void CleanupDevice();

  // Window resize method
  void ResizeWindow(const HWND& g_hWnd);

private:
  // Initialization device method
  HRESULT InitDevice(const HWND& g_hWnd);

  HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);

  void HandleInput();

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

  ID3D11VertexShader* g_pVertexShader = nullptr;
  ID3D11PixelShader* g_pPixelShader = nullptr;
  ID3D11InputLayout* g_pVertexLayout = nullptr;

  ID3D11Buffer* g_pVertexBuffer = nullptr;
  ID3D11Buffer* g_pIndexBuffer = nullptr;
  ID3D11Buffer* g_pWorldMatrixBuffer = nullptr;
  ID3D11Buffer* g_pSceneMatrixBuffer = nullptr;
  ID3D11RasterizerState* g_pRasterizerState = nullptr;
  ID3D11SamplerState* g_pSamplerState = nullptr;

  // initialization clock
  std::clock_t init_time;

  // initialization other thinngs (camera, input devices, etc.)
  Camera camera;
  Input input;
  Texture txt;

  // Velocity of world matrix rotation
  float angle_velocity = 3.1415926f;
};