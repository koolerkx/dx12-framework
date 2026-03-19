#include "ConstantBuffer/material_descriptor.hlsli"
#include "ConstantBuffer/object_cb.hlsli"

struct PSIN {
  float4 position : SV_POSITION;
  float2 uv : TEXCOORD;
  float4 color : BASE_COLOR;
  float4 overlayColor : OVERLAY_COLOR;
};

Texture2D g_Textures[] : register(t0, space1);
#include "ConstantBuffer/sampler.hlsli"

float4 main(PSIN input) : SV_TARGET {
  MaterialDescriptor mat = LoadMaterial(g_ObjectCB.materialDescriptorIndex);
  float4 texColor = g_Textures[mat.albedoTextureIndex].Sample(
      g_Samplers[mat.samplerIndex], input.uv);
  float4 finalColor = texColor * input.color * g_ObjectCB.color;
  finalColor.rgb += input.overlayColor.rgb * input.overlayColor.a;
  return finalColor;
}
