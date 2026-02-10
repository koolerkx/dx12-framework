#pragma once

#include "Framework/Math/Math.h"
#include "Graphic/Frame/frame_packet.h"

class LightSetting {
 public:
  void SetDirection(Math::Vector3 dir) { direction_ = dir; }
  void SetIntensity(float intensity) { intensity_ = intensity; }
  void SetDirectionalColor(Math::Vector3 color) { directional_color_ = color; }
  void SetAmbientIntensity(float ambient) { ambient_intensity_ = ambient; }
  void SetAmbientColor(Math::Vector3 color) { ambient_color_ = color; }

  Math::Vector3 GetDirection() const { return direction_; }
  float GetIntensity() const { return intensity_; }
  Math::Vector3 GetDirectionalColor() const { return directional_color_; }
  float GetAmbientIntensity() const { return ambient_intensity_; }
  Math::Vector3 GetAmbientColor() const { return ambient_color_; }

  LightingConfig ToConfig() const {
    return {direction_, intensity_, directional_color_, ambient_intensity_, ambient_color_};
  }

 private:
  Math::Vector3 direction_{0.0f, -1.0f, 0.0f};
  float intensity_ = 0.8f;
  Math::Vector3 directional_color_{1.0f, 1.0f, 1.0f};
  float ambient_intensity_ = 0.2f;
  Math::Vector3 ambient_color_{1.0f, 1.0f, 1.0f};
};
