#pragma once

#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>

#include "timer.h"

using namespace DirectX;

struct PostprocessingCB {
  XMFLOAT4 params; // x, y, z - parts of each color channel
};

class Postprocessing {
public:
  // Function to initialize
  HRESULT Init(ID3D11Device* device, HWND hwnd);
  
  // Function to realese
  void Release();
  
  // Render function
  void Render(ID3D11DeviceContext* context, ID3D11ShaderResourceView* sourceTexture, ID3D11RenderTargetView* renderTarget, D3D11_VIEWPORT viewport);

  // Params updating
  bool Frame(ID3D11DeviceContext* context);

private:
  // dx11 variables
  ID3D11VertexShader* g_pVertexShader = nullptr;
  ID3D11PixelShader* g_pPixelShader = nullptr;
  ID3D11SamplerState* g_pSamplerState = nullptr;
  ID3D11Buffer* g_pPostprocessingCB = nullptr;
};