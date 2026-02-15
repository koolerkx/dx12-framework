struct DepthViewCB {
  uint depthSrvIndex;
  float nearPlane;
  float farPlane;
  uint _padding;
};
ConstantBuffer<DepthViewCB> g_DepthViewCB : register(b2);

Texture2D g_Textures[] : register(t0, space1);
#include "ConstantBuffer/sampler.hlsli"

struct PSIN {
  float4 position : SV_POSITION;
  float2 uv : TEXCOORD;
};

float4 main(PSIN input) : SV_TARGET {
  float depth =
      g_Textures[g_DepthViewCB.depthSrvIndex].Sample(g_Samplers[SAMPLER_POINT_WRAP], input.uv).r;

  // Linearize reverse-Z depth
  float linear_depth = g_DepthViewCB.nearPlane /
      (g_DepthViewCB.farPlane - depth * (g_DepthViewCB.farPlane - g_DepthViewCB.nearPlane));

  return float4(linear_depth, linear_depth, linear_depth, 1.0);
}
