#include "basic_type.hlsli"

float4 main(BasicType input) : SV_TARGET {
  float4 tex =
      g_Textures[g_MaterialData.albedoTextureIndex].Sample(g_Sampler, input.uv);

  // alpha test（cutout）
  if (tex.a < 0.5) {
    discard;
  }

  return tex * g_ObjectColor;
}
