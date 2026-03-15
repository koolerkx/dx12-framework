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
  float3 worldTangent : TEXCOORD2;
  float3 worldBitangent : TEXCOORD3;
  float4 overlayColor : TEXCOORD4;
};

Texture2D g_Textures[] : register(t0, space1);
#include "ConstantBuffer/sampler.hlsli"

#include "ConstantBuffer/shadow_cb.hlsli"

static const float PI = 3.14159265359;
static const float MIN_ROUGHNESS = 0.04;

float DistributionGGX(float NdotH, float roughness) {
  float a = roughness * roughness;
  float a2 = a * a;
  float denom = NdotH * NdotH * (a2 - 1.0) + 1.0;
  return a2 / (PI * denom * denom);
}

float GeometrySchlickGGX(float NdotV, float roughness) {
  float r = roughness + 1.0;
  float k = (r * r) / 8.0;
  return NdotV / (NdotV * (1.0 - k) + k);
}

float GeometrySmith(float NdotV, float NdotL, float roughness) {
  return GeometrySchlickGGX(NdotV, roughness) *
         GeometrySchlickGGX(NdotL, roughness);
}

float3 FresnelSchlick(float cosTheta, float3 F0) {
  return F0 + (1.0 - F0) * pow(saturate(1.0 - cosTheta), 5.0);
}

