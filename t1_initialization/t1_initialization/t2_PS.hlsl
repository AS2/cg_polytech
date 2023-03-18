#include "lightCalc.h"

cbuffer WorldMatrixBuffer : register (b0)
{
  float4x4 worldMatrix;
  float4 shine; // x - specular power
};

Texture2D tex : register (t0);
Texture2D normal : register (t1);

SamplerState smplr : register (s0);

struct PS_INPUT
{
  float4 position : SV_POSITION;
  float4 worldPos : POSITION;
  float2 uv : TEXCOORD;
  float3 normal : NORMAL;
  float3 tangent : TANGENT;
};

float4 main(PS_INPUT input) : SV_Target0{
  // step 1  - count ambient color
  float3 ambient = ambientColor.xyz * tex.Sample(smplr, input.uv).xyz;
  
  // step 2 - calculate normal
  float3 binorm = normalize(cross(input.normal, input.tangent));
  float3 localNorm = normal.Sample(smplr, input.uv).xyz * 2.0 - 1.0;
  float3 norm = localNorm.x * normalize(input.tangent) + localNorm.y * binorm + localNorm.z * normalize(input.normal);

  // step 3 - return final color with lights
  return float4(CalculateColor(ambient, norm, input.worldPos.xyz, shine.x, false), 1.0);
}
