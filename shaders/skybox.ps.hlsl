#include "ConstantBuffer/object_cb.hlsli"

struct SkyboxCB {
  uint cubemapSrvIndex;
  uint3 _padding;
};
ConstantBuffer<SkyboxCB> g_SkyboxCB : register(b2);

TextureCube g_CubeTextures[] : register(t0, space1);
SamplerState g_Samplers[] : register(s0, space0);

struct PSIN {
  float4 position : SV_POSITION;
  float3 local_pos : TEXCOORD;
};

float4 main(PSIN input) : SV_TARGET {
  if (g_SkyboxCB.cubemapSrvIndex == 0xFFFFFFFF) return g_ObjectCB.color;

  float3 color = g_CubeTextures[g_SkyboxCB.cubemapSrvIndex]
                     .Sample(g_Samplers[0], normalize(input.local_pos))
                     .rgb;
  return float4(color, 1.0);
}
