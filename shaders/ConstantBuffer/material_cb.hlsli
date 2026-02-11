// Override before include
// #define MATERIAL_CB_SLOT b3
// #include "ConstantBuffer/material_cb.hlsli"

#ifndef MATERIAL_CB_HLSLI
#define MATERIAL_CB_HLSLI

#ifndef MATERIAL_CB_SLOT
#define MATERIAL_CB_SLOT b3
#endif

struct MaterialData {
  uint albedoTextureIndex;
  uint normalTextureIndex;
  uint metallicRoughnessIndex;
  uint flags;
  float specularIntensity;
  float specularPower;
};
ConstantBuffer<MaterialData> g_MaterialData : register(MATERIAL_CB_SLOT);

#endif
