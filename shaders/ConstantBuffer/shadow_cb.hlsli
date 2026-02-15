#ifndef SHADOW_CB_HLSLI
#define SHADOW_CB_HLSLI

#include "ConstantBuffer/sampler.hlsli"

#ifndef SHADOW_CB_SLOT
#define SHADOW_CB_SLOT b4
#endif

SamplerComparisonState g_ShadowCmpSampler : register(s0, space2);

#define SHADOW_SAMPLER_INDEX SAMPLER_POINT_CLAMP

#define SHADOW_ALGO_HARD 0
#define SHADOW_ALGO_PCF3X3 1
#define SHADOW_ALGO_POISSON 2
#define SHADOW_ALGO_ROTATED_POISSON 3
#define SHADOW_ALGO_PCSS 4

#define MAX_CASCADES 4

struct ShadowCB {
  row_major float4x4 lightViewProj[MAX_CASCADES];
  uint4 shadowMapIndex;
  float4 cascadeDepthBias;
  float4 cascadeNormalBias;
  float4 cascadeSplitDistances;
  uint shadowAlgorithm;
  uint shadowMapResolution;
  uint cascadeCount;
  float cascadeBlendRange;
  float3 shadowColor;
  float lightSize;
};
ConstantBuffer<ShadowCB> g_ShadowCB : register(SHADOW_CB_SLOT);

float3 ProjectToShadowSpace(float3 worldPos, float3 worldNormal, uint cascade) {
  float3 biasedPos =
      worldPos + worldNormal * g_ShadowCB.cascadeNormalBias[cascade];
  float4 shadowPos =
      mul(float4(biasedPos, 1.0f), g_ShadowCB.lightViewProj[cascade]);
  float3 projCoords = shadowPos.xyz / shadowPos.w;
  projCoords.x = projCoords.x * 0.5 + 0.5;
  projCoords.y = projCoords.y * -0.5 + 0.5;
  return projCoords;
}

uint SelectCascade(float viewDepth) {
  uint cascade = 0;
  [unroll] for (uint i = 0; i < MAX_CASCADES; ++i) {
    if (i < g_ShadowCB.cascadeCount &&
        viewDepth > g_ShadowCB.cascadeSplitDistances[i]) {
      cascade = i + 1;
    }
  }
  return min(cascade, g_ShadowCB.cascadeCount - 1);
}

float SampleShadowHard(float3 projCoords, uint cascade) {
  if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 ||
      projCoords.y < 0.0 || projCoords.y > 1.0)
    return 1.0;

  return g_Textures[g_ShadowCB.shadowMapIndex[cascade]].SampleCmpLevelZero(
      g_ShadowCmpSampler, projCoords.xy,
      projCoords.z - g_ShadowCB.cascadeDepthBias[cascade]);
}

static const float2 POISSON_DISK[16] = {
    float2(-0.94201624, -0.39906216), float2(0.94558609, -0.76890725),
    float2(-0.09418410, -0.92938870), float2(0.34495938, 0.29387760),
    float2(-0.91588581, 0.45771432),  float2(-0.81544232, -0.87912464),
    float2(-0.38277543, 0.27676845),  float2(0.97484398, 0.75648379),
    float2(0.44323325, -0.97511554),  float2(0.53742981, -0.47373420),
    float2(-0.26496911, -0.41893023), float2(0.79197514, 0.19090188),
    float2(-0.24188840, 0.99706507),  float2(-0.81409955, 0.91437590),
    float2(0.19984126, 0.78641367),   float2(0.14383161, -0.14100790),
};

static const uint POISSON_SAMPLE_COUNT = 16;
static const float POISSON_SPREAD = 2.5;

float InterleavedGradientNoise(float2 screenPos) {
  float3 magic = float3(0.06711056, 0.00583715, 52.9829189);
  return frac(magic.z * frac(dot(screenPos, magic.xy)));
}

float SampleShadowPoisson(float3 projCoords, uint cascade) {
  if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 ||
      projCoords.y < 0.0 || projCoords.y > 1.0)
    return 1.0;

  float texelSize = 1.0 / float(g_ShadowCB.shadowMapResolution);
  float shadow = 0.0;

  float biasedDepth = projCoords.z - g_ShadowCB.cascadeDepthBias[cascade];

  [unroll] for (uint i = 0; i < POISSON_SAMPLE_COUNT; ++i) {
    float2 offset = POISSON_DISK[i] * texelSize * POISSON_SPREAD;
    shadow += g_Textures[g_ShadowCB.shadowMapIndex[cascade]].SampleCmpLevelZero(
        g_ShadowCmpSampler, projCoords.xy + offset, biasedDepth);
  }
  return shadow / float(POISSON_SAMPLE_COUNT);
}

float SampleShadowRotatedPoisson(float3 projCoords, uint cascade,
                                 float2 screenPos) {
  if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 ||
      projCoords.y < 0.0 || projCoords.y > 1.0)
    return 1.0;

  float texelSize = 1.0 / float(g_ShadowCB.shadowMapResolution);
  float angle = InterleavedGradientNoise(screenPos) * 6.28318530;
  float s, c;
  sincos(angle, s, c);
  float2x2 rotation = float2x2(c, -s, s, c);

  float shadow = 0.0;

  float biasedDepth = projCoords.z - g_ShadowCB.cascadeDepthBias[cascade];

  [unroll] for (uint i = 0; i < POISSON_SAMPLE_COUNT; ++i) {
    float2 offset = mul(rotation, POISSON_DISK[i]) * texelSize * POISSON_SPREAD;
    shadow += g_Textures[g_ShadowCB.shadowMapIndex[cascade]].SampleCmpLevelZero(
        g_ShadowCmpSampler, projCoords.xy + offset, biasedDepth);
  }
  return shadow / float(POISSON_SAMPLE_COUNT);
}

