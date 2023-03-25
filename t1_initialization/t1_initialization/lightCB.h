#include "constants.h"

cbuffer LightCB : register (b2)
{
  float4 cameraPos;
  int4 lightCount; // x - light count (max MAX_LIGHT_SOURCES)
  float4 lightPos[MAX_LIGHT_SOURCES];
  float4 lightColor[MAX_LIGHT_SOURCES];
  float4 ambientColor;
};
