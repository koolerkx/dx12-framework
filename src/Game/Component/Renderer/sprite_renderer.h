#pragma once
#include <optional>

#include "Component/billboard_type.h"
#include "Component/component.h"
#include "Component/pivot_type.h"
#include "Component/render_settings.h"
#include "Component/sprite_sheet_animator.h"
#include "Component/transform_component.h"
#include "Framework/Math/Math.h"
#include "Game/Asset/asset_manager.h"
#include "Graphic/Frame/frame_packet.h"
#include "Graphic/Resource/Texture/texture.h"
#include "game_object.h"

using Math::Matrix4;
using Math::Vector2;
using Math::Vector3;
using Math::Vector4;

class SpriteRenderer : public Component<SpriteRenderer> {
 public:
  SpriteRenderer(GameObject* owner) : Component(owner) {
  }

  void SetTexture(Texture* tex) {
    texture_ = tex;
  }
  void SetColor(const Vector4& color) {
    color_ = color;
  }
  void SetSize(const Vector2& size) {
    size_ = size;
  }

  void SetRenderLayer(RenderLayer layer);

  void SetRenderTags(RenderTagMask tags) {
    render_tags_ = tags;
  }
  void AddRenderTag(RenderTag tag) {
    render_tags_ |= static_cast<uint32_t>(tag);
  }

  void SetBlendMode(Rendering::BlendMode mode) {
    render_settings_.blend_mode = mode;
  }
  void SetSampler(Rendering::SamplerType type) {
    render_settings_.sampler_type = type;
  }
  void SetDepthTest(bool enabled) {
    render_settings_.depth_test = enabled;
  }
  void SetDepthWrite(bool enabled) {
    render_settings_.depth_write = enabled;
  }
  void SetDoubleSided(bool enabled) {
    render_settings_.double_sided = enabled;
  }
  const Rendering::RenderSettings& GetRenderSettings() const {
    return render_settings_;
  }

  void SetBillboardMode(Billboard::Mode mode) {
    billboard_mode_ = mode;
  }
  Billboard::Mode GetBillboardMode() const {
    return billboard_mode_;
  }

  void SetPivot(Pivot::Preset preset);
  void SetPivot(const Pivot::Config& config);
  const Pivot::Config& GetPivot() const {
    return pivot_;
  }

  void SetUVOffset(const Vector2& offset) {
    uv_offset_ = offset;
  }
  void SetUVScale(const Vector2& scale) {
    uv_scale_ = scale;
  }
  const Vector2& GetUVOffset() const {
    return uv_offset_;
  }
  const Vector2& GetUVScale() const {
    return uv_scale_;
  }

  SpriteSheetAnimator& GetAnimator();

  void OnUpdate(float dt) override;
  void OnRender(FramePacket& packet) override;

 private:
  Matrix4 CalculateWorldMatrix(TransformComponent* transform, const CameraData& camera) const;

 private:
  Texture* texture_ = nullptr;
  Vector4 color_ = {1, 1, 1, 1};
  Vector2 size_ = {100, 100};
  Vector2 uv_offset_ = {0.0f, 0.0f};
  Vector2 uv_scale_ = {1.0f, 1.0f};

  Rendering::RenderSettings render_settings_ = Rendering::RenderSettings::Transparent();
  Billboard::Mode billboard_mode_ = Billboard::Mode::None;
  Pivot::Config pivot_;

  RenderLayer render_layer_ = RenderLayer::Transparent;
  RenderTagMask render_tags_ = 0;

  std::optional<SpriteSheetAnimator> animator_;
};
