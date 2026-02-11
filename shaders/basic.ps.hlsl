#include "ConstantBuffer/lighting_cb.hlsli"
#include "ConstantBuffer/material_cb.hlsli"
#include "ConstantBuffer/object_cb.hlsli"
#include "ConstantBuffer/shadow_cb.hlsli"

struct PSIN {
  float4 position : SV_POSITION;
  float3 worldNormal : NORMAL;
  float2 uv : TEXCOORD0;
  float4 color : COLOR;
  float3 worldPos : TEXCOORD1;
};

Texture2D g_Textures[] : register(t0, space1);
SamplerState g_Samplers[] : register(s0, space0);

float3 ProjectToShadowSpace(float3 worldPos, float3 worldNormal) {
  float3 biasedPos = worldPos + worldNormal * g_ShadowCB.shadowNormalBias;
  float4 shadowPos = mul(float4(biasedPos, 1.0f), g_ShadowCB.lightViewProj);
  float3 projCoords = shadowPos.xyz / shadowPos.w;
  projCoords.x = projCoords.x * 0.5 + 0.5;
  projCoords.y = projCoords.y * -0.5 + 0.5;
  return projCoords;
}

float CalculateShadowHard(float3 worldPos, float3 worldNormal) {
  float3 projCoords = ProjectToShadowSpace(worldPos, worldNormal);

  if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 ||
      projCoords.y < 0.0 || projCoords.y > 1.0)
    return 1.0;

  float depth = g_Textures[g_ShadowCB.shadowMapIndex]
                    .Sample(g_Samplers[SHADOW_SAMPLER_INDEX], projCoords.xy)
                    .r;
  return (projCoords.z - g_ShadowCB.shadowBias > depth) ? 0.0 : 1.0;
}

float CalculateShadowPCF3x3(float3 worldPos, float3 worldNormal) {
  float3 projCoords = ProjectToShadowSpace(worldPos, worldNormal);

  if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 ||
      projCoords.y < 0.0 || projCoords.y > 1.0)
    return 1.0;

  float texelSize = 1.0 / float(g_ShadowCB.shadowMapResolution);
  float shadow = 0.0;

  [unroll] for (int x = -1; x <= 1; ++x) {
    [unroll] for (int y = -1; y <= 1; ++y) {
      float2 offset = float2(x, y) * texelSize;
      float depth =
          g_Textures[g_ShadowCB.shadowMapIndex]
              .Sample(g_Samplers[SHADOW_SAMPLER_INDEX], projCoords.xy + offset)
              .r;
      shadow += (projCoords.z - g_ShadowCB.shadowBias > depth) ? 0.0 : 1.0;
    }
  }
  return shadow / 9.0;
}

float CalculateShadow(float3 worldPos, float3 worldNormal) {
  if (g_ShadowCB.shadowAlgorithm == SHADOW_ALGO_PCF3X3)
    return CalculateShadowPCF3x3(worldPos, worldNormal);
  return CalculateShadowHard(worldPos, worldNormal);
}

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
      shadow = CalculateShadow(input.worldPos, N);
    }

    float3 shadowTint = lerp(g_ShadowCB.shadowColor, float3(1, 1, 1), shadow);
    float3 diffuse = g_LightingCB.directionalColor *
                     g_LightingCB.lightIntensity * NdotL * shadowTint;
    float3 ambient = g_LightingCB.ambientColor * g_LightingCB.ambientIntensity;
    baseColor.rgb *= (diffuse + ambient);
  }

  return baseColor;
}
