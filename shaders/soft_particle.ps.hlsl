#include "ConstantBuffer/object_cb.hlsli"
#include "ConstantBuffer/material_cb.hlsli"
#include "ConstantBuffer/frame_cb.hlsli"
#include "ConstantBuffer/custom_cb.hlsli"

Texture2D g_Textures[] : register(t0, space1);
#include "ConstantBuffer/sampler.hlsli"

struct SoftParticleCB {
  uint depthSrvIndex;
  float emissiveIntensity;
  float softDistance;
  uint _pad;
};
ConstantBuffer<SoftParticleCB> g_SoftParticleCB : register(CUSTOM_CB_SLOT);

struct PSIN {
  float4 position : SV_POSITION;
  float2 uv : TEXCOORD;
  float4 color : COLOR;
};

float4 main(PSIN input) : SV_TARGET {
  float4 texColor = g_Textures[g_MaterialData.albedoTextureIndex].Sample(
      g_Samplers[g_ObjectCB.samplerIndex], input.uv);

  float4 finalColor = texColor * input.color;
  finalColor.rgb *= g_SoftParticleCB.emissiveIntensity;

  float2 screenUV = input.position.xy / g_FrameCB.screenSize;
  float sceneDepth = g_Textures[g_SoftParticleCB.depthSrvIndex].SampleLevel(
      g_Samplers[SAMPLER_LINEAR_CLAMP], screenUV, 0).w;
  float particleDepth = 1.0 / input.position.w;

  if (sceneDepth > 0.0) {
    float depthDiff = sceneDepth - particleDepth;
    float softFade = saturate(depthDiff / g_SoftParticleCB.softDistance);
    finalColor.a *= softFade;
  }

  if (finalColor.a < 0.01f) {
    discard;
  }

  return finalColor;
}
