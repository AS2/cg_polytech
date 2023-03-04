#include "scene.h"

HRESULT Scene::CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
{
  HRESULT hr = S_OK;

  DWORD dwShaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
  // Set the D3DCOMPILE_DEBUG flag to embed debug information in the shaders.
  // Setting this flag improves the shader debugging experience, but still allows 
  // the shaders to be optimized and to run exactly the way they will run in 
  // the release configuration of this program.
  dwShaderFlags |= D3DCOMPILE_DEBUG;

  // Disable optimizations to further improve shader debugging
  dwShaderFlags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

  ID3DBlob* pErrorBlob = nullptr;
  hr = D3DCompileFromFile(szFileName, nullptr, nullptr, szEntryPoint, szShaderModel,
    dwShaderFlags, 0, ppBlobOut, &pErrorBlob);
  if (FAILED(hr))
  {
    if (pErrorBlob)
    {
      OutputDebugStringA(reinterpret_cast<const char*>(pErrorBlob->GetBufferPointer()));
      pErrorBlob->Release();
    }
    return hr;
  }
  if (pErrorBlob) pErrorBlob->Release();

  return S_OK;
}

HRESULT Scene::Init(ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight) {
  // Compile the vertex shader
  ID3DBlob* pVSBlob = nullptr;
  HRESULT hr = CompileShaderFromFile(L"t2_VS.hlsl", "main", "vs_5_0", &pVSBlob);
  if (FAILED(hr))
  {
    MessageBox(nullptr,
      L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
    return hr;
  }

  // Create the vertex shader
  hr = device->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &g_pVertexShader);
  if (FAILED(hr))
  {
    pVSBlob->Release();
    return hr;
  }

  // Define the input layout
  D3D11_INPUT_ELEMENT_DESC layout[] =
  {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}
  };
  UINT numElements = ARRAYSIZE(layout);

  // Create the input layout
  hr = device->CreateInputLayout(layout, numElements, pVSBlob->GetBufferPointer(),
    pVSBlob->GetBufferSize(), &g_pVertexLayout);
  pVSBlob->Release();
  if (FAILED(hr))
    return hr;

  // Set the input layout
  context->IASetInputLayout(g_pVertexLayout);

  // Compile the pixel shader
  ID3DBlob* pPSBlob = nullptr;
  hr = CompileShaderFromFile(L"t2_PS.hlsl", "main", "ps_5_0", &pPSBlob);
  if (FAILED(hr))
  {
    MessageBox(nullptr,
      L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
    return hr;
  }

  // Create the pixel shader
  hr = device->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pPixelShader);
  pPSBlob->Release();
  if (FAILED(hr))
    return hr;

  // Load text
  hr = txt.Init(device, context, L"./src/hah.dds");

  // Create vertex buffer
  SimpleVertex vertices[] = {
    {-0.5, -0.5,  0.5, 0, 1},
    { 0.5, -0.5,  0.5, 1, 1},
    { 0.5, -0.5, -0.5, 1, 0},
    {-0.5, -0.5, -0.5, 0, 0},

    {-0.5,  0.5, -0.5, 1, 1},
    { 0.5,  0.5, -0.5, 0, 1},
    { 0.5,  0.5,  0.5, 0, 0},
    {-0.5,  0.5,  0.5, 1, 0},

    { 0.5, -0.5, -0.5, 0, 1},
    { 0.5, -0.5,  0.5, 1, 1},
    { 0.5,  0.5,  0.5, 1, 0},
    { 0.5,  0.5, -0.5, 0, 0},

    {-0.5, -0.5,  0.5, 0, 1},
    {-0.5, -0.5, -0.5, 1, 1},
    {-0.5,  0.5, -0.5, 1, 0},
    {-0.5,  0.5,  0.5, 0, 0},

    { 0.5, -0.5,  0.5, 1, 1},
    {-0.5, -0.5,  0.5, 0, 1},
    {-0.5,  0.5,  0.5, 0, 0},
    { 0.5,  0.5,  0.5, 1, 0},

    {-0.5, -0.5, -0.5, 1, 1},
    { 0.5, -0.5, -0.5, 0, 1},
    { 0.5,  0.5, -0.5, 0, 0},
    {-0.5,  0.5, -0.5, 1, 0}
  };

  USHORT indices[] = {
        0, 2, 1, 0, 3, 2,
        4, 6, 5, 4, 7, 6,
        8, 10, 9, 8, 11, 10,
        12, 14, 13, 12, 15, 14,
        16, 18, 17, 16, 19, 18,
        20, 22, 21, 20, 23, 22
  };

  D3D11_BUFFER_DESC bd;
  ZeroMemory(&bd, sizeof(bd));
  bd.Usage = D3D11_USAGE_DEFAULT;
  bd.ByteWidth = sizeof(vertices);
  bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  bd.CPUAccessFlags = 0;
  bd.MiscFlags = 0;
  bd.StructureByteStride = 0;

  D3D11_SUBRESOURCE_DATA InitData;
  ZeroMemory(&InitData, sizeof(InitData));
  InitData.pSysMem = &vertices;
  InitData.SysMemPitch = sizeof(vertices);
  InitData.SysMemSlicePitch = 0;

  hr = device->CreateBuffer(&bd, &InitData, &g_pVertexBuffer);
  if (FAILED(hr))
    return hr;

  // Create index buffer
  D3D11_BUFFER_DESC bd1;
  ZeroMemory(&bd1, sizeof(bd1));
  bd1.Usage = D3D11_USAGE_DEFAULT;
  bd1.ByteWidth = sizeof(indices);
  bd1.BindFlags = D3D11_BIND_INDEX_BUFFER;
  bd1.CPUAccessFlags = 0;
  bd1.MiscFlags = 0;
  bd1.StructureByteStride = 0;

  D3D11_SUBRESOURCE_DATA InitData1;
  ZeroMemory(&InitData1, sizeof(InitData1));
  InitData1.pSysMem = &indices;
  InitData1.SysMemPitch = sizeof(indices);
  InitData1.SysMemSlicePitch = 0;

  hr = device->CreateBuffer(&bd1, &InitData1, &g_pIndexBuffer);
  if (FAILED(hr))
    return hr;

  // Set constant buffers
  D3D11_BUFFER_DESC descWMB = {};
  descWMB.ByteWidth = sizeof(WorldMatrixBuffer);
  descWMB.Usage = D3D11_USAGE_DEFAULT;
  descWMB.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  descWMB.CPUAccessFlags = 0;
  descWMB.MiscFlags = 0;
  descWMB.StructureByteStride = 0;

  WorldMatrixBuffer worldMatrixBuffer;
  worldMatrixBuffer.worldMatrix = DirectX::XMMatrixIdentity();

  D3D11_SUBRESOURCE_DATA data;
  data.pSysMem = &worldMatrixBuffer;
  data.SysMemPitch = sizeof(worldMatrixBuffer);
  data.SysMemSlicePitch = 0;

  hr = device->CreateBuffer(&descWMB, &data, &g_pWorldMatrixBuffer);
  if (FAILED(hr))
    return hr;

  D3D11_BUFFER_DESC descSMB = {};
  descSMB.ByteWidth = sizeof(SceneMatrixBuffer);
  descSMB.Usage = D3D11_USAGE_DYNAMIC;
  descSMB.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  descSMB.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  descSMB.MiscFlags = 0;
  descSMB.StructureByteStride = 0;

  hr = device->CreateBuffer(&descSMB, nullptr, &g_pSceneMatrixBuffer);
  if (FAILED(hr))
    return hr;

  // Set rastrizer state
  D3D11_RASTERIZER_DESC descRastr = {};
  descRastr.AntialiasedLineEnable = false;
  descRastr.FillMode = D3D11_FILL_SOLID;
  descRastr.CullMode = D3D11_CULL_BACK;
  descRastr.DepthBias = 0;
  descRastr.DepthBiasClamp = 0.0f;
  descRastr.FrontCounterClockwise = false;
  descRastr.DepthClipEnable = true;
  descRastr.ScissorEnable = false;
  descRastr.MultisampleEnable = false;
  descRastr.SlopeScaledDepthBias = 0.0f;

  hr = device->CreateRasterizerState(&descRastr, &g_pRasterizerState);
  if (FAILED(hr))
    return hr;

  // Set sampler state
  D3D11_SAMPLER_DESC descSmplr = {};
  descSmplr.Filter = D3D11_FILTER_ANISOTROPIC;
  descSmplr.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
  descSmplr.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
  descSmplr.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
  descSmplr.MinLOD = -D3D11_FLOAT32_MAX;
  descSmplr.MaxLOD = D3D11_FLOAT32_MAX;
  descSmplr.MipLODBias = 0.0f;
  descSmplr.MaxAnisotropy = 16;
  descSmplr.ComparisonFunc = D3D11_COMPARISON_NEVER;
  descSmplr.BorderColor[0] =
    descSmplr.BorderColor[1] =
    descSmplr.BorderColor[2] =
    descSmplr.BorderColor[3] = 1.0f;

  hr = device->CreateSamplerState(&descSmplr, &g_pSamplerState);
  if (FAILED(hr))
    return hr;

  return S_OK;
}

