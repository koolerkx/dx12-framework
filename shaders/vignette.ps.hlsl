#include "ConstantBuffer/frame_cb.hlsli"

struct VignetteCB {
  uint sceneSrvIndex;
  float intensity;
  float radius;
  float softness;
  float roundness;
  float aspectRatio;
  uint _pad0;
  uint _pad1;
  float3 vignetteColor;
  uint _pad2;
};
ConstantBuffer<VignetteCB> g_VignetteCB : register(b2);

Texture2D g_Textures[] : register(t0, space1);
#include "ConstantBuffer/sampler.hlsli"

struct PSIN {
  float4 position : SV_POSITION;
  float2 uv : TEXCOORD;
};

float4 main(PSIN input) : SV_TARGET {
  float3 sceneColor = g_Textures[g_VignetteCB.sceneSrvIndex]
                          .SampleLevel(g_Samplers[SAMPLER_LINEAR_CLAMP], input.uv, 0)
                          .rgb;

  float2 offset = input.uv - 0.5;
  offset.x *= lerp(1.0, g_VignetteCB.aspectRatio, g_VignetteCB.roundness);

  float dist = length(offset);
  float vignette = smoothstep(g_VignetteCB.radius, g_VignetteCB.radius - g_VignetteCB.softness, dist);
  float factor = lerp(1.0, vignette, g_VignetteCB.intensity);

  float3 result = lerp(g_VignetteCB.vignetteColor, sceneColor, factor);

  return float4(result, 1.0);
}
