/**
 * @file unit_circle_lut.h
 * @brief Sin/cos lookup table for unit circle tessellation, computed once at startup.
 */
#pragma once

#include <array>
#include <cmath>
#include <numbers>

namespace Math {

struct CirclePoint {
  float cos;
  float sin;
};

template <int Segments>
struct UnitCircleLUT {
  static_assert(Segments > 0 && Segments % 2 == 0);

  std::array<CirclePoint, Segments> points;

  static UnitCircleLUT Build() {
    UnitCircleLUT lut{};
    constexpr double TWO_PI = 2.0 * std::numbers::pi;
    for (int i = 0; i < Segments; ++i) {
      double angle = i * TWO_PI / Segments;
      lut.points[i] = {static_cast<float>(std::cos(angle)), static_cast<float>(std::sin(angle))};
    }
    return lut;
  }
};

inline const auto& GetCircleLUT8() {
  static const auto lut = UnitCircleLUT<8>::Build();
  return lut;
}

inline const auto& GetCircleLUT16() {
  static const auto lut = UnitCircleLUT<16>::Build();
  return lut;
}

inline const auto& GetCircleLUT32() {
  static const auto lut = UnitCircleLUT<32>::Build();
  return lut;
}

template <int Segments>
inline const auto& GetCircleLUT() {
  static const auto lut = UnitCircleLUT<Segments>::Build();
  return lut;
}

}  // namespace Math
