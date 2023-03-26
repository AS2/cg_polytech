Texture2D sourceTexture : register(t0);
SamplerState Sampler : register(s0);

cbuffer PostEffectConstantBuffer : register(b0) {
  float4 params; // x, y, z - parts of each color channel
}

struct PS_INPUT
{
  float4 pos : SV_POSITION;
  float2 tex : TEXCOORD;
};

float4 main(PS_INPUT input) : SV_TARGET{
    float3 color = sourceTexture.Sample(Sampler, input.tex).xyz;
    return float4(color.x * params.x, color.y * params.y, color.z * params.z, 1.0);
}
