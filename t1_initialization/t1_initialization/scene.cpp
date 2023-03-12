#include "scene.h"

HRESULT Scene::Init(ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight) {
  std::vector<XMFLOAT4> colors = {
    XMFLOAT4(1.f, 0.f, 0.f, 1.f),
    XMFLOAT4(0.f, 1.f, 0.f, 1.f),
    XMFLOAT4(0.f, 0.f, 1.f, 1.f),
  };

  HRESULT hr = boxes.Init(device, context, screenWidth, screenHeight, 2);
  if (FAILED(hr))
    return hr;

  hr = planes.Init(device, context, screenWidth, screenHeight, 3, colors);
  if (FAILED(hr))
    return hr;

  hr = sb.Init(device, context, screenWidth, screenHeight);

  return hr;
}

void Scene::Realese() {
  boxes.Realese();

  planes.Realese();

  sb.Realese();
}

void Scene::Render(ID3D11DeviceContext* context) {
  // render boxes
  boxes.Render(context);

  // render skybox
  sb.Render(context);

  // render planes
  planes.Render(context);
}

bool Scene::FrameBoxes(ID3D11DeviceContext* context, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 cameraPos) {
  // Update world matrix angle of first cube
  auto duration = Timer::GetInstance().Clock();
  std::vector<XMMATRIX> worldMatricies = std::vector<XMMATRIX>(2);

  worldMatricies[0] = XMMatrixRotationY((float)duration * angle_velocity) *
    XMMatrixRotationZ((float)(sin(duration * angle_velocity * 0.30) * 0.5));
  
  // Update world matrix angle of second cube
  worldMatricies[1] = XMMatrixRotationY((float)duration * angle_velocity * 0.25f) *
    XMMatrixTranslation((float)sin(duration) * 3.0f, 0.0f, (float)cos(duration) * 3.0f);
  boxes.Frame(context, worldMatricies, viewMatrix, projectionMatrix, cameraPos);
  
  return S_OK;
}

bool Scene::FramePlanes(ID3D11DeviceContext* context, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 cameraPos) {
  auto duration = Timer::GetInstance().Clock();
  std::vector<XMMATRIX> worldMatricies = std::vector<XMMATRIX>(3);

  worldMatricies[0] = XMMatrixTranslation(1.25f, 0, (float)(sin(duration * 2) * -2.0));
  worldMatricies[1] = XMMatrixTranslation(-1.25f, 0, (float)(sin(duration * 2) * 2.0));
  worldMatricies[2] = XMMatrixTranslation((float)sin(duration * angle_velocity * 0.25) * -4.5f, 0.0f, (float)cos(duration * angle_velocity * 0.25) * -4.5f);

  planes.Frame(context, worldMatricies, viewMatrix, projectionMatrix, cameraPos);

  return S_OK;
}

bool Scene::Frame(ID3D11DeviceContext* context, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 cameraPos) {
  FrameBoxes(context, viewMatrix, projectionMatrix, cameraPos);
  
  FramePlanes(context, viewMatrix, projectionMatrix, cameraPos);
  
  sb.Frame(context, viewMatrix, projectionMatrix, cameraPos);
  
  return true;
}

void Scene::Resize(int screenWidth, int screenHeight) {
  boxes.Resize(screenWidth, screenHeight);

  planes.Resize(screenWidth, screenHeight);

  sb.Resize(screenWidth, screenHeight);
};
