#include "basic_type.hlsli"

cbuffer BlitCB : register(b2) {
  uint g_SrcSrvIndex;
}

SamplerState g_Samplers[] : register(s0, space0);

float4 main(float4 position : SV_POSITION, float2 uv : TEXCOORD) : SV_TARGET {
  return g_Textures[g_SrcSrvIndex].Sample(g_Samplers[0], uv);
}
