#pragma once

#include <d3d11.h>
#include <directxmath.h>

using namespace DirectX;

class Camera {
public:
  // Initialization camera method
  HRESULT Init();

  // Update frame method
  void Frame();

  // Get view matrix method
  void GetBaseViewMatrix(XMMATRIX& viewMatrix);

private:
  XMMATRIX viewMatrix;
  XMFLOAT3 pointOfInterest;

  float distanceToPoint;
  float phi;
  float theta;
};