#include "ConstantBuffer/frame_cb.hlsli"
#include "ConstantBuffer/lighting_cb.hlsli"
#include "ConstantBuffer/material_cb.hlsli"
#include "ConstantBuffer/material_descriptor.hlsli"
#include "ConstantBuffer/object_cb.hlsli"

struct PSIN {
  float4 position : SV_POSITION;
  float3 worldNormal : NORMAL;
  float2 uv : TEXCOORD0;
  float4 color : COLOR;
  float3 worldPos : TEXCOORD1;
};

Texture2D g_Textures[] : register(t0, space1);
#include "ConstantBuffer/sampler.hlsli"

#include "ConstantBuffer/shadow_cb.hlsli"

float4 main(PSIN input) : SV_TARGET {
  uint albedoIdx;
  uint matFlags;
  float specIntensity;
  float specPow;
  float rimInt;
  float rimPow;
  float3 rimCol;
  uint samplerIdx;

  if (UseBindlessMaterial(g_ObjectCB.materialDescriptorIndex)) {
    MaterialDescriptor mat = LoadMaterial(g_ObjectCB.materialDescriptorIndex);
    albedoIdx     = mat.albedoTextureIndex;
    matFlags      = mat.flags;
    specIntensity = mat.specularIntensity;
    specPow       = mat.specularPower;
    rimInt        = mat.rimIntensity;
    rimPow        = mat.rimPower;
    rimCol        = mat.rimColor;
    samplerIdx    = mat.samplerIndex;
  } else {
    albedoIdx     = g_MaterialData.albedoTextureIndex;
    matFlags      = g_MaterialData.flags;
    specIntensity = g_MaterialData.specularIntensity;
    specPow       = g_MaterialData.specularPower;
    rimInt        = g_MaterialData.rimIntensity;
    rimPow        = g_MaterialData.rimPower;
    rimCol        = g_MaterialData.rimColor;
    samplerIdx    = g_ObjectCB.samplerIndex;
  }

  float4 tex = g_Textures[albedoIdx].Sample(
      g_Samplers[samplerIdx], input.uv);

  float4 baseColor = tex * input.color * g_ObjectCB.color;

  if ((g_ObjectCB.flags & OBJECT_FLAG_OPAQUE) && baseColor.a < 0.5) {
    discard;
  }

  if (g_ObjectCB.flags & OBJECT_FLAG_LIT) {
    float3 N = normalize(input.worldNormal);
    float3 L = normalize(-g_LightingCB.lightDirection);
    float NdotL = saturate(dot(N, L));

    float shadow = 1.0;
    if (g_ObjectCB.flags & OBJECT_FLAG_RECEIVE_SHADOW) {
      shadow = CalculateShadow(input.worldPos, N, input.position.xy);
    }

    float3 shadowTint = lerp(g_ShadowCB.shadowColor, float3(1, 1, 1), shadow);
    float3 diffuse = g_LightingCB.directionalColor *
                     g_LightingCB.lightIntensity * NdotL * shadowTint;
    float3 ambient = g_LightingCB.ambientColor * g_LightingCB.ambientIntensity;

    float3 V = normalize(g_FrameCB.cameraPos - input.worldPos);
    float3 H = normalize(L + V);
    float spec = pow(saturate(dot(N, H)), specPow);
    float3 specular = g_LightingCB.directionalColor *
                      g_LightingCB.lightIntensity *
                      specIntensity * spec * NdotL * shadow;

    float rim = pow(1.0 - saturate(dot(N, V)), rimPow);
    float3 rimLight = rimCol * rimInt * rim;
    if (matFlags & MATERIAL_FLAG_RIM_SHADOW_AFFECTED)
      rimLight *= shadow;

    float3 pointDiffuse = float3(0, 0, 0);
    float3 pointSpecular = float3(0, 0, 0);
    if (g_LightingCB.pointLightCount > 0) {
      CalcPointLightContribution(
          N, V, input.worldPos, g_LightingCB.pointLightCount,
          specIntensity, specPow,
          pointDiffuse, pointSpecular);
    }

    float ao = 1.0;
    if (g_LightingCB.ssaoSrvIndex != 0xFFFFFFFF) {
      float2 screenUV = input.position.xy / g_FrameCB.screenSize;
      ao = g_Textures[g_LightingCB.ssaoSrvIndex]
               .Sample(g_Samplers[SAMPLER_LINEAR_CLAMP], screenUV)
               .r;
    }

    baseColor.rgb = baseColor.rgb * ((diffuse + ambient + pointDiffuse) * ao) +
                    specular + pointSpecular + rimLight;
  }

  return baseColor;
}
