#pragma once

#include <algorithm>
#include <cstdint>

#include "Graphic/Frame/frame_packet.h"
#include "Graphic/Render/shadow_config.h"

class ShadowSetting {
 public:
  void SetEnabled(bool enabled) {
    enabled_ = enabled;
  }
  void SetResolution(uint32_t resolution) {
    resolution_ = resolution;
  }
  void SetShadowDistance(float distance) {
    shadow_distance_ = distance;
  }
  void SetLightDistance(float distance) {
    light_distance_ = distance;
  }
  void SetAlgorithm(ShadowAlgorithm algorithm) {
    algorithm_ = algorithm;
  }
  void SetShadowColor(const Math::Vector3& color) {
    shadow_color_ = color;
  }
  void SetCascadeBlendRange(float range) {
    cascade_blend_range_ = range;
  }

  void SetCascadeCount(uint32_t count) {
    cascade_count_ = std::clamp(count, 1u, ShadowCascadeConfig::MAX_CASCADES);
  }
  void SetCascadeDepthBias(uint32_t index, float bias) {
    if (index < ShadowCascadeConfig::MAX_CASCADES) cascade_depth_bias_[index] = bias;
  }
  void SetCascadeNormalBias(uint32_t index, float bias) {
    if (index < ShadowCascadeConfig::MAX_CASCADES) cascade_normal_bias_[index] = bias;
  }

  bool IsEnabled() const {
    return enabled_;
  }
  uint32_t GetResolution() const {
    return resolution_;
  }
  float GetShadowDistance() const {
    return shadow_distance_;
  }
  float GetLightDistance() const {
    return light_distance_;
  }
  ShadowAlgorithm GetAlgorithm() const {
    return algorithm_;
  }
  const Math::Vector3& GetShadowColor() const {
    return shadow_color_;
  }
  float GetCascadeBlendRange() const {
    return cascade_blend_range_;
  }

  uint32_t GetCascadeCount() const {
    return cascade_count_;
  }
  float GetCascadeDepthBias(uint32_t index) const {
    return index < ShadowCascadeConfig::MAX_CASCADES ? cascade_depth_bias_[index] : 0.0f;
  }
  float GetCascadeNormalBias(uint32_t index) const {
    return index < ShadowCascadeConfig::MAX_CASCADES ? cascade_normal_bias_[index] : 0.0f;
  }

  ShadowConfig ToConfig() const {
    ShadowConfig config{};
    config.resolution = resolution_;
    config.cascade_count = cascade_count_;
    for (uint32_t i = 0; i < ShadowCascadeConfig::MAX_CASCADES; ++i) {
      config.cascade_depth_bias[i] = cascade_depth_bias_[i];
      config.cascade_normal_bias[i] = cascade_normal_bias_[i];
    }
    config.shadow_distance = shadow_distance_;
    config.light_distance = light_distance_;
    config.cascade_blend_range = cascade_blend_range_;
    config.algorithm = algorithm_;
    config.shadow_color = shadow_color_;
    config.enabled = enabled_;
    return config;
  }

 private:
  bool enabled_ = true;
  uint32_t resolution_ = 2048;
  uint32_t cascade_count_ = ShadowCascadeConfig::DEFAULT_CASCADE_COUNT;
  float cascade_depth_bias_[ShadowCascadeConfig::MAX_CASCADES] = {ShadowReceiverDefaults::DEPTH_BIAS,
    ShadowReceiverDefaults::DEPTH_BIAS,
    ShadowReceiverDefaults::DEPTH_BIAS,
    ShadowReceiverDefaults::DEPTH_BIAS};
  float cascade_normal_bias_[ShadowCascadeConfig::MAX_CASCADES] = {ShadowReceiverDefaults::NORMAL_BIAS,
    ShadowReceiverDefaults::NORMAL_BIAS,
    ShadowReceiverDefaults::NORMAL_BIAS,
    ShadowReceiverDefaults::NORMAL_BIAS};
  float shadow_distance_ = 100.0f;
  float light_distance_ = 100.0f;
  float cascade_blend_range_ = 0.1f;
  ShadowAlgorithm algorithm_ = ShadowAlgorithm::PCF3x3;
  Math::Vector3 shadow_color_ = Math::Vector3(0.0f, 0.0f, 0.0f);
};
