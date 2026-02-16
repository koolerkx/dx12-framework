#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <vector>

enum class ProceduralShape : uint8_t { Circle, Rect };

struct ProceduralTextureDesc {
  uint32_t size = 64;
  float falloff = 2.0f;
  ProceduralShape shape = ProceduralShape::Circle;
  float corner_radius = 0.0f;
};

inline std::vector<uint8_t> GenerateProceduralTexture(const ProceduralTextureDesc& desc) {
  const uint32_t s = desc.size;
  std::vector<uint8_t> pixels(s * s * 4, 255);

  for (uint32_t y = 0; y < s; ++y) {
    for (uint32_t x = 0; x < s; ++x) {
      float u = (static_cast<float>(x) + 0.5f) / static_cast<float>(s) * 2.0f - 1.0f;
      float v = (static_cast<float>(y) + 0.5f) / static_cast<float>(s) * 2.0f - 1.0f;

      float dist = 0.0f;

      if (desc.shape == ProceduralShape::Circle) {
        dist = std::sqrt(u * u + v * v);
      } else {
        float ax = std::abs(u);
        float ay = std::abs(v);
        float r = (std::clamp)(desc.corner_radius, 0.0f, 1.0f);
        if (r > 0.0f) {
          float qx = (std::max)(ax - (1.0f - r), 0.0f);
          float qy = (std::max)(ay - (1.0f - r), 0.0f);
          dist = std::sqrt(qx * qx + qy * qy) / r;
          float d1 = (ax - 1.0f) / r;
          float d2 = (ay - 1.0f) / r;
          dist = (std::max)(dist, (std::max)(d1, (std::max)(d2, 0.0f)));
        } else {
          dist = (std::max)(ax, ay);
        }
      }

      float clamped = (std::clamp)(dist, 0.0f, 1.0f);
      float base = (std::max)(1.0f - clamped, 0.0f);
      float alpha = std::pow(base, desc.falloff);
      uint8_t a = static_cast<uint8_t>((std::clamp)(alpha * 255.0f, 0.0f, 255.0f));

      size_t idx = (static_cast<size_t>(y) * s + x) * 4;
      pixels[idx + 0] = 255;
      pixels[idx + 1] = 255;
      pixels[idx + 2] = 255;
      pixels[idx + 3] = a;
    }
  }

  return pixels;
}