void Scene::Realese() {
  txt.Release();

  if (g_pSamplerState) g_pSamplerState->Release();
  if (g_pRasterizerState) g_pRasterizerState->Release();
  if (g_pWorldMatrixBuffer) g_pWorldMatrixBuffer->Release();
  if (g_pSceneMatrixBuffer) g_pSceneMatrixBuffer->Release();
  if (g_pIndexBuffer) g_pIndexBuffer->Release();
  if (g_pVertexBuffer) g_pVertexBuffer->Release();
  if (g_pVertexLayout) g_pVertexLayout->Release();
  if (g_pVertexShader) g_pVertexShader->Release();
  if (g_pPixelShader) g_pPixelShader->Release();
}

void Scene::Render(ID3D11DeviceContext* context) {
  context->RSSetState(g_pRasterizerState);

  context->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
  ID3D11SamplerState* samplers[] = { g_pSamplerState };
  context->PSSetSamplers(0, 1, samplers);

  ID3D11ShaderResourceView* resources[] = { txt.GetTexture() };
  context->PSSetShaderResources(0, 1, resources);
  ID3D11Buffer* vertexBuffers[] = { g_pVertexBuffer };
  UINT strides[] = { 20 };
  UINT offsets[] = { 0 };

  context->IASetVertexBuffers(0, 1, vertexBuffers, strides, offsets);
  context->IASetInputLayout(g_pVertexLayout);
  context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  context->VSSetShader(g_pVertexShader, nullptr, 0);
  context->VSSetConstantBuffers(0, 1, &g_pWorldMatrixBuffer);
  context->VSSetConstantBuffers(1, 1, &g_pSceneMatrixBuffer);
  context->PSSetShader(g_pPixelShader, nullptr, 0);

  context->DrawIndexed(36, 0, 0);
}

bool Scene::Frame(ID3D11DeviceContext* context, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 cameraPos) {
  // Update world matrix angle
  auto duration = Timer::GetInstance().Clock();

  WorldMatrixBuffer worldMatrixBuffer;
  worldMatrixBuffer.worldMatrix = XMMatrixRotationY((float)duration * angle_velocity) *
    XMMatrixRotationZ((float)(sin(duration * angle_velocity * 0.30) * 0.5)) *
    XMMatrixTranslation((float)(sin(duration) * 3.0), 0, 0);// (float)(cos(duration) * 3.0));
  context->UpdateSubresource(g_pWorldMatrixBuffer, 0, nullptr, &worldMatrixBuffer, 0, 0);

  // Get the view matrix
  D3D11_MAPPED_SUBRESOURCE subresource;
  HRESULT hr = context->Map(g_pSceneMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
  if (FAILED(hr))
    return FAILED(hr);

  SceneMatrixBuffer& sceneBuffer = *reinterpret_cast<SceneMatrixBuffer*>(subresource.pData);
  sceneBuffer.viewProjectionMatrix = XMMatrixMultiply(viewMatrix, projectionMatrix);
  context->Unmap(g_pSceneMatrixBuffer, 0);
  
  return S_OK;
}
