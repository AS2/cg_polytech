#include "scene.h"

HRESULT Scene::Init(ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight) {
  // Init boxes
  HRESULT hr = box.Init(device, context, screenWidth, screenHeight, { L"./src/245.dds", L"./src/245_norm.dds", 256.f});
  if (FAILED(hr))
    return hr;

  // Init planes
  std::vector<XMFLOAT4> colors = {
    XMFLOAT4(1.f, 1.f, 1.f, 0.45f),
    XMFLOAT4(1.f, 1.f, 1.f, 0.67f),
    XMFLOAT4(0.f, 0.f, 1.f, 0.75f),
  };

  hr = planes.Init(device, context, screenWidth, screenHeight, 3, colors);
  if (FAILED(hr))
    return hr;

  // Init skybox
  hr = sb.Init(device, context, screenWidth, screenHeight);
  if (FAILED(hr))
    return hr;

  // Init lights
  lights = std::vector<Light>(4);
  hr = lights[0].Init(device, context, screenWidth, screenHeight, 
    XMFLOAT4(0.f, 1.f, 0.0f, 1.f), XMFLOAT4(0.f, 0.f, 1.0f, 1.f));
  hr = lights[1].Init(device, context, screenWidth, screenHeight, 
    XMFLOAT4(1.f, 0.f, 0.0f, 1.f), XMFLOAT4(0.f, 1.0f, 0.f, 1.f));
  hr = lights[2].Init(device, context, screenWidth, screenHeight, 
    XMFLOAT4(0.f, 0.f, 1.0f, 1.f), XMFLOAT4(1.0f, 0.f, 0.f, 1.f));
  hr = lights[3].Init(device, context, screenWidth, screenHeight,
    XMFLOAT4(1.f, 1.f, 1.0f, 1.f), XMFLOAT4(-1.0f, 0.f, 0.f, 1.f));

  return hr;
}

void Scene::Realese() {
  box.Realese();

  planes.Realese();

  sb.Realese();

  for (auto& light : lights)
    light.Realese();
}

void Scene::Render(ID3D11DeviceContext* context) {
  // render boxes
  box.Render(context);

  for (auto& light : lights)
    light.Render(context);

  // render skybox
  sb.Render(context);

  // render planes
  planes.Render(context);
}

bool Scene::FrameBoxes(ID3D11DeviceContext* context, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 cameraPos) {
  // Update world matrix angle of first cube
  auto duration = Timer::GetInstance().Clock();
  XMMATRIX worldMatrix = XMMatrixRotationY((float)duration * angle_velocity * 0.5f) *
    XMMatrixRotationZ((float)(sin(duration * angle_velocity * 0.30) * 0.25f));
  
  // Update world matrix angle of second cube
  box.Frame(context, worldMatrix, viewMatrix, projectionMatrix, cameraPos, lights);
  
  return S_OK;
}

bool Scene::FramePlanes(ID3D11DeviceContext* context, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 cameraPos) {
  auto duration = Timer::GetInstance().Clock();
  std::vector<XMMATRIX> worldMatricies = std::vector<XMMATRIX>(3);

  worldMatricies[0] = XMMatrixTranslation(1.25f, 0, (float)(sin(duration * 2) * -2.0));
  worldMatricies[1] = XMMatrixTranslation(-1.25f, 0, (float)(sin(duration * 2) * 2.0));
  worldMatricies[2] = XMMatrixTranslation(2.5f, 0, -1.25f);
  
  planes.Frame(context, worldMatricies, viewMatrix, projectionMatrix, cameraPos, lights);

  return S_OK;
}

bool Scene::Frame(ID3D11DeviceContext* context, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 cameraPos) {
  FrameBoxes(context, viewMatrix, projectionMatrix, cameraPos);
  
  FramePlanes(context, viewMatrix, projectionMatrix, cameraPos);
  
  sb.Frame(context, viewMatrix, projectionMatrix, cameraPos);


  for (auto& light : lights)
    light.Frame(context, viewMatrix, projectionMatrix, cameraPos);
  
  return true;
}

void Scene::Resize(int screenWidth, int screenHeight) {
  box.Resize(screenWidth, screenHeight);

  planes.Resize(screenWidth, screenHeight);

  sb.Resize(screenWidth, screenHeight);

  for (auto& light : lights)
    light.Resize(screenWidth, screenHeight);
};
