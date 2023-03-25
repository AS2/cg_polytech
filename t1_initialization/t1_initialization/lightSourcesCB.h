#include "constants.h"

struct LightGeomBuffer
{
  float4x4 worldMatrix;
  float4 color;
};

cbuffer WorldMatrixBuffer : register (b0)
{
  LightGeomBuffer lightsGeomBuffer[MAX_LIGHT_SOURCES];
};

cbuffer SceneMatrixBuffer : register (b1)
{
  float4x4 viewProjectionMatrix;
};
