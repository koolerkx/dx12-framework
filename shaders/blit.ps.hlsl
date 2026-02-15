struct BlitCB {
  uint srcSrvIndex;
};
ConstantBuffer<BlitCB> g_BlitCB : register(b2);

Texture2D g_Textures[] : register(t0, space1);
#include "ConstantBuffer/sampler.hlsli"

struct PSIN {
  float4 position : SV_POSITION;
  float2 uv : TEXCOORD;
};

float4 main(PSIN input) : SV_TARGET {
  return g_Textures[g_BlitCB.srcSrvIndex].Sample(g_Samplers[SAMPLER_POINT_WRAP], input.uv);
}
