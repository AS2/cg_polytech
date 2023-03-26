#include "scene.h"

HRESULT Scene::Init(ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight) {
  // Init boxes
  std::vector<XMFLOAT4> boxPositions = std::vector<XMFLOAT4>(MAX_CUBES);
  for (int i = 0; i < MAX_CUBES; i++) {
    boxPositions[i] = XMFLOAT4(
      (rand() / (float)(RAND_MAX + 1) * SCENE_SIZE - SCENE_SIZE / 2.f),
      (rand() / (float)(RAND_MAX + 1) * SCENE_SIZE - SCENE_SIZE / 2.f),
      (rand() / (float)(RAND_MAX + 1) * SCENE_SIZE - SCENE_SIZE / 2.f), 1.f);
  }
  HRESULT hr = box.Init(device, context, screenWidth, screenHeight, {{ L"./src/245.dds", L"./src/hah.dds"}, L"./src/245_norm.dds", 256.f }, boxPositions);
  if (FAILED(hr))
    return hr;

  // Init planes
  std::vector<XMFLOAT4> planeColors = {
    XMFLOAT4(1.f, 1.f, 1.f, 0.45f),
    XMFLOAT4(1.f, 1.f, 1.f, 0.67f),
    XMFLOAT4(0.f, 0.f, 1.f, 0.75f),
  };

  hr = planes.Init(device, context, screenWidth, screenHeight, 3, planeColors);
  if (FAILED(hr))
    return hr;

  // Init skybox
  hr = sb.Init(device, context, screenWidth, screenHeight);
  if (FAILED(hr))
    return hr;

  // Init lights
  
  std::vector<XMFLOAT4> colors = std::vector<XMFLOAT4>(MAX_LIGHT_SOURCES);
  std::vector<XMFLOAT4> positions = std::vector<XMFLOAT4>(MAX_LIGHT_SOURCES);

  for (int i = 0; i < MAX_LIGHT_SOURCES; i++) {
    colors[i] = XMFLOAT4(
      (float)(0.5f + rand() / (float)(RAND_MAX + 1) * 0.5f),
      (float)(0.5f + rand() / (float)(RAND_MAX + 1) * 0.5f),
      (float)(0.5f + rand() / (float)(RAND_MAX + 1) * 0.5f), 1.f);
    positions[i] = XMFLOAT4(
      (rand() / (float)(RAND_MAX + 1) * SCENE_SIZE - SCENE_SIZE / 2.f),
      (rand() / (float)(RAND_MAX + 1) * SCENE_SIZE - SCENE_SIZE / 2.f),
      (rand() / (float)(RAND_MAX + 1) * SCENE_SIZE - SCENE_SIZE / 2.f), 1.f);
  }
  hr = lights.Init(device, context, screenWidth, screenHeight, colors, positions);
  
  return hr;
}

void Scene::Realese() {
  box.Realese();

  planes.Realese();

  sb.Realese();

  lights.Realese();
}

void Scene::Render(ID3D11DeviceContext* context) {
  // render boxes
  box.Render(context);

  lights.Render(context);

  // render skybox
  sb.Render(context);

  // render planes
  planes.Render(context);
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
  box.Frame(context, viewMatrix, projectionMatrix, cameraPos, lights);
  
  FramePlanes(context, viewMatrix, projectionMatrix, cameraPos);
  
  sb.Frame(context, viewMatrix, projectionMatrix, cameraPos);

  lights.Frame(context, viewMatrix, projectionMatrix, cameraPos);
  
  return true;
}

void Scene::Resize(int screenWidth, int screenHeight) {
  box.Resize(screenWidth, screenHeight);

  planes.Resize(screenWidth, screenHeight);

  sb.Resize(screenWidth, screenHeight);

  lights.Resize(screenWidth, screenHeight);
};
