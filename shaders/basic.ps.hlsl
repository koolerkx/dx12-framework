#include "ConstantBuffer/frame_cb.hlsli"
#include "ConstantBuffer/lighting_cb.hlsli"
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
  MaterialDescriptor mat = LoadMaterial(g_ObjectCB.materialDescriptorIndex);

  float4 tex = g_Textures[mat.albedoTextureIndex].Sample(
      g_Samplers[mat.samplerIndex], input.uv);

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
    float spec = pow(saturate(dot(N, H)), mat.specularPower);
    float3 specular = g_LightingCB.directionalColor *
                      g_LightingCB.lightIntensity * mat.specularIntensity *
                      spec * NdotL * shadow;

    float rim = pow(1.0 - saturate(dot(N, V)), mat.rimPower);
    float3 rimLight = mat.rimColor * mat.rimIntensity * rim;
    if (mat.flags & MATERIAL_FLAG_RIM_SHADOW_AFFECTED) rimLight *= shadow;

    float3 pointDiffuse = float3(0, 0, 0);
    float3 pointSpecular = float3(0, 0, 0);
    if (g_LightingCB.pointLightCount > 0) {
      CalcPointLightContribution(N, V, input.worldPos,
                                 g_LightingCB.pointLightCount,
                                 mat.specularIntensity, mat.specularPower,
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
