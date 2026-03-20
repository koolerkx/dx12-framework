#pragma once

#include "Component/renderer_component.h"
#include "Component/pivot_type.h"
#include "Framework/Math/Math.h"
#include "Framework/Render/frame_packet.h"
#include "game_object.h"

using Math::Vector2;
using Math::Vector4;

class UIGlassRenderer : public RendererComponent<UIGlassRenderer> {
 public:
  // Default values here are the single source of truth for glass appearance.
  struct Props {
    Vector2 size = {200, 100};
    Vector4 tint_color = {1.0f, 1.0f, 1.0f, 0.1f};
    // float distortion_strength = 0.02f;  // UV offset magnitude for refraction (higher = more warping)
    float distortion_strength = 0.1f;  // UV offset magnitude for refraction (higher = more warping)
    float tint_alpha = 0.15f;          // blend factor between blurred background and tint color
    std::optional<int> layer_id = std::nullopt;
    Vector2 pivot = {0.0f, 0.0f};
    float chromatic_strength = 0.1f;         // R/G/B channel separation along distortion direction
    float fresnel_power = 1.5f;              // exponent controlling rim light falloff (higher = thinner rim)
    float fresnel_intensity = 0.3f;          // brightness of the rim/edge glow
    float specular_intensity = 0.15f;        // peak brightness of the fake specular highlight blob
    float specular_sharpness = 8.0f;         // gaussian falloff rate (higher = smaller, sharper highlight)
    Vector2 specular_offset = {0.3f, 0.3f};  // highlight center offset from panel center in UV space
    float edge_shadow_strength = 0.5f;       // darkening at panel edges via smoothstep vignette
    float corner_radius_px = 24.0f;
    float darken = 0.2f;  // 0.0 = no darkening, 1.0 = fully black
  };

  UIGlassRenderer(GameObject* owner) : RendererComponent(owner) {
  }

  UIGlassRenderer(GameObject* owner, const Props& props) : RendererComponent(owner) {
    size_ = props.size;
    tint_color_ = props.tint_color;
    distortion_strength_ = props.distortion_strength;
    tint_alpha_ = props.tint_alpha;
    layer_id_ = props.layer_id.value_or(InheritParentUILayerId());
    ui_pivot_ = props.pivot;
    chromatic_strength_ = props.chromatic_strength;
    fresnel_power_ = props.fresnel_power;
    fresnel_intensity_ = props.fresnel_intensity;
    specular_intensity_ = props.specular_intensity;
    specular_sharpness_ = props.specular_sharpness;
    specular_offset_ = props.specular_offset;
    edge_shadow_strength_ = props.edge_shadow_strength;
    corner_radius_px_ = props.corner_radius_px;
    darken_ = props.darken;
  }

  void SetSize(const Vector2& size) {
    size_ = size;
  }
  void SetTintColor(const Vector4& color) {
    tint_color_ = color;
  }
  void SetDistortionStrength(float strength) {
    distortion_strength_ = strength;
  }
  void SetTintAlpha(float alpha) {
    tint_alpha_ = alpha;
  }
  void SetLayerId(int id) {
    layer_id_ = id;
  }
  void SetPivot(const Vector2& pivot) {
    ui_pivot_ = pivot;
  }

  std::optional<int> GetUILayerId() const override {
    return layer_id_;
  }

  void OnRender(FramePacket& packet) override;

 private:
  Vector2 size_ = {200, 100};
  Vector4 tint_color_ = {1.0f, 1.0f, 1.0f, 0.1f};
  float distortion_strength_ = 0.02f;
  float tint_alpha_ = 0.15f;
  int layer_id_ = 0;
  Vector2 ui_pivot_ = {0.0f, 0.0f};
  float chromatic_strength_ = 0.005f;
  float fresnel_power_ = 3.0f;
  float fresnel_intensity_ = 0.3f;
  float specular_intensity_ = 0.15f;
  float specular_sharpness_ = 8.0f;
  Vector2 specular_offset_ = {0.3f, 0.3f};
  float edge_shadow_strength_ = 1.0f;
  float corner_radius_px_ = 16.0f;
  float darken_ = 0.0f;
};
