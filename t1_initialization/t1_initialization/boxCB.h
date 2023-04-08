#include "constants.h"
#include "lightCalc.h"

struct BoxGeomBuffer
{
  float4x4 worldMatrix;
  float4x4 norm;
  float4 boxParams; // x - specular power, y - rotation speed, z - texture id, w - normal map presence
};

cbuffer BoxGeomBuffers : register (b0)
{
  BoxGeomBuffer geomBuffers[MAX_CUBES];
};

cbuffer SceneCB : register (b1)
{
  float4x4 viewProjectionMatrix;
  float4 planes[6]; // x - index
};

cbuffer IndexBuffer : register(b2)
{
  uint4 objectID[MAX_CUBES];
}

