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

  int GetCulledCount() { return MAX_CUBES - cubesDrawedOnGPU; };
private:
  HRESULT InitQuery(ID3D11Device* device);
  void ReadQueries(ID3D11DeviceContext* context);

  HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);

  // dx11 vars
  ID3D11VertexShader* g_pVertexShader = nullptr;
  ID3D11PixelShader* g_pPixelShader = nullptr;
  ID3D11InputLayout* g_pVertexLayout = nullptr;
  ID3D11ComputeShader* g_pCullShader = nullptr;

  ID3D11Buffer* g_pVertexBuffer = nullptr;
  ID3D11Buffer* g_pIndexBuffer = nullptr;
  ID3D11Buffer* g_pGeomBuffer = nullptr;
  ID3D11Buffer* g_pCullParams = nullptr;
  ID3D11Buffer* g_pSceneMatrixBuffer = nullptr;
  ID3D11Buffer* g_LightConstantBuffer = nullptr;
  ID3D11RasterizerState* g_pRasterizerState = nullptr;
  ID3D11SamplerState* g_pSamplerState = nullptr;
  ID3D11DepthStencilState* g_pDepthState = nullptr;

  ID3D11Buffer* g_pInderectArgsSrc = nullptr;
  ID3D11Buffer* g_pInderectArgs = nullptr;
  ID3D11UnorderedAccessView* g_pInderectArgsUAV = nullptr;
  ID3D11Buffer* g_pGeomBufferInstVis = nullptr;
  ID3D11Buffer* g_pGeomBufferInstVisGpu = nullptr;
  ID3D11UnorderedAccessView* g_pGeomBufferInstVisGpu_UAV = nullptr;

  std::vector<Texture> boxesTextures;
  std::vector<BoxModel> boxesModelVector;
  std::vector<int> boxesIndexies;

  FrustumCulling frustum;
  
  int cubesDrawedOnGPU = MAX_CUBES;

  UINT curFrame = 0;
  UINT lastCompletedFrame = 0;
  ID3D11Query* queries[MAX_QUERY];
};
