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

HRESULT Scene::InitBox(ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight) {
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
  hr = txt.Init(device, context, L"./src/hah2.dds");

  // Create vertex buffer
  TexVertex vertices[] = {
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
  bd.Usage = D3D11_USAGE_IMMUTABLE;
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
  bd1.Usage = D3D11_USAGE_IMMUTABLE;
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

  hr = device->CreateBuffer(&descWMB, &data, &g_pWorldMatrixBuffer1);
  if (FAILED(hr))
    return hr;
  hr = device->CreateBuffer(&descWMB, &data, &g_pWorldMatrixBuffer2);
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
  descRastr.FillMode = D3D11_FILL_SOLID;
  descRastr.CullMode = D3D11_CULL_BACK;
  descRastr.FrontCounterClockwise = false;
  descRastr.DepthBias = 0;
  descRastr.SlopeScaledDepthBias = 0.0f;
  descRastr.DepthBiasClamp = 0.0f;
  descRastr.DepthClipEnable = true;
  descRastr.ScissorEnable = false;
  descRastr.MultisampleEnable = false;
  descRastr.AntialiasedLineEnable = false;

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

  // Set depth state
  D3D11_DEPTH_STENCIL_DESC dsDesc = { 0 };
  ZeroMemory(&dsDesc, sizeof(dsDesc));
  dsDesc.DepthEnable = TRUE;
  dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
  dsDesc.DepthFunc = D3D11_COMPARISON_GREATER_EQUAL;
  dsDesc.StencilEnable = FALSE;

  hr = device->CreateDepthStencilState(&dsDesc, &g_pDepthState);
  if (FAILED(hr))
    return hr;

  return S_OK;
}

HRESULT Scene::InitPlanes(ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight) {
  // Compile the vertex shader
  ID3DBlob* pVSBlob = nullptr;
  HRESULT hr = CompileShaderFromFile(L"transparent_VS.hlsl", "main", "vs_5_0", &pVSBlob);
  if (FAILED(hr))
  {
    MessageBox(nullptr,
      L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
    return hr;
  }

  // Create the vertex shader
  hr = device->CreateVertexShader(pVSBlob->GetBufferPointer(), pVSBlob->GetBufferSize(), nullptr, &g_pVertexShaderPlane);
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
    pVSBlob->GetBufferSize(), &g_pVertexLayoutPlane);
  pVSBlob->Release();
  if (FAILED(hr))
    return hr;

  // Set the input layout
  context->IASetInputLayout(g_pVertexLayoutPlane);

  // Compile the pixel shader
  ID3DBlob* pPSBlob = nullptr;
  hr = CompileShaderFromFile(L"transparent_PS.hlsl", "main", "ps_5_0", &pPSBlob);
  if (FAILED(hr))
  {
    MessageBox(nullptr,
      L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
    return hr;
  }

  // Create the pixel shader
  hr = device->CreatePixelShader(pPSBlob->GetBufferPointer(), pPSBlob->GetBufferSize(), nullptr, &g_pPixelShaderPlane);
  pPSBlob->Release();
  if (FAILED(hr))
    return hr;

  // Create vertex buffer
  USHORT indices[] = {
        0, 2, 1, 0, 3, 2,
  };

  D3D11_BUFFER_DESC bd;
  ZeroMemory(&bd, sizeof(bd));
  bd.Usage = D3D11_USAGE_IMMUTABLE;
  bd.ByteWidth = sizeof(Vertices);
  bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
  bd.CPUAccessFlags = 0;
  bd.MiscFlags = 0;
  bd.StructureByteStride = 0;

  D3D11_SUBRESOURCE_DATA InitData;
  ZeroMemory(&InitData, sizeof(InitData));
  InitData.pSysMem = &Vertices;
  InitData.SysMemPitch = sizeof(Vertices);
  InitData.SysMemSlicePitch = 0;

  hr = device->CreateBuffer(&bd, &InitData, &g_pVertexBufferPlane);
  if (FAILED(hr))
    return hr;

  // Create index buffer
  D3D11_BUFFER_DESC bd1;
  ZeroMemory(&bd1, sizeof(bd1));
  bd1.Usage = D3D11_USAGE_IMMUTABLE;
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

  hr = device->CreateBuffer(&bd1, &InitData1, &g_pIndexBufferPlane);
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

  hr = device->CreateBuffer(&descWMB, &data, &g_pWorldMatrixBuffer1Plane);
  if (FAILED(hr))
    return hr;
  hr = device->CreateBuffer(&descWMB, &data, &g_pWorldMatrixBuffer2Plane);
  if (FAILED(hr))
    return hr;

  D3D11_BUFFER_DESC descSMB = {};
  descSMB.ByteWidth = sizeof(SceneMatrixBuffer);
  descSMB.Usage = D3D11_USAGE_DYNAMIC;
  descSMB.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  descSMB.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  descSMB.MiscFlags = 0;
  descSMB.StructureByteStride = 0;

  hr = device->CreateBuffer(&descSMB, nullptr, &g_pSceneMatrixBufferPlane);
  if (FAILED(hr))
    return hr;

  // Set rastrizer state
  D3D11_RASTERIZER_DESC descRastr = {};
  descRastr.FillMode = D3D11_FILL_SOLID;
  descRastr.CullMode = D3D11_CULL_NONE;
  descRastr.FrontCounterClockwise = false;
  descRastr.DepthBias = 0;
  descRastr.SlopeScaledDepthBias = 0.0f;
  descRastr.DepthBiasClamp = 0.0f;
  descRastr.DepthClipEnable = true;
  descRastr.ScissorEnable = false;
  descRastr.MultisampleEnable = false;
  descRastr.AntialiasedLineEnable = false;

  hr = device->CreateRasterizerState(&descRastr, &g_pRasterizerStatePlane);
  if (FAILED(hr))
    return hr;

  // Set depth state
  D3D11_DEPTH_STENCIL_DESC dsDesc = { 0 };
  ZeroMemory(&dsDesc, sizeof(dsDesc));
  dsDesc.DepthEnable = TRUE;
  dsDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
  dsDesc.DepthFunc = D3D11_COMPARISON_GREATER;
  dsDesc.StencilEnable = FALSE;

  hr = device->CreateDepthStencilState(&dsDesc, &g_pDepthStatePlane);
  if (FAILED(hr))
    return hr;

  // Create blend state
  D3D11_BLEND_DESC descBS = { 0 };
  descBS.AlphaToCoverageEnable = false;
  descBS.IndependentBlendEnable = false;
  descBS.RenderTarget[0].BlendEnable = true;
  descBS.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
  descBS.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
  descBS.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
  descBS.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_RED |
                                                 D3D11_COLOR_WRITE_ENABLE_GREEN | 
                                                 D3D11_COLOR_WRITE_ENABLE_BLUE;
  descBS.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
  descBS.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
  descBS.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;

  hr = device->CreateBlendState(&descBS, &g_pTransBlendStatePlane);

  return S_OK;
}

HRESULT Scene::Init(ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight) {
  HRESULT hr = InitBox(device, context, screenWidth, screenHeight);
  if (FAILED(hr))
    return hr;

  hr = InitPlanes(device, context, screenWidth, screenHeight);
  if (FAILED(hr))
    return hr;

  hr = sb.Init(device, context, screenWidth, screenHeight);

  return hr;
}

void Scene::RealeseBox() {
  txt.Release();

  if (g_pSamplerState) g_pSamplerState->Release();
  if (g_pRasterizerState) g_pRasterizerState->Release();
  if (g_pWorldMatrixBuffer1) g_pWorldMatrixBuffer1->Release();
  if (g_pWorldMatrixBuffer2) g_pWorldMatrixBuffer2->Release();
  if (g_pDepthState) g_pDepthState->Release();
  if (g_pSceneMatrixBuffer) g_pSceneMatrixBuffer->Release();
  if (g_pIndexBuffer) g_pIndexBuffer->Release();
  if (g_pVertexBuffer) g_pVertexBuffer->Release();
  if (g_pVertexLayout) g_pVertexLayout->Release();
  if (g_pVertexShader) g_pVertexShader->Release();
  if (g_pPixelShader) g_pPixelShader->Release();
}

void Scene::RealesePlanes() {
  if (g_pTransBlendStatePlane) g_pTransBlendStatePlane->Release();
  if (g_pRasterizerStatePlane) g_pRasterizerStatePlane->Release();
  if (g_pWorldMatrixBuffer1Plane) g_pWorldMatrixBuffer1Plane->Release();
  if (g_pWorldMatrixBuffer2Plane) g_pWorldMatrixBuffer2Plane->Release();
  if (g_pDepthStatePlane) g_pDepthStatePlane->Release();
  if (g_pSceneMatrixBufferPlane) g_pSceneMatrixBufferPlane->Release();
  if (g_pIndexBufferPlane) g_pIndexBufferPlane->Release();
  if (g_pVertexBufferPlane) g_pVertexBufferPlane->Release();
  if (g_pVertexLayoutPlane) g_pVertexLayoutPlane->Release();
  if (g_pVertexShaderPlane) g_pVertexShaderPlane->Release();
  if (g_pPixelShaderPlane) g_pPixelShaderPlane->Release();
}

void Scene::Realese() {
  RealeseBox();
  RealesePlanes();

  sb.Realese();
}

void Scene::RenderBox(ID3D11DeviceContext* context) {
  context->OMSetDepthStencilState(g_pDepthState, 0);
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
  context->VSSetConstantBuffers(1, 1, &g_pSceneMatrixBuffer);
  context->PSSetShader(g_pPixelShader, nullptr, 0);

  {
    context->VSSetConstantBuffers(0, 1, &g_pWorldMatrixBuffer1);
    context->DrawIndexed(36, 0, 0);
  }

  {
    context->VSSetConstantBuffers(0, 1, &g_pWorldMatrixBuffer2);
    context->DrawIndexed(36, 0, 0);
  }
}

void Scene::RenderPlanes(ID3D11DeviceContext* context) {
  context->OMSetDepthStencilState(g_pDepthStatePlane, 0);
  context->RSSetState(g_pRasterizerStatePlane);

  context->IASetIndexBuffer(g_pIndexBufferPlane, DXGI_FORMAT_R16_UINT, 0);
  ID3D11Buffer* vertexBuffers[] = { g_pVertexBufferPlane };
  UINT stride = sizeof(XMFLOAT4);
  UINT offset = 0;
  context->IASetVertexBuffers(0, 1, vertexBuffers, &stride, &offset);

  context->IASetInputLayout(g_pVertexLayoutPlane);
  context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  context->VSSetShader(g_pVertexShaderPlane, nullptr, 0);
  context->VSSetConstantBuffers(1, 1, &g_pSceneMatrixBufferPlane);
  context->PSSetShader(g_pPixelShaderPlane, nullptr, 0);
  context->OMSetBlendState(g_pTransBlendStatePlane, nullptr, 0xFFFFFFFF);


  if (!firstNear) {
    {
      context->VSSetConstantBuffers(0, 1, &g_pWorldMatrixBuffer2Plane);
      context->PSSetConstantBuffers(0, 1, &g_pWorldMatrixBuffer2Plane);
      context->DrawIndexed(6, 0, 0);
    }

    {
      context->VSSetConstantBuffers(0, 1, &g_pWorldMatrixBuffer1Plane);
      context->PSSetConstantBuffers(0, 1, &g_pWorldMatrixBuffer1Plane);
      context->DrawIndexed(6, 0, 0);
    }
  }
  else {
    {
      context->VSSetConstantBuffers(0, 1, &g_pWorldMatrixBuffer1Plane);
      context->PSSetConstantBuffers(0, 1, &g_pWorldMatrixBuffer1Plane);
      context->DrawIndexed(6, 0, 0);
    }

    {
      context->VSSetConstantBuffers(0, 1, &g_pWorldMatrixBuffer2Plane);
      context->PSSetConstantBuffers(0, 1, &g_pWorldMatrixBuffer2Plane);
      context->DrawIndexed(6, 0, 0);
    }
  }
}

void Scene::Render(ID3D11DeviceContext* context) {
  RenderBox(context);
  sb.Render(context);
  RenderPlanes(context);
}

bool Scene::FrameBox(ID3D11DeviceContext* context, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 cameraPos) {
  // Update world matrix angle of first cube
  auto duration = Timer::GetInstance().Clock();

  WorldMatrixBuffer worldMatrixBuffer;
  worldMatrixBuffer.worldMatrix = XMMatrixRotationY((float)duration * angle_velocity) *
    XMMatrixRotationZ((float)(sin(duration * angle_velocity * 0.30) * 0.5));
  context->UpdateSubresource(g_pWorldMatrixBuffer1, 0, nullptr, &worldMatrixBuffer, 0, 0);

  // Update world matrix angle of second cube
  worldMatrixBuffer.worldMatrix = XMMatrixRotationY((float)duration * angle_velocity * 0.25f) *
    XMMatrixTranslation((float)sin(duration) * 3.0f, 0.0f, (float)cos(duration) * 3.0f);
  context->UpdateSubresource(g_pWorldMatrixBuffer2, 0, nullptr, &worldMatrixBuffer, 0, 0);

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

float Scene::DistToPlane(WorldMatrixBuffer worldMatrix, XMFLOAT3 cameraPos) {
  XMFLOAT4 rectVert[4];
  float maxDist = -D3D11_FLOAT32_MAX;

  std::copy(Vertices, Vertices + 4, rectVert);
  for (int i = 0; i < 4; i++) {
    XMStoreFloat4(&rectVert[i], XMVector4Transform(XMLoadFloat4(&rectVert[i]), worldMatrix.worldMatrix));
    float dist = (rectVert[i].x * cameraPos.x) + (rectVert[i].y * cameraPos.y) + (rectVert[i].z * cameraPos.z);
    maxDist = max(maxDist, dist);
  }

  return maxDist;
}

bool Scene::FramePlanes(ID3D11DeviceContext* context, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 cameraPos) {
  // Update world matrix angle
  auto duration = Timer::GetInstance().Clock();

  WorldMatrixBuffer worldMatrixBuffer1;
  worldMatrixBuffer1.color = XMFLOAT4(0.f, 0.0f, 1.f, 0.5f);
  worldMatrixBuffer1.worldMatrix = XMMatrixTranslation(1.25f, 0, (float)(sin(duration) * -2.0));
  context->UpdateSubresource(g_pWorldMatrixBuffer1Plane, 0, nullptr, &worldMatrixBuffer1, 0, 0);

  WorldMatrixBuffer worldMatrixBuffer2;
  worldMatrixBuffer2.color = XMFLOAT4(1.0f, 0.0f, 0.0f, 0.0f);
  worldMatrixBuffer2.worldMatrix = XMMatrixTranslation(-1.25f, 0, (float)(sin(duration) * 2.0));
  context->UpdateSubresource(g_pWorldMatrixBuffer2Plane, 0, nullptr, &worldMatrixBuffer2, 0, 0);

  // count distances to planes
  firstNear = DistToPlane(worldMatrixBuffer1, cameraPos) < DistToPlane(worldMatrixBuffer2, cameraPos);

  // Get the view matrix
  D3D11_MAPPED_SUBRESOURCE subresource;
  HRESULT hr = context->Map(g_pSceneMatrixBufferPlane, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
  if (FAILED(hr))
    return FAILED(hr);

  SceneMatrixBuffer& sceneBuffer = *reinterpret_cast<SceneMatrixBuffer*>(subresource.pData);
  sceneBuffer.viewProjectionMatrix = XMMatrixMultiply(viewMatrix, projectionMatrix);
  context->Unmap(g_pSceneMatrixBufferPlane, 0);
  
  return S_OK;
}

bool Scene::Frame(ID3D11DeviceContext* context, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 cameraPos) {
  FrameBox(context, viewMatrix, projectionMatrix, cameraPos);
  
  FramePlanes(context, viewMatrix, projectionMatrix, cameraPos);
  
  sb.Frame(context, viewMatrix, projectionMatrix, cameraPos);
  
  return true;
}
