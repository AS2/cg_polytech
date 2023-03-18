#pragma once

#include <directxmath.h>

#define MAX_LIGHT_SOURCES 10  // Additional - fix constant in shaders

using namespace DirectX;

struct WorldMatrixBuffer {
  XMMATRIX worldMatrix;
  XMFLOAT4 color;
};

struct SceneMatrixBuffer {
  XMMATRIX viewProjectionMatrix;
};

struct TexVertex
{
  XMFLOAT3 pos;       // positional coords
  XMFLOAT2 uv;        // texture coords
  XMFLOAT3 normal;    // normal vec
  XMFLOAT3 tangent;   // tangent vec
};

struct LightableSceneMatrixBuffer {
  XMMATRIX viewProjectionMatrix;
  XMFLOAT4 cameraPos;
  XMINT4 lightCount;
  XMFLOAT4 lightPos[MAX_LIGHT_SOURCES];
  XMFLOAT4 lightColor[MAX_LIGHT_SOURCES];
  XMFLOAT4 ambientColor;
};

struct SimpleVertex
{
  float x, y, z;      // positional coords
};

// Only Skybox buffers
struct SBWorldMatrixBuffer {
  XMMATRIX worldMatrix;
  XMFLOAT4 size;
};

struct SBSceneMatrixBuffer {
  XMMATRIX viewProjectionMatrix;
  XMFLOAT4 cameraPos;
};
