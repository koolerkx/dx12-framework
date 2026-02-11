#ifndef SHADOW_CB_HLSLI
#define SHADOW_CB_HLSLI

#ifndef SHADOW_CB_SLOT
#define SHADOW_CB_SLOT b4
#endif

#define SHADOW_SAMPLER_INDEX 3

struct ShadowCB {
  row_major float4x4 lightViewProj;
  uint shadowMapIndex;
  float shadowBias;
  float shadowNormalBias;
  uint shadowMapResolution;
};
ConstantBuffer<ShadowCB> g_ShadowCB : register(SHADOW_CB_SLOT);

#endif
