#pragma once
#include "Component/component.h"
#include "Graphic/frame_packet.h"
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

  void OnRender(FramePacket& packet) override {
    if (!texture_) return;

    UiDrawCommand cmd;

    DirectX::XMStoreFloat4x4(&cmd.world_matrix, GetOwner()->GetTransform()->GetWorldMatrix());

    cmd.texture = texture_;
    cmd.color = color_;
    cmd.size = size_;
    cmd.layer_id = layer_id_;

    switch (pass_tag_) {
      case RenderPassTag::Ui:
        // Push to the UI Pass queue
        packet.ui_pass.push_back(cmd);
        break;
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
};
