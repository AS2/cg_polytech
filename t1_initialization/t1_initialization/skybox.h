#pragma once

#include <d3dcompiler.h>
#include <dxgi.h>
#include <d3d11.h>
#include <directxmath.h>
#include <string>
#include <vector>

#include "texture.h"

using namespace DirectX;

struct SBVertex
{
  float x, y, z;      // positional coords
};

struct SBWorldMatrixBuffer {
  XMMATRIX worldMatrix;
  XMFLOAT4 size;
};

struct SBSceneMatrixBuffer {
  XMMATRIX viewProjectionMatrix;
  XMFLOAT4 cameraPos;
};

class Skybox {
public:
  HRESULT Init(ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight);
  
  void Realese();
  
  void Resize(int screenWidth, int screenHeight);
  
  void Render(ID3D11DeviceContext* context);

  bool Frame(ID3D11DeviceContext* context, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 cameraPos);

private:
  void GenerateSphere(UINT LatLines, UINT LongLines, std::vector<SBVertex>& vertices, std::vector<UINT>& indices);

  // dx11 vars
  ID3D11Buffer* g_pVertexBuffer = nullptr;
  ID3D11Buffer* g_pIndexBuffer = nullptr;
  ID3D11Buffer* g_pWorldMatrixBuffer = nullptr;
  ID3D11Buffer* g_pSceneMatrixBuffer = nullptr;
  ID3D11RasterizerState* g_pRasterizerState = nullptr;
  ID3D11SamplerState* g_pSamplerState = nullptr;

  ID3D11InputLayout* g_pVertexLayout = nullptr;
  ID3D11VertexShader* g_pVertexShader = nullptr;
  ID3D11PixelShader* g_pPixelShader = nullptr;

  // Texture with skybox
  Texture txt;

  // Sphere (Skybox) geometry params
  UINT numSphereVertices = 0;
  UINT numSphereFaces = 0;
  float radius = 1.0f;
};
