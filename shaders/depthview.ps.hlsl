#include "basic_type.hlsli"

cbuffer DepthViewCB : register(b2) {
  uint g_DepthSrvIndex;
  float g_NearPlane;
  float g_FarPlane;
  uint _padding;
}

SamplerState g_Samplers[] : register(s0, space0);

float4 main(float4 position : SV_POSITION, float2 uv : TEXCOORD) : SV_TARGET {
  float depth = g_Textures[g_DepthSrvIndex].Sample(g_Samplers[0], uv).r;

  // Linearize reverse-Z depth
  float linear_depth =
      g_NearPlane / (g_FarPlane - depth * (g_FarPlane - g_NearPlane));

  return float4(linear_depth, linear_depth, linear_depth, 1.0);
}
