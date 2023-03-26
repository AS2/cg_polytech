#pragma once

#include <directxmath.h>

using namespace DirectX;

class FrustumCulling {
public:
  void Init(float screenDepth) { this->screenDepth = screenDepth; };

  void Realese() {};

  void ConstructFrustum(XMMATRIX viewMatrix, XMMATRIX projectionMatrix);

  bool CheckRectangle(float maxWidth, float maxHeight, float maxDepth, float minWidth, float minHeight, float minDepth);

private:
  float screenDepth;
  float planes[6][4];
};
