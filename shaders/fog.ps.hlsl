#include "ConstantBuffer/frame_cb.hlsli"

struct FogCB {
  uint sceneSrvIndex;
  uint depthSrvIndex;
  float density;
  float heightFalloff;
  float baseHeight;
  float maxDistance;
  uint _pad0;
  uint _pad1;
  float3 fogColor;
  uint _pad2;
};
ConstantBuffer<FogCB> g_FogCB : register(b2);

Texture2D g_Textures[] : register(t0, space1);
SamplerState g_Samplers[] : register(s0, space0);

struct PSIN {
  float4 position : SV_POSITION;
  float2 uv : TEXCOORD;
};

float3 ReconstructViewPosition(float2 uv, float viewZ) {
  float2 ndc = uv * 2.0 - 1.0;
  ndc.y = -ndc.y;
  float4 viewRay = mul(float4(ndc, 1.0, 1.0), g_FrameCB.invProj);
  viewRay.xyz /= viewRay.w;
  return viewRay.xyz * (viewZ / viewRay.z);
}

float4 main(PSIN input) : SV_TARGET {
  float3 sceneColor = g_Textures[g_FogCB.sceneSrvIndex]
                          .SampleLevel(g_Samplers[4], input.uv, 0)
                          .rgb;

  float viewZ = g_Textures[g_FogCB.depthSrvIndex]
                    .SampleLevel(g_Samplers[4], input.uv, 0)
                    .w;

  if (viewZ <= 0.0 || viewZ > g_FogCB.maxDistance * 2.0) {
    return float4(sceneColor, 1.0);
  }

  float3 viewPos = ReconstructViewPosition(input.uv, viewZ);
  float3 worldPos = mul(float4(viewPos, 1.0), g_FrameCB.invView).xyz;

  float distance = min(length(viewPos), g_FogCB.maxDistance);

  float distanceFog = exp(-g_FogCB.density * distance);
  float heightFog =
      exp(-g_FogCB.heightFalloff * max(g_FogCB.baseHeight - worldPos.y, 0.0));
  float fogFactor = saturate(distanceFog * heightFog);

  float3 result = lerp(g_FogCB.fogColor, sceneColor, fogFactor);

  return float4(result, 1.0);
}
