#pragma once

#include "texture.h"

struct MaterialParams {
  const wchar_t* diffPath;
  const wchar_t* normalPath;
  float shines;
};

class Material {
public:
  HRESULT Init(ID3D11Device* device, ID3D11DeviceContext* context, const MaterialParams& params);

  void Realese();

  void AttachToShaders(ID3D11DeviceContext* context);

  float GetShines() { return shines; };
private:
  Texture diffuse,
    normal;

  float shines;
};
