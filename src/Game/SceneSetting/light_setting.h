#pragma once

#include <cmath>

#include "Framework/Math/Math.h"
#include "Graphic/Frame/frame_packet.h"

class LightSetting {
 public:
  void SetAzimuth(float degrees) { azimuth_ = degrees; }
  void SetElevation(float degrees) { elevation_ = degrees; }
  void SetIntensity(float intensity) { intensity_ = intensity; }
  void SetDirectionalColor(Math::Vector3 color) { directional_color_ = color; }
  void SetAmbientIntensity(float ambient) { ambient_intensity_ = ambient; }
  void SetAmbientColor(Math::Vector3 color) { ambient_color_ = color; }

  float GetAzimuth() const { return azimuth_; }
  float GetElevation() const { return elevation_; }
  float GetIntensity() const { return intensity_; }
  Math::Vector3 GetDirectionalColor() const { return directional_color_; }
  float GetAmbientIntensity() const { return ambient_intensity_; }
  Math::Vector3 GetAmbientColor() const { return ambient_color_; }

  LightingConfig ToConfig() const {
    constexpr float DEG_TO_RAD = 3.14159265f / 180.0f;
    float azimuth_rad = azimuth_ * DEG_TO_RAD;
    float elevation_rad = elevation_ * DEG_TO_RAD;
    float cos_elev = std::cos(elevation_rad);
    Math::Vector3 direction{
        std::sin(azimuth_rad) * cos_elev,
        -std::sin(elevation_rad),
        std::cos(azimuth_rad) * cos_elev};
    direction.Normalize();
    return {direction, intensity_, directional_color_, ambient_intensity_,
            ambient_color_};
  }

 private:
  float azimuth_ = 45.0f;
  float elevation_ = 45.0f;
  float intensity_ = 0.8f;
  Math::Vector3 directional_color_{1.0f, 1.0f, 1.0f};
  float ambient_intensity_ = 0.2f;
  Math::Vector3 ambient_color_{1.0f, 1.0f, 1.0f};
};
