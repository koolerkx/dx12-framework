#pragma once
#include "Framework/Math/Math.h"

using Math::Matrix4;
using Math::Vector2;
using Math::Vector3;
using Math::Vector4;

struct alignas(256) FrameCB {
  Matrix4 view;
  Matrix4 proj;
  Matrix4 viewProj;
  Matrix4 invView;
  Matrix4 invProj;
  Vector3 cameraPos;
  float time;
  Vector2 screenSize;
  Vector2 _padding;
};
static_assert(sizeof(FrameCB) == 512);

struct alignas(256) ObjectCB {
  Matrix4 world;
  Matrix4 worldViewProj;
  Matrix4 normalMatrix;
  Vector4 color;
  Vector2 uvOffset;
  Vector2 uvScale;
  uint32_t samplerIndex;
  uint32_t flags;
  uint32_t _padding[2];
};
static_assert(sizeof(ObjectCB) == 256);

struct alignas(256) LightingCB {
  Vector3 lightDirection;
  float lightIntensity;
  Vector3 directionalColor;
  float ambientIntensity;
  Vector3 ambientColor;
  float _padding;
};
static_assert(sizeof(LightingCB) == 256);

struct alignas(256) ShadowCB {
  Matrix4 lightViewProj;
  uint32_t shadowMapIndex;
  float shadowBias;
  float shadowNormalBias;
  uint32_t shadowMapResolution;
};
static_assert(sizeof(ShadowCB) == 256);
