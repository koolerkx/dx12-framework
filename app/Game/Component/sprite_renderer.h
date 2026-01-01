#pragma once
#include "Component/component.h"
#include "Component/render_settings.h"
#include "Game/Asset/asset_manager.h"
#include "Graphic/Frame/frame_packet.h"
#include "Graphic/Resource/Texture/texture.h"
#include "game_context.h"
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

  void OnRender(FramePacket& packet) override {
    if (!texture_) return;

    auto* context = GetOwner()->GetContext();
    auto& material_mgr = context->GetGraphic()->GetMaterialManager();

    switch (pass_tag_) {
      case RenderPassTag::Ui: {
        // Push to the UI Pass queue
        UiDrawCommand cmd;
        cmd.mesh = context->GetAssetManager().GetDefaultMesh(DefaultMesh::Quad);
        cmd.material = material_mgr.GetOrCreateMaterial(render_settings_);
        cmd.color = color_;
        cmd.size = size_;
        cmd.layer_id = layer_id_;
        cmd.depth = static_cast<float>(layer_id_);

        // For UI rendering with orthographic projection:
        // The world matrix should transform from local space to screen space
        // Local quad vertices are [-0.5, 0.5] range
        // We apply the sprite's size as a base scale, then apply the transform's world matrix
        auto* transform = GetOwner()->GetTransform();

        // Build world matrix: Scale(size) * TransformWorld
        // This allows the sprite to have a base size in pixels, while still being
        // affected by the transform hierarchy (position, rotation, and additional scaling)
        DirectX::XMMATRIX size_scale = DirectX::XMMatrixScaling(size_.x, size_.y, 1.0f);
        DirectX::XMMATRIX world = size_scale * transform->GetWorldMatrix();

        DirectX::XMStoreFloat4x4(&cmd.world_matrix, world);

        // Setup material instance
        cmd.material_instance.material = cmd.material;
        cmd.material_instance.albedo_texture_index = texture_->GetBindlessIndex();
        cmd.material_instance.sampler_index = static_cast<uint32_t>(render_settings_.sampler_type);

        packet.ui_pass.push_back(cmd);
      } break;
      default:
        // TODO: Add support for other render passes, e.g. billboard
        // Handle other cases or log warning
        break;
    }
  }

 private:
  Texture* texture_ = nullptr;
  DirectX::XMFLOAT4 color_ = {1, 1, 1, 1};
  DirectX::XMFLOAT2 size_ = {100, 100};
  int layer_id_ = 0;

  RenderPassTag pass_tag_ = RenderPassTag::Ui;
  Rendering::RenderSettings render_settings_ = Rendering::RenderSettings::UI();
};
