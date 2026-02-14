struct SMAABlendWeightCB {
  float4 rtMetrics;
  uint edgesSrvIndex;
  uint areaSrvIndex;
  uint searchSrvIndex;
  uint _pad;
};
ConstantBuffer<SMAABlendWeightCB> g_SMAACB : register(b2);

#define SMAA_RT_METRICS g_SMAACB.rtMetrics
#include "smaa_common.hlsli"

struct PSIN {
  float4 position : SV_POSITION;
  float2 uv : TEXCOORD;
};

float4 main(PSIN input) : SV_TARGET {
  float2 pixcoord;
  float4 offset[3];
  SMAABlendingWeightCalculationVS(input.uv, pixcoord, offset);
  return SMAABlendingWeightCalculationPS(
      input.uv, pixcoord, offset, g_Textures[g_SMAACB.edgesSrvIndex],
      g_Textures[g_SMAACB.areaSrvIndex], g_Textures[g_SMAACB.searchSrvIndex],
      float4(0, 0, 0, 0));
}
