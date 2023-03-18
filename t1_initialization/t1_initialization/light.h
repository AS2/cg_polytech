#pragma once

#include <d3dcompiler.h>
#include <dxgi.h>
#include <d3d11.h>
#include <directxmath.h>
#include <vector>
#include "D3DInclude.h"
#include "def.h"

using namespace DirectX;

class Light {
public:
  HRESULT Init(ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight, XMFLOAT4 color, XMFLOAT4 position);

  void Realese();

  void Resize(int screenWidth, int screenHeight) {};

  void Render(ID3D11DeviceContext* context);
  
  bool Frame(ID3D11DeviceContext* context, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 cameraPos);

  XMFLOAT4 GetColor() { return color; };
  XMFLOAT4 GetPosition() { return position; };
private:
  void GenerateSphere(UINT LatLines, UINT LongLines, std::vector<SimpleVertex>& vertices, std::vector<UINT>& indices);

  // dx11 vars
  ID3D11Buffer* g_pVertexBuffer = nullptr;
  ID3D11Buffer* g_pIndexBuffer = nullptr;
  ID3D11Buffer* g_pWorldMatrixBuffer = nullptr;
  ID3D11Buffer* g_pSceneMatrixBuffer = nullptr;
  ID3D11RasterizerState* g_pRasterizerState = nullptr;

  ID3D11InputLayout* g_pVertexLayout = nullptr;
  ID3D11VertexShader* g_pVertexShader = nullptr;
  ID3D11PixelShader* g_pPixelShader = nullptr;

  // Sphere light geometry params
  UINT numSphereVertices = 0;
  UINT numSphereFaces = 0;
  float radius = 0.1f;

  XMFLOAT4 color;
  XMFLOAT4 position;
};
