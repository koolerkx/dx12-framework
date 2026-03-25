#pragma once
#include <cstdint>

#include "Framework/Math/Math.h"
#include "Framework/Render/material_descriptor.h"
#include "Framework/Render/shadow_config.h"

using Math::Matrix4;
using Math::Vector2;
using Math::Vector3;
using Math::Vector4;

enum class ObjectFlags : uint32_t {
  None = 0,
  Lit = 1u << 0,
  Opaque = 1u << 1,
  ReceiveShadow = 1u << 2,
};

struct alignas(256) FrameCB {
  Matrix4 view;
  Matrix4 proj;
  Matrix4 viewProj;
  Matrix4 invView;
  Matrix4 invProj;
  Vector3 cameraPos;
  float time;
  Vector2 screenSize;
  Vector2 _pad;
};
static_assert(sizeof(FrameCB) == 512);

struct alignas(256) LightingCB {
  Vector3 lightDirection;
  float lightIntensity;
  Vector3 directionalColor;
  float ambientIntensity;
  Vector3 ambientColor;
  uint32_t pointLightCount;
  uint32_t ssaoSrvIndex;
  uint32_t _pad[3];
};
static_assert(sizeof(LightingCB) == 256);

struct PointLightData {
  Vector3 position;
  float intensity;
  Vector3 color;
  float radius;
  float falloff;
  uint32_t enabled;
  float _pad[2];
};
static_assert(sizeof(PointLightData) == 48);
static_assert(sizeof(PointLightData) % 16 == 0);

struct alignas(256) ShadowCB {
  Matrix4 lightViewProj[ShadowCascadeConfig::MAX_CASCADES];
  uint32_t shadowMapIndex[ShadowCascadeConfig::MAX_CASCADES];
  float cascadeDepthBias[ShadowCascadeConfig::MAX_CASCADES];
  float cascadeNormalBias[ShadowCascadeConfig::MAX_CASCADES];
  float cascadeSplitDistances[ShadowCascadeConfig::MAX_CASCADES];
  uint32_t shadowAlgorithm;
  uint32_t shadowMapResolution;
  uint32_t cascadeCount;
  float cascadeBlendRange;
  Vector3 shadowColor;
  float lightSize;
};
static_assert(sizeof(ShadowCB) == 512);

struct alignas(256) CustomCB {
  float data[20];
};
static_assert(sizeof(CustomCB) == 256);
