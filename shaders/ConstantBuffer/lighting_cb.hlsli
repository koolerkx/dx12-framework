#ifndef LIGHTING_CB_HLSLI
#define LIGHTING_CB_HLSLI

#ifndef LIGHTING_CB_SLOT
#define LIGHTING_CB_SLOT b2
#endif

struct LightingCB {
  float3 lightDirection;
  float lightIntensity;
  float3 directionalColor;
  float ambientIntensity;
  float3 ambientColor;
  float _padding;
};
ConstantBuffer<LightingCB> g_LightingCB : register(LIGHTING_CB_SLOT);

#endif
