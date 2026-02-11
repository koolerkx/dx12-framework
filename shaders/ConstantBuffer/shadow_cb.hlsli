#ifndef SHADOW_CB_HLSLI
#define SHADOW_CB_HLSLI

#ifndef SHADOW_CB_SLOT
#define SHADOW_CB_SLOT b4
#endif

#define SHADOW_SAMPLER_INDEX 3

#define SHADOW_ALGO_HARD 0
#define SHADOW_ALGO_PCF3X3 1

struct ShadowCB {
  row_major float4x4 lightViewProj;
  uint shadowMapIndex;
  float shadowBias;
  float shadowNormalBias;
  uint shadowMapResolution;
  uint shadowAlgorithm;
  float3 shadowColor;
};
ConstantBuffer<ShadowCB> g_ShadowCB : register(SHADOW_CB_SLOT);

#endif
