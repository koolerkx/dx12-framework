#include "basic_type.hlsli"

float4 main(BasicType input) : SV_TARGET {
  return g_Textures[g_MaterialData.albedoTextureIndex].Sample(g_Sampler, input.uv) * g_ObjectColor;
}