float4 main(PSIN input) : SV_TARGET {
  uint albedoIdx;
  uint normalIdx;
  uint mrIdx;
  uint emissiveIdx;
  uint matFlags;
  float specIntensity;
  float specPow;
  float rimInt;
  float rimPow;
  float3 rimCol;
  float metallicVal;
  float roughnessVal;
  uint samplerIdx;
  float3 emissiveVal;

  if (UseBindlessMaterial(g_ObjectCB.materialDescriptorIndex)) {
    MaterialDescriptor mat = LoadMaterial(g_ObjectCB.materialDescriptorIndex);
    albedoIdx     = mat.albedoTextureIndex;
    normalIdx     = mat.normalTextureIndex;
    mrIdx         = mat.metallicRoughnessIndex;
    emissiveIdx   = mat.emissiveTextureIndex;
    matFlags      = mat.flags;
    specIntensity = mat.specularIntensity;
    specPow       = mat.specularPower;
    rimInt        = mat.rimIntensity;
    rimPow        = mat.rimPower;
    rimCol        = mat.rimColor;
    metallicVal   = mat.metallicFactor;
    roughnessVal  = mat.roughnessFactor;
    samplerIdx    = mat.samplerIndex;
    emissiveVal   = mat.emissiveFactor;
  } else {
    albedoIdx     = g_MaterialData.albedoTextureIndex;
    normalIdx     = g_MaterialData.normalTextureIndex;
    mrIdx         = g_MaterialData.metallicRoughnessIndex;
    emissiveIdx   = g_MaterialData.emissiveTextureIndex;
    matFlags      = g_MaterialData.flags;
    specIntensity = g_MaterialData.specularIntensity;
    specPow       = g_MaterialData.specularPower;
    rimInt        = g_MaterialData.rimIntensity;
    rimPow        = g_MaterialData.rimPower;
    rimCol        = g_MaterialData.rimColor;
    metallicVal   = g_MaterialData.metallicFactor;
    roughnessVal  = g_MaterialData.roughnessFactor;
    samplerIdx    = g_ObjectCB.samplerIndex;
    emissiveVal   = g_MaterialData.emissiveFactor;
  }

  float4 albedoTex = g_Textures[albedoIdx].Sample(
      g_Samplers[samplerIdx], input.uv);
  float4 baseColor = albedoTex * input.color * g_ObjectCB.color;

  if ((g_ObjectCB.flags & OBJECT_FLAG_OPAQUE) && baseColor.a < 0.5) {
    discard;
  }

  if (!(g_ObjectCB.flags & OBJECT_FLAG_LIT)) {
    return baseColor;
  }

  float3 N = normalize(input.worldNormal);

  if (matFlags & MATERIAL_FLAG_HAS_NORMAL_MAP) {
    float3 T = normalize(input.worldTangent);
    float3 B = normalize(input.worldBitangent);
    float3x3 TBN = float3x3(T, B, N);
    float3 normalMap =
        g_Textures[normalIdx]
            .Sample(g_Samplers[samplerIdx], input.uv)
            .rgb;
    normalMap = normalMap * 2.0 - 1.0;
    N = normalize(mul(normalMap, TBN));
  }

  float metallic = metallicVal;
  float roughness = roughnessVal;

  if (matFlags & MATERIAL_FLAG_HAS_METALLIC_ROUGHNESS) {
    float4 mrSample = g_Textures[mrIdx].Sample(
        g_Samplers[samplerIdx], input.uv);
    metallic *= mrSample.b;
    roughness *= mrSample.g;
  }

  roughness = max(roughness, MIN_ROUGHNESS);

  float3 albedo = baseColor.rgb;
  float3 V = normalize(g_FrameCB.cameraPos - input.worldPos);
  float3 L = normalize(-g_LightingCB.lightDirection);
  float3 H = normalize(V + L);

  float NdotV = max(dot(N, V), 0.001);
  float NdotL = saturate(dot(N, L));
  float NdotH = saturate(dot(N, H));
  float HdotV = saturate(dot(H, V));

  float3 F0 = lerp(float3(0.04, 0.04, 0.04), albedo, metallic);

  float shadow = 1.0;
  if (g_ObjectCB.flags & OBJECT_FLAG_RECEIVE_SHADOW) {
    shadow = CalculateShadow(input.worldPos, N, input.position.xy);
  }
  float3 shadowTint = lerp(g_ShadowCB.shadowColor, float3(1, 1, 1), shadow);

  float D = DistributionGGX(NdotH, roughness);
  float G = GeometrySmith(NdotV, NdotL, roughness);
  float3 F = FresnelSchlick(HdotV, F0);

  float3 specular = (D * G * F) / (4.0 * NdotV * NdotL + 0.0001);

  float3 kD = (1.0 - F) * (1.0 - metallic);
  float3 diffuse = kD * albedo / PI;

  float3 directional = (diffuse + specular) * g_LightingCB.directionalColor *
                       g_LightingCB.lightIntensity * NdotL * shadowTint;

  float3 ambient =
      g_LightingCB.ambientColor * g_LightingCB.ambientIntensity * albedo;

  float ao = 1.0;
  if (g_LightingCB.ssaoSrvIndex != 0xFFFFFFFF) {
    float2 screenUV = input.position.xy / g_FrameCB.screenSize;
    ao =
        g_Textures[g_LightingCB.ssaoSrvIndex].Sample(g_Samplers[SAMPLER_LINEAR_CLAMP], screenUV).r;
  }

  float3 pointDiffuse = float3(0, 0, 0);
  float3 pointSpecular = float3(0, 0, 0);
  if (g_LightingCB.pointLightCount > 0) {
    CalcPointLightContribution(N, V, input.worldPos,
                               g_LightingCB.pointLightCount, 1.0,
                               max(2.0 / (roughness * roughness) - 2.0, 1.0),
                               pointDiffuse, pointSpecular);
    pointDiffuse *= albedo * (1.0 - metallic);
  }

  float rim = pow(1.0 - saturate(dot(N, V)), rimPow);
  float3 rimLight = rimCol * rimInt * rim;
  if (matFlags & MATERIAL_FLAG_RIM_SHADOW_AFFECTED) rimLight *= shadow;

  float3 emissive = emissiveVal;
  if (matFlags & MATERIAL_FLAG_HAS_EMISSIVE) {
    float3 emissiveTex =
        g_Textures[emissiveIdx]
            .Sample(g_Samplers[samplerIdx], input.uv)
            .rgb;
    emissive *= emissiveTex;
  }

  float3 finalColor = (directional + ambient + pointDiffuse) * ao +
                      pointSpecular + rimLight + emissive;
  finalColor += input.overlayColor.rgb * input.overlayColor.a;
  return float4(finalColor, baseColor.a);
}