float SampleShadowPCF3x3(float3 projCoords, uint cascade) {
  if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 ||
      projCoords.y < 0.0 || projCoords.y > 1.0)
    return 1.0;

  float texelSize = 1.0 / float(g_ShadowCB.shadowMapResolution);
  float shadow = 0.0;

  float biasedDepth = projCoords.z - g_ShadowCB.cascadeDepthBias[cascade];

  [unroll] for (int x = -1; x <= 1; ++x) {
    [unroll] for (int y = -1; y <= 1; ++y) {
      float2 offset = float2(x, y) * texelSize;
      shadow +=
          g_Textures[g_ShadowCB.shadowMapIndex[cascade]].SampleCmpLevelZero(
              g_ShadowCmpSampler, projCoords.xy + offset, biasedDepth);
    }
  }
  return shadow / 9.0;
}

float SampleShadowPCSS(float3 projCoords, uint cascade, float2 screenPos) {
  if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 ||
      projCoords.y < 0.0 || projCoords.y > 1.0)
    return 1.0;

  float texelSize = 1.0 / float(g_ShadowCB.shadowMapResolution);
  float angle = InterleavedGradientNoise(screenPos) * 6.28318530;
  float s, c;
  sincos(angle, s, c);
  float2x2 rotation = float2x2(c, -s, s, c);

  float searchRadius = g_ShadowCB.lightSize * texelSize * POISSON_SPREAD;
  float blockerSum = 0.0;
  uint blockerCount = 0;
  float receiverDepth = projCoords.z - g_ShadowCB.cascadeDepthBias[cascade];

  [unroll] for (uint i = 0; i < POISSON_SAMPLE_COUNT; ++i) {
    float2 offset = mul(rotation, POISSON_DISK[i]) * searchRadius;
    float depth =
        g_Textures[g_ShadowCB.shadowMapIndex[cascade]]
            .Sample(g_Samplers[SHADOW_SAMPLER_INDEX], projCoords.xy + offset)
            .r;
    if (receiverDepth > depth) {
      blockerSum += depth;
      blockerCount++;
    }
  }

  if (blockerCount == 0)
    return 1.0;

  float avgBlockerDepth = blockerSum / float(blockerCount);
  float penumbraWidth = (receiverDepth - avgBlockerDepth) / avgBlockerDepth *
                        g_ShadowCB.lightSize;
  float filterRadius = penumbraWidth * texelSize * POISSON_SPREAD;
  filterRadius = max(filterRadius, texelSize);

  float shadow = 0.0;
  [unroll] for (uint j = 0; j < POISSON_SAMPLE_COUNT; ++j) {
    float2 offset = mul(rotation, POISSON_DISK[j]) * filterRadius;
    shadow += g_Textures[g_ShadowCB.shadowMapIndex[cascade]].SampleCmpLevelZero(
        g_ShadowCmpSampler, projCoords.xy + offset, receiverDepth);
  }
  return shadow / float(POISSON_SAMPLE_COUNT);
}

float SampleShadowForCascade(float3 worldPos, float3 worldNormal,
                             float2 screenPos, uint cascade) {
  float3 projCoords = ProjectToShadowSpace(worldPos, worldNormal, cascade);

  if (g_ShadowCB.shadowAlgorithm == SHADOW_ALGO_PCSS)
    return SampleShadowPCSS(projCoords, cascade, screenPos);
  if (g_ShadowCB.shadowAlgorithm == SHADOW_ALGO_ROTATED_POISSON)
    return SampleShadowRotatedPoisson(projCoords, cascade, screenPos);
  if (g_ShadowCB.shadowAlgorithm == SHADOW_ALGO_POISSON)
    return SampleShadowPoisson(projCoords, cascade);
  if (g_ShadowCB.shadowAlgorithm == SHADOW_ALGO_PCF3X3)
    return SampleShadowPCF3x3(projCoords, cascade);
  return SampleShadowHard(projCoords, cascade);
}

float CalculateShadow(float3 worldPos, float3 worldNormal, float2 screenPos) {
  float viewDepth = abs(mul(float4(worldPos, 1.0), g_FrameCB.view).z);
  uint cascade = SelectCascade(viewDepth);
  float shadow =
      SampleShadowForCascade(worldPos, worldNormal, screenPos, cascade);

  if (cascade < g_ShadowCB.cascadeCount - 1) {
    float splitDist = g_ShadowCB.cascadeSplitDistances[cascade];
    float blendRange = splitDist * g_ShadowCB.cascadeBlendRange;
    if (blendRange > 0.0 && viewDepth > splitDist - blendRange) {
      float nextShadow =
          SampleShadowForCascade(worldPos, worldNormal, screenPos, cascade + 1);
      float t = (viewDepth - (splitDist - blendRange)) / blendRange;
      shadow = lerp(shadow, nextShadow, saturate(t));
    }
  }

  return shadow;
}

#endif
