#pragma once

#include <d3d11.h>
#include <directxmath.h>

using namespace DirectX;

#define MOVEMENT_DOWNSHIFTING 300.f

class Camera {
public:
  // Initialization camera method
  HRESULT InitCamera();

  // Update frame method
  void Frame();

  // Release method
  void Realese() {};

  // Get view matrix method
  void GetBaseViewMatrix(XMMATRIX& viewMatrix);

  // Move camera with mouse method
  void Move(float dx, float dy, float wheel);

private:
  XMMATRIX viewMatrix;
  XMFLOAT3 pointOfInterest;

  float distanceToPoint;
  float phi;
  float theta;
};