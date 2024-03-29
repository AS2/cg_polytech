#pragma once

#include <windows.h>
#include <d3dcompiler.h>
#include <dxgi.h>
#include <d3d11.h>
#include <directxmath.h>
#include <string>
#include <vector>

#include "skybox.h"
#include "light.h"
#include "box.h"
#include "plane.h"
#include "timer.h"
#include "texture.h"

using namespace DirectX;

class Scene {
public:
  HRESULT Init(ID3D11Device* device, ID3D11DeviceContext* context, int screenWidth, int screenHeight);

  void Realese();

  void Resize(int screenWidth, int screenHeight);

  void Render(ID3D11DeviceContext* context);

  bool Frame(ID3D11DeviceContext* context, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 cameraPos);

  int GetName() {
    return box.GetCulledCount();
  };
private:
  bool FramePlanes(ID3D11DeviceContext* context, XMMATRIX viewMatrix, XMMATRIX projectionMatrix, XMFLOAT3 cameraPos);

  Box box;

  Plane planes;
  Light lights;
  
  Skybox sb;

  // Velocity of world matrix rotation
  float angle_velocity = 3.1415926f;
};
