#pragma once

#include <directxmath.h>

#include "constants.h"

using namespace DirectX;


// common buffers structures
struct WorldMatrixBuffer {
  XMMATRIX worldMatrix;
  XMFLOAT4 color;
};

struct SceneMatrixBuffer {
  XMMATRIX viewProjectionMatrix;
};

// Boxes buffers structures
struct BoxSceneMatrixBuffer {
  XMMATRIX viewProjectionMatrix;
  XMFLOAT4 planes[6];
};

struct CullParams {
  XMINT4 numShapes; // x - objects count;
  XMFLOAT4 bbMin[MAX_CUBES];
  XMFLOAT4 bbMax[MAX_CUBES];
};

struct GeomBuffer {
  XMMATRIX worldMatrix;
  XMMATRIX norm;
  XMFLOAT4 params;
};

struct BoxModel {
  XMFLOAT4 pos;
  XMFLOAT4 params;
};

struct TexVertex
{
  XMFLOAT3 pos;       // positional coords
  XMFLOAT2 uv;        // texture coords
  XMFLOAT3 normal;    // normal vec
  XMFLOAT3 tangent;   // tangent vec
};

struct LightableCB {
  XMFLOAT4 cameraPos;
  XMINT4 lightCount;
  XMFLOAT4 lightPos[MAX_LIGHT_SOURCES];
  XMFLOAT4 lightColor[MAX_LIGHT_SOURCES];
  XMFLOAT4 ambientColor;
};

// Plane special structure
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
