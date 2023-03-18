#include "Timer.h"
#include "camera.h"

// Initialization camera method
HRESULT Camera::InitCamera() {
  phi = (120.f) / 180.f * XM_PI;
  theta = (15.f) / 180.f * XM_PI;
  distanceToPoint = 1.6f;

  pointOfInterest = XMFLOAT3(0.0f, 0.0f, 0.0f);

  return S_OK;
}

// Update frame method
void Camera::Frame() {
  // animate camera
  //phi = XM_PI * (float)sin(Timer::GetInstance().Clock());
  //theta = XM_PI / 10.f * (float)sin(Timer::GetInstance().Clock() * 0.30);

  XMFLOAT3 pos = GetPos();
  
  float upTheta = theta + XM_PIDIV2;
  XMFLOAT3 up = XMFLOAT3(
    cosf(upTheta) * cosf(phi), 
    sinf(upTheta), 
    cosf(upTheta) * sinf(phi)
  );

  viewMatrix = DirectX::XMMatrixLookAtLH(
    DirectX::XMVectorSet(pos.x, pos.y, pos.z, 0.0f),
    DirectX::XMVectorSet(pointOfInterest.x, pointOfInterest.y, pointOfInterest.z, 0.0f),
    DirectX::XMVectorSet(up.x, up.y, up.z, 0.0f)
  );
}

void Camera::Move(float dx, float dy, float wheel) {
  phi -= dx / MOVEMENT_DOWNSHIFTING;

  theta += dy / MOVEMENT_DOWNSHIFTING;
  theta = min(max(theta, -XM_PIDIV2), XM_PIDIV2);

  distanceToPoint -= wheel / MOVEMENT_DOWNSHIFTING;
  distanceToPoint = max(distanceToPoint, 1.0f);
}

// Get view matrix method
void Camera::GetBaseViewMatrix(XMMATRIX& viewMatrix) {
  viewMatrix = this->viewMatrix;
};

XMFLOAT3 Camera::GetPos() {
  XMFLOAT3 pos = XMFLOAT3(
    cosf(theta) * cosf(phi), 
    sinf(theta), 
    cosf(theta) * sinf(phi));

  pos.x = pos.x * distanceToPoint + pointOfInterest.x;
  pos.y = pos.y * distanceToPoint + pointOfInterest.y;
  pos.z = pos.z * distanceToPoint + pointOfInterest.z;

  return pos;
};
