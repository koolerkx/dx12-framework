#include "ConstantBuffer/object_cb.hlsli"
#include "ConstantBuffer/material_cb.hlsli"

Texture2D g_Textures[] : register(t0, space1);
SamplerState g_Samplers[] : register(s0, space0);

struct PSIN {
  float4 position : SV_POSITION;
  float2 uv : TEXCOORD;
  float4 color : COLOR;
};

float4 main(PSIN input) : SV_TARGET {
  float4 texColor = g_Textures[g_MaterialData.albedoTextureIndex].Sample(
      g_Samplers[g_ObjectCB.samplerIndex], input.uv);

  float4 finalColor = texColor * input.color;

  if ((g_MaterialData.flags & 1u) && finalColor.a < 0.01f) {
    discard;
  }

  return finalColor;
}
