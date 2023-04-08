#include "box.h"

HRESULT Box::CompileShaderFromFile(const WCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel, ID3DBlob** ppBlobOut)
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
  D3DInclude includeObj;

  ID3DBlob* pErrorBlob = nullptr;
  hr = D3DCompileFromFile(szFileName, nullptr, &includeObj, szEntryPoint, szShaderModel,
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

void Box::ReadQueries(ID3D11DeviceContext* context) {
  D3D11_QUERY_DATA_PIPELINE_STATISTICS stats;

  while (lastCompletedFrame < curFrame) {
    HRESULT hr = context->GetData(queries[lastCompletedFrame % MAX_QUERY], &stats, sizeof(D3D11_QUERY_DATA_PIPELINE_STATISTICS), 0);
    if (hr == S_OK) {
      cubesDrawedOnGPU = int(stats.IAPrimitives / 12);
      lastCompletedFrame++;
    }
    else {
      break;
    }
  }
}

HRESULT Box::InitQuery(ID3D11Device* device) {
  HRESULT hr = S_OK;
  D3D11_QUERY_DESC desc;

  desc.Query = D3D11_QUERY_PIPELINE_STATISTICS;
  desc.MiscFlags = 0;
  for (int i = 0; i < MAX_QUERY && SUCCEEDED(hr); i++)
    hr = device->CreateQuery(&desc, &queries[i]);

  return hr;
}


HRESULT Box::Init(ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight, const MaterialParams &params, const std::vector<XMFLOAT4>& positions) {
  assert(positions.size() == MAX_CUBES);

  InitQuery(device);

  // Init frustum culling
  frustum.Init(0.01f);

  // Init cubes params
  boxesModelVector = std::vector<BoxModel>(MAX_CUBES);
  for (int i = 0; i < MAX_CUBES; i++) {
    BoxModel tmp;
    float textureIndex = (float)(rand() % params.diffPaths.size());
    tmp.pos = positions[i];
    tmp.params = XMFLOAT4(params.shines,
      (float)(rand() % 10 - 5), 
      textureIndex, 
      textureIndex > 0.0f ? 0.0f : 1.0f);
    boxesModelVector[i] = tmp;
  }
  
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
      {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0}, 
      {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D11_INPUT_PER_VERTEX_DATA, 0},
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

  // Compile the compute shader
  ID3DBlob* pCSBlob = nullptr;
  hr = CompileShaderFromFile(L"FrustumCullingShader.hlsl", "main", "cs_5_0", &pCSBlob);
  if (FAILED(hr))
  {
    MessageBox(nullptr,
      L"The FX file cannot be compiled.  Please run this executable from the directory that contains the FX file.", L"Error", MB_OK);
    return hr;
  }

  // Create the pixel shader
  hr = device->CreateComputeShader(pCSBlob->GetBufferPointer(), pCSBlob->GetBufferSize(), nullptr, &g_pCullShader);
  pCSBlob->Release();
  if (FAILED(hr))
    return hr;

  // Load texts
  boxesTextures = std::vector<Texture>(2);
  hr = boxesTextures[0].InitArray(device, context, params.diffPaths);
  hr = boxesTextures[1].Init(device, context, params.normalPath);
  if (FAILED(hr))
    return hr;

  // Create vertex buffer
  TexVertex vertices[] = {
    {{-0.5, -0.5,  0.5}, {0, 1}, {0, -1, 0}, {1, 0, 0}},
    {{ 0.5, -0.5,  0.5}, {1, 1}, {0, -1, 0}, {1, 0, 0}},
    {{ 0.5, -0.5, -0.5}, {1, 0}, {0, -1, 0}, {1, 0, 0}},
    {{-0.5, -0.5, -0.5}, {0, 0}, {0, -1, 0}, {1, 0, 0}},

    {{-0.5,  0.5, -0.5}, {1, 1}, {0, 1, 0}, {1, 0, 0}},
    {{ 0.5,  0.5, -0.5}, {0, 1}, {0, 1, 0}, {1, 0, 0}},
    {{ 0.5,  0.5,  0.5}, {0, 0}, {0, 1, 0}, {1, 0, 0}},
    {{-0.5,  0.5,  0.5}, {1, 0}, {0, 1, 0}, {1, 0, 0}},

    {{ 0.5, -0.5, -0.5}, {0, 1}, {1, 0, 0}, {0, 0, 1}},
    {{ 0.5, -0.5,  0.5}, {1, 1}, {1, 0, 0}, {0, 0, 1}},
    {{ 0.5,  0.5,  0.5}, {1, 0}, {1, 0, 0}, {0, 0, 1}},
    {{ 0.5,  0.5, -0.5}, {0, 0}, {1, 0, 0}, {0, 0, 1}},

    {{-0.5, -0.5,  0.5}, {0, 1}, {-1, 0, 0}, {0, 0, -1}},
    {{-0.5, -0.5, -0.5}, {1, 1}, {-1, 0, 0}, {0, 0, -1}},
    {{-0.5,  0.5, -0.5}, {1, 0}, {-1, 0, 0}, {0, 0, -1}},
    {{-0.5,  0.5,  0.5}, {0, 0}, {-1, 0, 0}, {0, 0, -1}},

    {{ 0.5, -0.5,  0.5}, {1, 1}, {0, 0, 1}, {-1, 0, 0}},
    {{-0.5, -0.5,  0.5}, {0, 1}, {0, 0, 1}, {-1, 0, 0}},
    {{-0.5,  0.5,  0.5}, {0, 0}, {0, 0, 1}, {-1, 0, 0}},
    {{ 0.5,  0.5,  0.5}, {1, 0}, {0, 0, 1}, {-1, 0, 0}},

    {{-0.5, -0.5, -0.5}, {1, 1}, {0, 0, -1}, {1, 0, 0}},
    {{ 0.5, -0.5, -0.5}, {0, 1}, {0, 0, -1}, {1, 0, 0}},
    {{ 0.5,  0.5, -0.5}, {0, 0}, {0, 0, -1}, {1, 0, 0}},
    {{-0.5,  0.5, -0.5}, {1, 0}, {0, 0, -1}, {1, 0, 0}}
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
  descWMB.ByteWidth = sizeof(GeomBuffer) * MAX_CUBES;
  descWMB.Usage = D3D11_USAGE_DEFAULT;
  descWMB.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  descWMB.CPUAccessFlags = 0;
  descWMB.MiscFlags = 0;
  descWMB.StructureByteStride = 0;

  // Find cubes in frustum
  static const XMFLOAT4 AABB[] = {
    {-0.5, -0.5, -0.5, 1.0},
    {0.5,  0.5, 0.5, 1.0}
  };
  GeomBuffer geomBufferInst[MAX_CUBES];
  CullParams cullParams;
  for (int i = 0; i < MAX_CUBES; i++) {
    geomBufferInst[i].worldMatrix = XMMatrixTranslation(
      boxesModelVector[i].pos.x, 
      boxesModelVector[i].pos.y,
      boxesModelVector[i].pos.z);
    geomBufferInst[i].norm = geomBufferInst[i].worldMatrix;
    geomBufferInst[i].params = boxesModelVector[i].params;

    XMFLOAT4 min, max;
    XMStoreFloat4(&min, XMVector4Transform(XMLoadFloat4(&AABB[0]), geomBufferInst[i].worldMatrix));
    XMStoreFloat4(&max, XMVector4Transform(XMLoadFloat4(&AABB[1]), geomBufferInst[i].worldMatrix));
    cullParams.bbMin[i] = min;
    cullParams.bbMax[i] = max;
  }
  cullParams.numShapes = XMINT4(int(boxesModelVector.size()), 0, 0, 0);

  D3D11_SUBRESOURCE_DATA cullData;
  cullData.pSysMem = &cullParams;
  cullData.SysMemPitch = sizeof(cullParams);
  cullData.SysMemSlicePitch = 0;
  hr = device->CreateBuffer(&descWMB, &cullData, &g_pCullParams);
  if (FAILED(hr))
    return hr;

  D3D11_BUFFER_DESC argSrcDesc = {};
  argSrcDesc.ByteWidth = sizeof(D3D11_DRAW_INDEXED_INSTANCED_INDIRECT_ARGS);
  argSrcDesc.Usage = D3D11_USAGE_DEFAULT;
  argSrcDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
  argSrcDesc.CPUAccessFlags = 0;
  argSrcDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
  argSrcDesc.StructureByteStride = sizeof(UINT);

  hr = device->CreateBuffer(&argSrcDesc, nullptr, &g_pInderectArgsSrc);
  if (FAILED(hr))
    return hr;
  hr = device->CreateUnorderedAccessView(g_pInderectArgsSrc, nullptr, &g_pInderectArgsUAV);
  if (FAILED(hr))
    return hr;

  D3D11_BUFFER_DESC argDesc = {};
  argDesc.ByteWidth = sizeof(D3D11_DRAW_INDEXED_INSTANCED_INDIRECT_ARGS);
  argDesc.Usage = D3D11_USAGE_DEFAULT;
  argDesc.BindFlags = 0;
  argDesc.CPUAccessFlags = 0;
  argDesc.MiscFlags = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS;
  argDesc.StructureByteStride = 0;

  hr = device->CreateBuffer(&argDesc, nullptr, &g_pInderectArgs);
  if (FAILED(hr))
    return hr;

  D3D11_BUFFER_DESC gbDesc = {};
  gbDesc.ByteWidth = sizeof(XMINT4) * MAX_CUBES;
  gbDesc.Usage = D3D11_USAGE_DEFAULT;
  gbDesc.BindFlags = D3D11_BIND_UNORDERED_ACCESS;
  gbDesc.CPUAccessFlags = 0;
  gbDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
  gbDesc.StructureByteStride = sizeof(XMINT4);

  hr = device->CreateBuffer(&gbDesc, nullptr, &g_pGeomBufferInstVisGpu);
  if (FAILED(hr))
    return hr;
  hr = device->CreateUnorderedAccessView(g_pGeomBufferInstVisGpu, nullptr, &g_pGeomBufferInstVisGpu_UAV);
  if (FAILED(hr))
    return hr;

  D3D11_BUFFER_DESC gbDescGPU = {};
  gbDescGPU.ByteWidth = sizeof(XMINT4) * MAX_CUBES;
  gbDescGPU.Usage = D3D11_USAGE_DEFAULT;
  gbDescGPU.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  gbDescGPU.CPUAccessFlags = 0;
  gbDescGPU.MiscFlags = 0;
  gbDescGPU.StructureByteStride = 0;

  hr = device->CreateBuffer(&gbDescGPU, nullptr, &g_pGeomBufferInstVis);
  if (FAILED(hr))
    return hr;

  D3D11_SUBRESOURCE_DATA data;
  data.pSysMem = &geomBufferInst;
  data.SysMemPitch = sizeof(geomBufferInst);
  data.SysMemSlicePitch = 0;

  hr = device->CreateBuffer(&descWMB, &data, &g_pGeomBuffer);
  if (FAILED(hr))
    return hr;
  
  D3D11_BUFFER_DESC descSMB = {};
  descSMB.ByteWidth = sizeof(BoxSceneMatrixBuffer);
  descSMB.Usage = D3D11_USAGE_DYNAMIC;
  descSMB.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  descSMB.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  descSMB.MiscFlags = 0;
  descSMB.StructureByteStride = 0;

  hr = device->CreateBuffer(&descSMB, nullptr, &g_pSceneMatrixBuffer);
  if (FAILED(hr))
    return hr;

  D3D11_BUFFER_DESC descLCB = {};
  descLCB.ByteWidth = sizeof(LightableCB);
  descLCB.Usage = D3D11_USAGE_DYNAMIC;
  descLCB.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  descLCB.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
  descLCB.MiscFlags = 0;
  descLCB.StructureByteStride = 0;

  hr = device->CreateBuffer(&descLCB, nullptr, &g_LightConstantBuffer);
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


void Box::Realese() {
  for (auto& tex : boxesTextures)
    tex.Release();

  frustum.Realese();

  if (g_pSamplerState) g_pSamplerState->Release();
  if (g_pRasterizerState) g_pRasterizerState->Release();

  if (g_pGeomBuffer) g_pGeomBuffer->Release();
  if (g_LightConstantBuffer) g_LightConstantBuffer->Release();

  if (g_pDepthState) g_pDepthState->Release();
  if (g_pSceneMatrixBuffer) g_pSceneMatrixBuffer->Release();
  if (g_pIndexBuffer) g_pIndexBuffer->Release();
  if (g_pVertexBuffer) g_pVertexBuffer->Release();
  if (g_pVertexLayout) g_pVertexLayout->Release();
  if (g_pVertexShader) g_pVertexShader->Release();
  if (g_pPixelShader) g_pPixelShader->Release();

  if (g_pInderectArgsSrc) g_pInderectArgsSrc->Release();
  if (g_pInderectArgs) g_pInderectArgs->Release();
  if (g_pGeomBufferInstVisGpu) g_pGeomBufferInstVisGpu->Release();
  if (g_pGeomBufferInstVisGpu_UAV) g_pGeomBufferInstVisGpu_UAV->Release();
  if (g_pGeomBufferInstVis) g_pGeomBufferInstVis->Release();
  if (g_pInderectArgsUAV) g_pInderectArgsUAV->Release();
  if (g_pCullShader) g_pCullShader->Release();
  if (g_pCullParams) g_pCullParams->Release();

  for (auto& q : queries) {
    q->Release();
  }
}

void Box::Render(ID3D11DeviceContext* context) {
  context->OMSetDepthStencilState(g_pDepthState, 0);
  context->RSSetState(g_pRasterizerState);

  context->IASetIndexBuffer(g_pIndexBuffer, DXGI_FORMAT_R16_UINT, 0);
  ID3D11SamplerState* samplers[] = { g_pSamplerState };
  context->PSSetSamplers(0, 1, samplers);

  ID3D11ShaderResourceView* resources[] = { 
    boxesTextures[0].GetTexture(), 
    boxesTextures[1].GetTexture()
  };
  context->PSSetShaderResources(0, 2, resources);
  
  ID3D11Buffer* vertexBuffers[] = { g_pVertexBuffer };
  UINT strides[] = { sizeof(TexVertex) };
  UINT offsets[] = { 0 };

  context->IASetVertexBuffers(0, 1, vertexBuffers, strides, offsets);
  context->IASetInputLayout(g_pVertexLayout);
  context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
  
  context->VSSetShader(g_pVertexShader, nullptr, 0);
  context->VSSetConstantBuffers(0, 1, &g_pGeomBuffer);
  context->VSSetConstantBuffers(1, 1, &g_pSceneMatrixBuffer);
  context->VSSetConstantBuffers(2, 1, &g_pGeomBufferInstVis);

  context->PSSetShader(g_pPixelShader, nullptr, 0);
  context->PSSetConstantBuffers(0, 1, &g_pGeomBuffer);
  context->PSSetConstantBuffers(1, 1, &g_pSceneMatrixBuffer);
  context->PSSetConstantBuffers(2, 1, &g_LightConstantBuffer);

  context->Begin(queries[curFrame % MAX_QUERY]);
  context->DrawIndexedInstancedIndirect(g_pInderectArgs, 0);
  context->End(queries[curFrame % MAX_QUERY]);
  curFrame++;

  ReadQueries(context);
}


bool Box::Frame(ID3D11DeviceContext* context, XMMATRIX& viewMatrix, XMMATRIX& projectionMatrix, XMFLOAT3& cameraPos, const Light& lights) {
  CullParams cullParams;
  
  // Update world matrix angle of all cubes
  auto duration = Timer::GetInstance().Clock();
  GeomBuffer geomBufferInst[MAX_CUBES];

  for (int i = 0; i < MAX_CUBES; i++) {
    geomBufferInst[i].worldMatrix = 
      XMMatrixRotationY((float)duration * boxesModelVector[i].params.y * 0.5f) *
      XMMatrixRotationZ((float)(sin(duration * boxesModelVector[i].params.y * 0.30) * 0.25f)) *
      XMMatrixTranslation(0, (float)(sin(duration * boxesModelVector[i].params.y * 0.30) * 0.25f), 0) *
      XMMatrixTranslation(boxesModelVector[i].pos.x, boxesModelVector[i].pos.y, boxesModelVector[i].pos.z);
    geomBufferInst[i].norm = geomBufferInst[i].worldMatrix;
    geomBufferInst[i].params = boxesModelVector[i].params;
  }

  context->UpdateSubresource(g_pGeomBuffer, 0, nullptr, &geomBufferInst, 0, 0);

  // Calculate frustum
  frustum.ConstructFrustum(viewMatrix, projectionMatrix);

  static const XMFLOAT4 AABB[] = {
    {-0.5, -0.5, -0.5, 1.0},
    {0.5,  0.5, 0.5, 1.0}
  };

  boxesIndexies.clear();
  for (int i = 0; i < MAX_CUBES; i++) {
    XMFLOAT4 min, max;
    
    XMStoreFloat4(&min, XMVector4Transform(XMLoadFloat4(&AABB[0]), geomBufferInst[i].worldMatrix));
    XMStoreFloat4(&max, XMVector4Transform(XMLoadFloat4(&AABB[1]), geomBufferInst[i].worldMatrix));
    
    //if (frustum.CheckRectangle(min, max))
    boxesIndexies.push_back(i);

    cullParams.bbMin[i] = min;
    cullParams.bbMax[i] = max;
  }

  cullParams.numShapes = XMINT4(MAX_CUBES, 0, 0, 0);
  context->UpdateSubresource(g_pCullParams, 0, nullptr, &cullParams, 0, 0);

  // Get the view matrix
  D3D11_MAPPED_SUBRESOURCE subresource;
  HRESULT hr = context->Map(g_pSceneMatrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
  if (FAILED(hr))
    return FAILED(hr);
  BoxSceneMatrixBuffer& sceneBuffer = *reinterpret_cast<BoxSceneMatrixBuffer*>(subresource.pData);
  sceneBuffer.viewProjectionMatrix = XMMatrixMultiply(viewMatrix, projectionMatrix);
  XMFLOAT4* planes = frustum.GetPlanes();
  for (int i = 0; i < 6; i++) {
    sceneBuffer.planes[i] = planes[i];
  }
  context->Unmap(g_pSceneMatrixBuffer, 0);

  // Update Light buffer
  hr = context->Map(g_LightConstantBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &subresource);
  if (FAILED(hr))
    return FAILED(hr);

  LightableCB& lightBuffer = *reinterpret_cast<LightableCB*>(subresource.pData);
  lightBuffer.cameraPos = XMFLOAT4(cameraPos.x, cameraPos.y, cameraPos.z, 1.0f);
  lightBuffer.ambientColor = XMFLOAT4(0.75f, 0.75f, 0.75f, 1.0f);
  auto& lightColors = lights.GetColors();
  auto& lightPos = lights.GetPositions();
  lightBuffer.lightCount = XMINT4(int(lightColors.size()), 1, 0, 0);

  for (int i = 0; i < lightColors.size(); i++) {
    lightBuffer.lightPos[i] = XMFLOAT4(lightPos[i].x, lightPos[i].y, lightPos[i].z, 1.0f);
    lightBuffer.lightColor[i] = XMFLOAT4(lightColors[i].x, lightColors[i].y, lightColors[i].z, 1.0f);
  }
  context->Unmap(g_LightConstantBuffer, 0);


  // GPU Culling
  D3D11_DRAW_INDEXED_INSTANCED_INDIRECT_ARGS args;
  args.IndexCountPerInstance = 36;
  args.InstanceCount = 0;
  args.StartInstanceLocation = 0;
  args.BaseVertexLocation = 0;
  args.StartIndexLocation = 0;
  context->UpdateSubresource(g_pInderectArgsSrc, 0, nullptr, &args, 0, 0);
  UINT groupNumber = MAX_CUBES / 64u + !!(MAX_CUBES % 64u);
  context->CSSetConstantBuffers(0, 1, &g_pCullParams);
  context->CSSetConstantBuffers(1, 1, &g_pSceneMatrixBuffer);
  context->CSSetUnorderedAccessViews(0, 1, &g_pInderectArgsUAV, nullptr);
  context->CSSetUnorderedAccessViews(1, 1, &g_pGeomBufferInstVisGpu_UAV, nullptr);
  context->CSSetShader(g_pCullShader, nullptr, 0);
  context->Dispatch(groupNumber, 1, 1);

  context->CopyResource(g_pGeomBufferInstVis, g_pGeomBufferInstVisGpu);
  context->CopyResource(g_pInderectArgs, g_pInderectArgsSrc);


  return S_OK;
}
