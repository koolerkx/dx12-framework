struct SkyboxCB {
  uint cubemapSrvIndex;
  float3 fallbackColor;
};
ConstantBuffer<SkyboxCB> g_SkyboxCB : register(b2);

TextureCube g_CubeTextures[] : register(t0, space1);
#include "ConstantBuffer/sampler.hlsli"

struct PSIN {
  float4 position : SV_POSITION;
  float3 local_pos : TEXCOORD;
};

float4 main(PSIN input) : SV_TARGET {
  if (g_SkyboxCB.cubemapSrvIndex == 0xFFFFFFFF)
    return float4(g_SkyboxCB.fallbackColor, 1.0);

  float3 color =
      g_CubeTextures[g_SkyboxCB.cubemapSrvIndex]
          .Sample(g_Samplers[SAMPLER_POINT_WRAP], normalize(input.local_pos))
          .rgb;
  return float4(color, 1.0);
}
