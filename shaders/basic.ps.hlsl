#include "ConstantBuffer/material_cb.hlsli"
#include "ConstantBuffer/object_cb.hlsli"

struct PSIN {
  float4 position : SV_POSITION;
  float2 uv : TEXCOORD;
  float4 color : COLOR;
};

Texture2D g_Textures[] : register(t0, space1);
SamplerState g_Sampler : register(s0);

float4 main(PSIN input) : SV_TARGET {
  float4 tex =
      g_Textures[g_MaterialData.albedoTextureIndex].Sample(g_Sampler, input.uv);

  // alpha test (cutout)
  if (tex.a < 0.5) {
    discard;
  }

  return tex * input.color * g_ObjectCB.color;
}
