struct SMAAEdgeCB {
  float4 rtMetrics;
  uint colorSrvIndex;
  uint3 _pad;
};
ConstantBuffer<SMAAEdgeCB> g_SMAACB : register(b2);

#define SMAA_RT_METRICS g_SMAACB.rtMetrics
#include "smaa_common.hlsli"

struct PSIN {
  float4 position : SV_POSITION;
  float2 uv : TEXCOORD;
};

float4 main(PSIN input) : SV_TARGET {
  float4 offset[3];
  SMAAEdgeDetectionVS(input.uv, offset);
  float2 edges = SMAALumaEdgeDetectionPS(input.uv, offset,
                                         g_Textures[g_SMAACB.colorSrvIndex]);
  return float4(edges, 0.0, 0.0);
}
