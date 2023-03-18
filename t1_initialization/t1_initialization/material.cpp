#include "Material.h"

HRESULT Material::Init(ID3D11Device* device, ID3D11DeviceContext* context, const MaterialParams& params) {
  this->shines = params.shines;

  HRESULT hr = diffuse.Init(device, context, params.diffPath);
  if (FAILED(hr))
    return hr;

  hr = normal.Init(device, context, params.normalPath);
  
  return hr;
}

void Material::Realese() {
  diffuse.Release();
  normal.Release();
}

void Material::AttachToShaders(ID3D11DeviceContext* context) {
  ID3D11ShaderResourceView* resources[] = { 
    diffuse.GetTexture(),
    normal.GetTexture() 
  };
  context->PSSetShaderResources(0, 2, resources);
}
