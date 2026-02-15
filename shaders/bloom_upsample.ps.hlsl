struct BloomUpCB {
  uint sourceSrvIndex;
  float2 texelSize;
  float padding;
};
ConstantBuffer<BloomUpCB> g_CB : register(b2);

Texture2D g_Textures[] : register(t0, space1);
#include "ConstantBuffer/sampler.hlsli"

struct PSIN {
  float4 position : SV_POSITION;
  float2 uv : TEXCOORD;
};

float4 main(PSIN input) : SV_TARGET {
  Texture2D src = g_Textures[g_CB.sourceSrvIndex];
  SamplerState samp = g_Samplers[SAMPLER_LINEAR_CLAMP];
  float2 uv = input.uv;
  float2 offset = g_CB.texelSize * 0.5;

  float4 sum = 0;
  sum += src.Sample(samp, uv + float2(-offset.x * 2, 0)) * 1.0;
  sum += src.Sample(samp, uv + float2(+offset.x * 2, 0)) * 1.0;
  sum += src.Sample(samp, uv + float2(0, -offset.y * 2)) * 1.0;
  sum += src.Sample(samp, uv + float2(0, +offset.y * 2)) * 1.0;
  sum += src.Sample(samp, uv + float2(-offset.x, -offset.y)) * 2.0;
  sum += src.Sample(samp, uv + float2(+offset.x, +offset.y)) * 2.0;
  sum += src.Sample(samp, uv + float2(+offset.x, -offset.y)) * 2.0;
  sum += src.Sample(samp, uv + float2(-offset.x, +offset.y)) * 2.0;

  return sum / 12.0;
}
