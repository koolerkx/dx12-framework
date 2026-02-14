struct SMAANeighborhoodCB {
  float4 rtMetrics;
  uint colorSrvIndex;
  uint blendSrvIndex;
  uint2 _pad;
};
ConstantBuffer<SMAANeighborhoodCB> g_SMAACB : register(b2);

#define SMAA_RT_METRICS g_SMAACB.rtMetrics
#include "smaa_common.hlsli"

struct PSIN {
  float4 position : SV_POSITION;
  float2 uv : TEXCOORD;
};

float4 main(PSIN input) : SV_TARGET {
  float4 offset;
  SMAANeighborhoodBlendingVS(input.uv, offset);
  return SMAANeighborhoodBlendingPS(input.uv, offset,
                                    g_Textures[g_SMAACB.colorSrvIndex],
                                    g_Textures[g_SMAACB.blendSrvIndex]);
}
