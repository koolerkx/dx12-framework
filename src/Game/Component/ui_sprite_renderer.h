#pragma once
#include <optional>

#include "Component/component.h"
#include "Component/render_settings.h"
#include "Component/sprite_sheet_animator.h"
#include "Graphic/Frame/frame_packet.h"
#include "Graphic/Resource/Texture/texture.h"
#include "game_object.h"

class UISpriteRenderer : public Component<UISpriteRenderer> {
 public:
  UISpriteRenderer(GameObject* owner) : Component(owner) {
  }

  void SetTexture(Texture* tex) {
    texture_ = tex;
  }
  void SetColor(const DirectX::XMFLOAT4& color) {
    color_ = color;
  }
  void SetSize(const DirectX::XMFLOAT2& size) {
    size_ = size;
  }
  void SetLayerId(int id) {
    layer_id_ = id;
  }

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
  const Rendering::RenderSettings& GetRenderSettings() const {
    return render_settings_;
  }

  void SetUVOffset(const DirectX::XMFLOAT2& offset) {
    uv_offset_ = offset;
  }
  void SetUVScale(const DirectX::XMFLOAT2& scale) {
    uv_scale_ = scale;
  }
  const DirectX::XMFLOAT2& GetUVOffset() const {
    return uv_offset_;
  }
  const DirectX::XMFLOAT2& GetUVScale() const {
    return uv_scale_;
  }

  SpriteSheetAnimator& GetAnimator();

  void OnUpdate(float dt) override;
  void OnRender(FramePacket& packet) override;

 private:
  Texture* texture_ = nullptr;
  DirectX::XMFLOAT4 color_ = {1, 1, 1, 1};
  DirectX::XMFLOAT2 size_ = {100, 100};
  DirectX::XMFLOAT2 uv_offset_ = {0.0f, 0.0f};
  DirectX::XMFLOAT2 uv_scale_ = {1.0f, 1.0f};
  int layer_id_ = 0;

  Rendering::RenderSettings render_settings_ = Rendering::RenderSettings::UI();
  RenderTagMask render_tags_ = 0;

  std::optional<SpriteSheetAnimator> animator_;
};
