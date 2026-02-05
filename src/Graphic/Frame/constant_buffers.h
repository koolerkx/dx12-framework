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
  Vector3 cameraPos;
  float time;
  Vector2 screenSize;
  Vector2 _padding;
};

struct alignas(256) ObjectCB {
  Matrix4 world;
  Matrix4 worldViewProj;
  Vector4 color;
  Vector2 uvOffset;
  Vector2 uvScale;
  uint32_t samplerIndex;
  uint32_t _padding[3];
};

struct alignas(256) LightingCB {
  Vector3 lightDirection;
  float lightIntensity;
  Vector3 lightColor;
  float _padding;
};
