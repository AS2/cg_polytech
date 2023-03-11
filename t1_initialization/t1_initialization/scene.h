// CubemMap.h - class for rendering cubemap
#pragma once

#include <d3dcompiler.h>
#include <dxgi.h>
#include <d3d11.h>
#include <directxmath.h>
#include <string>
#include <vector>

#include "timer.h"
#include "texture.h"

using namespace DirectX;

struct TexVertex
{
  float x, y, z;      // positional coords
  float u, v;         // texture coords
};

static const XMFLOAT4 Vertices[] = {
    {0, -1, -1, 1},
    {0,  1, -1, 1},
    {0,  1,  1, 1},
    {0, -1,  1, 1}
};

struct WorldMatrixBuffer {
  XMMATRIX worldMatrix;
  XMFLOAT4 color;
};

struct SceneMatrixBuffer {
  XMMATRIX viewProjectionMatrix;
};

class Scene {
public:
  HRESULT Init(ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight);

  void Realese();

  void Resize(int screenWidth, int screenHeight) {};

  void Render(ID3D11DeviceContext* context);

  bool Frame(ID3D11DeviceContext* context, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 cameraPos);

private:
  HRESULT CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut);

  HRESULT InitBox(ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight);
  bool FrameBox(ID3D11DeviceContext* context, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 cameraPos);
  void RenderBox(ID3D11DeviceContext* context);
  void RealeseBox();
  
  HRESULT InitPlanes(ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight);
  bool FramePlanes(ID3D11DeviceContext* context, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 cameraPos);
  void RenderPlanes(ID3D11DeviceContext* context);
  void RealesePlanes();

  // dx11 vars
  // - for boxes
  ID3D11VertexShader*       g_pVertexShader = nullptr;
  ID3D11PixelShader*        g_pPixelShader = nullptr;
  ID3D11InputLayout*        g_pVertexLayout = nullptr;

  ID3D11Buffer*             g_pVertexBuffer = nullptr;
  ID3D11Buffer*             g_pIndexBuffer = nullptr;
  ID3D11Buffer*             g_pWorldMatrixBuffer1 = nullptr;  // for box 1
  ID3D11Buffer*             g_pWorldMatrixBuffer2 = nullptr;  // for box 2
  ID3D11Buffer*             g_pSceneMatrixBuffer = nullptr;
  ID3D11RasterizerState*    g_pRasterizerState = nullptr;
  ID3D11SamplerState*       g_pSamplerState = nullptr;
  ID3D11DepthStencilState*  g_pDepthState = nullptr;

  // - for planes
  ID3D11VertexShader*       g_pVertexShaderPlane = nullptr;
  ID3D11PixelShader*        g_pPixelShaderPlane = nullptr;
  ID3D11InputLayout*        g_pVertexLayoutPlane = nullptr;

  ID3D11Buffer*             g_pVertexBufferPlane = nullptr;
  ID3D11Buffer*             g_pIndexBufferPlane = nullptr;
  ID3D11Buffer*             g_pWorldMatrixBuffer1Plane = nullptr;  // for plane 1
  ID3D11Buffer*             g_pWorldMatrixBuffer2Plane = nullptr;  // for plane 2
  ID3D11Buffer*             g_pSceneMatrixBufferPlane = nullptr;
  ID3D11RasterizerState*    g_pRasterizerStatePlane = nullptr;
  ID3D11DepthStencilState*  g_pDepthStatePlane = nullptr;
  ID3D11BlendState*         g_pTransBlendStatePlane = nullptr;

  // Texture with aphex twin
  Texture txt;

  // Velocity of world matrix rotation
  float angle_velocity = 3.1415926f;
};
