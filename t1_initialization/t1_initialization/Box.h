#pragma once

#include <d3dcompiler.h>
#include <dxgi.h>
#include <d3d11.h>
#include <directxmath.h>
#include <string>
#include <vector>

#include "timer.h"
#include "frustumCulling.h"
#include "Material.h"
#include "D3DInclude.h"
#include "def.h"
#include "Light.h"

using namespace DirectX;

class Box {
public:
  HRESULT Init(ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight, const MaterialParams& params, const std::vector<XMFLOAT4>& positions);

  void Realese();

  void Resize(int screenWidth, int screenHeight) {};

  void Render(ID3D11DeviceContext* context);

  bool Frame(ID3D11DeviceContext* context, XMMATRIX& viewMatrix, XMMATRIX& projectionMatrix, XMFLOAT3& cameraPos, const Light& lights);

private:
  HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);

  // dx11 vars
  ID3D11VertexShader* g_pVertexShader = nullptr;
  ID3D11PixelShader* g_pPixelShader = nullptr;
  ID3D11InputLayout* g_pVertexLayout = nullptr;

  ID3D11Buffer* g_pVertexBuffer = nullptr;
  ID3D11Buffer* g_pIndexBuffer = nullptr;
  ID3D11Buffer* g_pGeomBuffer = nullptr;
  ID3D11Buffer* g_pSceneMatrixBuffer = nullptr;
  ID3D11Buffer* g_LightConstantBuffer = nullptr;
  ID3D11RasterizerState* g_pRasterizerState = nullptr;
  ID3D11SamplerState* g_pSamplerState = nullptr;
  ID3D11DepthStencilState* g_pDepthState = nullptr;

  std::vector<Texture> boxesTextures;
  std::vector<BoxModel> boxesModelVector;
  std::vector<int> boxesIndexies;

  FrustumCulling frustum;
};
