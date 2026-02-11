#pragma once

#include <cstdint>

#include "Graphic/Frame/frame_packet.h"

class ShadowSetting {
 public:
  void SetEnabled(bool enabled) { enabled_ = enabled; }
  void SetResolution(uint32_t resolution) { resolution_ = resolution; }
  void SetDepthBias(float bias) { depth_bias_ = bias; }
  void SetNormalBias(float bias) { normal_bias_ = bias; }
  void SetShadowDistance(float distance) { shadow_distance_ = distance; }
  void SetAlgorithm(ShadowAlgorithm algorithm) { algorithm_ = algorithm; }
  void SetShadowColor(const Math::Vector3& color) { shadow_color_ = color; }

  bool IsEnabled() const { return enabled_; }
  uint32_t GetResolution() const { return resolution_; }
  float GetDepthBias() const { return depth_bias_; }
  float GetNormalBias() const { return normal_bias_; }
  float GetShadowDistance() const { return shadow_distance_; }
  ShadowAlgorithm GetAlgorithm() const { return algorithm_; }
  const Math::Vector3& GetShadowColor() const { return shadow_color_; }

  ShadowConfig ToConfig() const {
    return {
        .resolution = resolution_,
        .depth_bias = depth_bias_,
        .normal_bias = normal_bias_,
        .shadow_distance = shadow_distance_,
        .algorithm = algorithm_,
        .shadow_color = shadow_color_,
        .enabled = enabled_,
    };
  }

 private:
  bool enabled_ = true;
  uint32_t resolution_ = 2048;
  float depth_bias_ = 0.005f;
  float normal_bias_ = 0.02f;
  float shadow_distance_ = 100.0f;
  ShadowAlgorithm algorithm_ = ShadowAlgorithm::PCF3x3;
  Math::Vector3 shadow_color_ = Math::Vector3(0.0f, 0.0f, 0.0f);
};
