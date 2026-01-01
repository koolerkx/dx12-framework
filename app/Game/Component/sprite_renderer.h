#pragma once
#include "Component/billboard_type.h"
#include "Component/component.h"
#include "Component/pivot_type.h"
#include "Component/render_settings.h"
#include "Game/Asset/asset_manager.h"
#include "Graphic/Frame/frame_packet.h"
#include "Graphic/Resource/Texture/texture.h"
#include "game_object.h"
#include "transform_component.h"

class SpriteRenderer : public Component<SpriteRenderer> {
 public:
  SpriteRenderer(GameObject* owner) : Component(owner) {
  }

  void SetRenderPassTag(RenderPassTag tag) {
    pass_tag_ = tag;
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

  // Render settings API
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

  // Billboard mode API
  void SetBillboardMode(Billboard::Mode mode) {
    billboard_mode_ = mode;
  }
  Billboard::Mode GetBillboardMode() const {
    return billboard_mode_;
  }

  // Pivot API
  void SetPivot(Pivot::Preset preset);
  void SetPivot(const Pivot::Config& config);
  const Pivot::Config& GetPivot() const {
    return pivot_;
  }

  void OnRender(FramePacket& packet) override;

 private:
  DirectX::XMMATRIX CalculateWorldMatrix(TransformComponent* transform, const CameraData& camera) const;

 private:
  Texture* texture_ = nullptr;
  DirectX::XMFLOAT4 color_ = {1, 1, 1, 1};
  DirectX::XMFLOAT2 size_ = {100, 100};
  int layer_id_ = 0;

  RenderPassTag pass_tag_ = RenderPassTag::Ui;
  Rendering::RenderSettings render_settings_ = Rendering::RenderSettings::UI();
  Billboard::Mode billboard_mode_ = Billboard::Mode::None;
  Pivot::Config pivot_;
};
