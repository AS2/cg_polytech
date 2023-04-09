#pragma once

#include <directxmath.h>

using namespace DirectX;

class FrustumCulling {
public:
  void Init(float screenDepth) { this->screenDepth = screenDepth; };

  void Realese() {};

  void ConstructFrustum(XMMATRIX viewMatrix, XMMATRIX projectionMatrix);

  bool CheckRectangle(XMFLOAT4 bbMin, XMFLOAT4 bbMax);

  XMFLOAT4* GetPlanes() { return planes; };
private:
  float screenDepth;
  XMFLOAT4 planes[6];
};
