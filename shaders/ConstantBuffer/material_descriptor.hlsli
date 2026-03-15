#ifndef MATERIAL_DESCRIPTOR_HLSLI
#define MATERIAL_DESCRIPTOR_HLSLI

#define MATERIAL_FLAG_ALPHA_TEST (1u << 0)
#define MATERIAL_FLAG_DOUBLE_SIDED (1u << 1)
#define MATERIAL_FLAG_RIM_SHADOW_AFFECTED (1u << 2)
#define MATERIAL_FLAG_HAS_NORMAL_MAP (1u << 3)
#define MATERIAL_FLAG_HAS_METALLIC_ROUGHNESS (1u << 4)
#define MATERIAL_FLAG_HAS_EMISSIVE (1u << 5)

struct MaterialDescriptor {
  uint albedoTextureIndex;
  uint normalTextureIndex;
  uint metallicRoughnessIndex;
  uint emissiveTextureIndex;

  uint flags;
  float specularIntensity;
  float specularPower;
  float rimIntensity;

  float rimPower;
  float3 rimColor;

  float metallicFactor;
  float roughnessFactor;
  uint samplerIndex;
  float _pad0;

  float3 emissiveFactor;
  float _pad1;
};

StructuredBuffer<MaterialDescriptor> g_Materials : register(t0, space5);

MaterialDescriptor LoadMaterial(uint materialIndex) {
  return g_Materials[materialIndex];
}

#endif
