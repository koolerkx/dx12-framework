#include "ConstantBuffer/frame_cb.hlsli"

struct ChromaticAberrationCB {
  uint sceneSrvIndex;
  float intensity;
  uint _pad0;
  uint _pad1;
};
ConstantBuffer<ChromaticAberrationCB> g_ChromaticAberrationCB : register(b2);

Texture2D g_Textures[] : register(t0, space1);
#include "ConstantBuffer/sampler.hlsli"

struct PSIN {
  float4 position : SV_POSITION;
  float2 uv : TEXCOORD;
};

float4 main(PSIN input) : SV_TARGET {
  float2 dir = input.uv - 0.5;
  float offset = g_ChromaticAberrationCB.intensity * 0.03;

  float r = g_Textures[g_ChromaticAberrationCB.sceneSrvIndex]
                .SampleLevel(g_Samplers[SAMPLER_LINEAR_CLAMP], input.uv + dir * offset, 0)
                .r;
  float g = g_Textures[g_ChromaticAberrationCB.sceneSrvIndex]
                .SampleLevel(g_Samplers[SAMPLER_LINEAR_CLAMP], input.uv, 0)
                .g;
  float b = g_Textures[g_ChromaticAberrationCB.sceneSrvIndex]
                .SampleLevel(g_Samplers[SAMPLER_LINEAR_CLAMP], input.uv - dir * offset, 0)
                .b;

  return float4(r, g, b, 1.0);
}
