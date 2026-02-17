#pragma once

#include "Component/component.h"
#include "Component/pivot_type.h"
#include "Framework/Math/Math.h"
#include "Graphic/Frame/frame_packet.h"
#include "game_object.h"

using Math::Vector2;
using Math::Vector4;

class UIGlassRenderer : public Component<UIGlassRenderer> {
 public:
  struct Props {
    Vector2 size = {200, 100};
    Vector4 tint_color = {1.0f, 1.0f, 1.0f, 0.1f};
    float distortion_strength = 0.02f;
    float tint_alpha = 0.15f;
    int layer_id = 0;
    Vector2 pivot = {0.0f, 0.0f};
  };

  UIGlassRenderer(GameObject* owner) : Component(owner) {
  }

  UIGlassRenderer(GameObject* owner, const Props& props) : Component(owner) {
    size_ = props.size;
    tint_color_ = props.tint_color;
    distortion_strength_ = props.distortion_strength;
    tint_alpha_ = props.tint_alpha;
    layer_id_ = props.layer_id;
    ui_pivot_ = props.pivot;
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

  void OnRender(FramePacket& packet) override;

 private:
  Vector2 size_ = {200, 100};
  Vector4 tint_color_ = {1.0f, 1.0f, 1.0f, 0.1f};
  float distortion_strength_ = 0.02f;
  float tint_alpha_ = 0.15f;
  int layer_id_ = 0;
  Vector2 ui_pivot_ = {0.0f, 0.0f};
};
