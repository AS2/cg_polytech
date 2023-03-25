#include "postprocessing.h"

// Function to initialize
HRESULT Postprocessing::Init(ID3D11Device* device, HWND hwnd) {
  HRESULT hr = S_OK;

  ID3D10Blob* vertexShaderBuffer = nullptr;
  ID3D10Blob* pixelShaderBuffer = nullptr;
  int flags = 0;
#ifdef _DEBUG
  flags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
  // Compile the vertex shader code.
  hr = D3DCompileFromFile(L"Postprocessing_VS.hlsl", NULL, NULL, "main", "vs_5_0", flags, 0, &vertexShaderBuffer, NULL);
  hr = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &g_pVertexShader);
  if (FAILED(hr))
    return hr;

  // Compile the pixel shader code.
  hr = D3DCompileFromFile(L"Postprocessing_PS.hlsl", NULL, NULL, "main", "ps_5_0", flags, 0, &pixelShaderBuffer, NULL);
  hr = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &g_pPixelShader);
  if (FAILED(hr))
    return hr;

  // Release the vertex shader buffer and pixel shader buffer since they are no longer needed.
  vertexShaderBuffer->Release();
  pixelShaderBuffer->Release();
  
  // Create the sampler state
  D3D11_SAMPLER_DESC samplerDesc;
  ZeroMemory(&samplerDesc, sizeof(samplerDesc));
  samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
  samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
  samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
  samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
  samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
  samplerDesc.MinLOD = 0;
  samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
  samplerDesc.MaxAnisotropy = D3D11_MAX_MAXANISOTROPY;
  
  // Create the texture sampler state.
  hr = device->CreateSamplerState(&samplerDesc, &g_pSamplerState);
  if (FAILED(hr))
    return hr;
  
  // Create constant bufer
  D3D11_BUFFER_DESC desc = {};
  desc.ByteWidth = sizeof(PostprocessingCB);
  desc.Usage = D3D11_USAGE_DEFAULT;
  desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
  desc.CPUAccessFlags = 0;
  desc.MiscFlags = 0;
  desc.StructureByteStride = 0;

  PostprocessingCB postCB;
  postCB.params = XMFLOAT4(0, 0, 0, 0);

  D3D11_SUBRESOURCE_DATA data;
  data.pSysMem = &postCB;
  data.SysMemPitch = sizeof(PostprocessingCB);
  data.SysMemSlicePitch = 0;

  hr = device->CreateBuffer(&desc, &data, &g_pPostprocessingCB);

  return hr;
}

void Postprocessing::Release() {
  if (g_pSamplerState) g_pSamplerState->Release();
  if (g_pPixelShader) g_pPixelShader->Release();
  if (g_pVertexShader) g_pVertexShader->Release();
  if (g_pPostprocessingCB) g_pPostprocessingCB->Release();
}


void Postprocessing::Render(ID3D11DeviceContext* context, ID3D11ShaderResourceView* sourceTexture, ID3D11RenderTargetView* renderTarget, D3D11_VIEWPORT viewport) {
  context->OMSetRenderTargets(1, &renderTarget, nullptr);
  context->RSSetViewports(1, &viewport);

  context->IASetInputLayout(nullptr);
  context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

  context->VSSetShader(g_pVertexShader, nullptr, 0);
  context->PSSetShader(g_pPixelShader, nullptr, 0);
  context->PSSetConstantBuffers(0, 1, &g_pPostprocessingCB);
  context->PSSetShaderResources(0, 1, &sourceTexture);
  context->PSSetSamplers(0, 1, &g_pSamplerState);

  context->Draw(3, 0);

  ID3D11ShaderResourceView* nullsrv[] = { nullptr };
  context->PSSetShaderResources(0, 1, nullsrv);
}

bool Postprocessing::Frame(ID3D11DeviceContext* context) {
  auto duration = Timer::GetInstance().Clock();
  PostprocessingCB postCB;

  float tripVelocity = 1.f / 4.f;
  float alpha = (float)(sin(tripVelocity * duration / 0.33f) * 0.4f + 0.6f),
    beta = (float)(cos(tripVelocity * duration / 0.5f) * 0.4f + 0.6f),
    gamma = (float)(sin(tripVelocity * duration / 0.5f) * 0.4f + 0.6f);
  postCB.params = XMFLOAT4(
    alpha,
    beta,
    gamma, 1.f);

  context->UpdateSubresource(g_pPostprocessingCB, 0, nullptr, &postCB, 0, 0);
  return true;
}
