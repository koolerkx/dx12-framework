#include "ConstantBuffer/material_cb.hlsli"
#include "ConstantBuffer/object_cb.hlsli"

struct PSIN {
  float4 position : SV_POSITION;
  float2 uv : TEXCOORD;
  float4 color : COLOR;
};

Texture2D g_Textures[] : register(t0, space1);
#include "ConstantBuffer/sampler.hlsli"

float4 main(PSIN input) : SV_TARGET {
  float4 texColor = g_Textures[g_MaterialData.albedoTextureIndex].Sample(
      g_Samplers[g_ObjectCB.samplerIndex], input.uv);
  return texColor * input.color * g_ObjectCB.color;
}
