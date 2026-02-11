#ifndef SHADOW_CB_HLSLI
#define SHADOW_CB_HLSLI

#ifndef SHADOW_CB_SLOT
#define SHADOW_CB_SLOT b4
#endif

#define SHADOW_SAMPLER_INDEX 3

#define SHADOW_ALGO_HARD 0
#define SHADOW_ALGO_PCF3X3 1
#define SHADOW_ALGO_POISSON 2
#define SHADOW_ALGO_ROTATED_POISSON 3

struct ShadowCB {
  row_major float4x4 lightViewProj;
  uint shadowMapIndex;
  float shadowBias;
  float shadowNormalBias;
  uint shadowMapResolution;
  uint shadowAlgorithm;
  float3 shadowColor;
};
ConstantBuffer<ShadowCB> g_ShadowCB : register(SHADOW_CB_SLOT);

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

float CalculateShadowPoisson(float3 worldPos, float3 worldNormal) {
  float3 projCoords = ProjectToShadowSpace(worldPos, worldNormal);

  if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 ||
      projCoords.y < 0.0 || projCoords.y > 1.0)
    return 1.0;

  float texelSize = 1.0 / float(g_ShadowCB.shadowMapResolution);
  float shadow = 0.0;

  [unroll] for (uint i = 0; i < POISSON_SAMPLE_COUNT; ++i) {
    float2 offset = POISSON_DISK[i] * texelSize * POISSON_SPREAD;
    float depth =
        g_Textures[g_ShadowCB.shadowMapIndex]
            .Sample(g_Samplers[SHADOW_SAMPLER_INDEX], projCoords.xy + offset)
            .r;
    shadow += (projCoords.z - g_ShadowCB.shadowBias > depth) ? 0.0 : 1.0;
  }
  return shadow / float(POISSON_SAMPLE_COUNT);
}

float CalculateShadowRotatedPoisson(float3 worldPos, float3 worldNormal,
                                    float2 screenPos) {
  float3 projCoords = ProjectToShadowSpace(worldPos, worldNormal);

  if (projCoords.z > 1.0 || projCoords.x < 0.0 || projCoords.x > 1.0 ||
      projCoords.y < 0.0 || projCoords.y > 1.0)
    return 1.0;

  float texelSize = 1.0 / float(g_ShadowCB.shadowMapResolution);
  float angle = InterleavedGradientNoise(screenPos) * 6.28318530;
  float s, c;
  sincos(angle, s, c);
  float2x2 rotation = float2x2(c, -s, s, c);

  float shadow = 0.0;

  [unroll] for (uint i = 0; i < POISSON_SAMPLE_COUNT; ++i) {
    float2 offset = mul(rotation, POISSON_DISK[i]) * texelSize * POISSON_SPREAD;
    float depth =
        g_Textures[g_ShadowCB.shadowMapIndex]
            .Sample(g_Samplers[SHADOW_SAMPLER_INDEX], projCoords.xy + offset)
            .r;
    shadow += (projCoords.z - g_ShadowCB.shadowBias > depth) ? 0.0 : 1.0;
  }
  return shadow / float(POISSON_SAMPLE_COUNT);
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

float CalculateShadow(float3 worldPos, float3 worldNormal, float2 screenPos) {
  if (g_ShadowCB.shadowAlgorithm == SHADOW_ALGO_ROTATED_POISSON)
    return CalculateShadowRotatedPoisson(worldPos, worldNormal, screenPos);
  if (g_ShadowCB.shadowAlgorithm == SHADOW_ALGO_POISSON)
    return CalculateShadowPoisson(worldPos, worldNormal);
  if (g_ShadowCB.shadowAlgorithm == SHADOW_ALGO_PCF3X3)
    return CalculateShadowPCF3x3(worldPos, worldNormal);
  return CalculateShadowHard(worldPos, worldNormal);
}

#endif
