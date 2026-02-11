#include "ConstantBuffer/frame_cb.hlsli"
#include "ConstantBuffer/lighting_cb.hlsli"
#include "ConstantBuffer/material_cb.hlsli"
#include "ConstantBuffer/object_cb.hlsli"

struct PSIN {
  float4 position : SV_POSITION;
  float3 worldNormal : NORMAL;
  float2 uv : TEXCOORD0;
  float4 color : COLOR;
  float3 worldPos : TEXCOORD1;
};

Texture2D g_Textures[] : register(t0, space1);
SamplerState g_Samplers[] : register(s0, space0);

#include "ConstantBuffer/shadow_cb.hlsli"

float4 main(PSIN input) : SV_TARGET {
  float4 tex = g_Textures[g_MaterialData.albedoTextureIndex].Sample(
      g_Samplers[g_ObjectCB.samplerIndex], input.uv);

  float4 baseColor = tex * input.color * g_ObjectCB.color;

  if ((g_ObjectCB.flags & 2u) && baseColor.a < 0.5) {
    discard;
  }

  if (g_ObjectCB.flags & 1u) {
    float3 N = normalize(input.worldNormal);
    float3 L = normalize(-g_LightingCB.lightDirection);
    float NdotL = saturate(dot(N, L));

    float shadow = 1.0;
    if (g_ObjectCB.flags & 4u) {
      shadow = CalculateShadow(input.worldPos, N, input.position.xy);
    }

    float3 shadowTint = lerp(g_ShadowCB.shadowColor, float3(1, 1, 1), shadow);
    float3 diffuse = g_LightingCB.directionalColor *
                     g_LightingCB.lightIntensity * NdotL * shadowTint;
    float3 ambient = g_LightingCB.ambientColor * g_LightingCB.ambientIntensity;

    float3 V = normalize(g_FrameCB.cameraPos - input.worldPos);
    float3 H = normalize(L + V);
    float spec = pow(saturate(dot(N, H)), g_MaterialData.specularPower);
    float3 specular = g_LightingCB.directionalColor *
                      g_LightingCB.lightIntensity *
                      g_MaterialData.specularIntensity * spec * NdotL * shadow;

    float rim = pow(1.0 - saturate(dot(N, V)), g_MaterialData.rimPower);
    float3 rimLight =
        g_MaterialData.rimColor * g_MaterialData.rimIntensity * rim;
    if (g_MaterialData.flags & 4u) rimLight *= shadow;

    float3 pointDiffuse = float3(0, 0, 0);
    float3 pointSpecular = float3(0, 0, 0);
    if (g_LightingCB.pointLightCount > 0) {
      CalcPointLightContribution(
          N, V, input.worldPos, g_LightingCB.pointLightCount,
          g_MaterialData.specularIntensity, g_MaterialData.specularPower,
          pointDiffuse, pointSpecular);
    }

    baseColor.rgb = baseColor.rgb * (diffuse + ambient + pointDiffuse) +
                    specular + pointSpecular + rimLight;
  }

  return baseColor;
}
