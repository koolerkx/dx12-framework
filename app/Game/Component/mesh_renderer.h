#pragma once
#include <DirectXMath.h>

#include "Component/component.h"
#include "Graphic/Frame/frame_packet.h"
#include "Graphic/Resource/mesh.h"
#include "game_object.h"
#include "transform_component.h"

struct Texture;

class MeshRenderer : public Component<MeshRenderer> {
 public:
  using Component::Component;

  void SetMesh(const Mesh* mesh) {
    mesh_ = mesh;
  }

  void SetTexture(Texture* texture) {
    texture_ = texture;
  }

  void SetColor(const DirectX::XMFLOAT4& color) {
    color_ = color;
  }

  const Mesh* GetMesh() const {
    return mesh_;
  }

  void OnRender(FramePacket& packet) override {
    if (!mesh_) return;

    auto* transform = GetOwner()->GetTransform();
    if (!transform) return;

    OpaqueDrawCommand cmd;
    DirectX::XMStoreFloat4x4(&cmd.world_matrix, transform->GetWorldMatrix());
    cmd.color = color_;
    cmd.texture = texture_;
    cmd.mesh = mesh_;

    // Calculate depth from camera for sorting
    // TODO: Use actual camera position from FramePacket
    DirectX::XMFLOAT3 position = transform->GetPosition();
    cmd.depth = position.z;  // Simple depth calculation

    packet.opaque_pass.push_back(cmd);
  }

 private:
  const Mesh* mesh_ = nullptr;
  Texture* texture_ = nullptr;
  DirectX::XMFLOAT4 color_ = {1.0f, 1.0f, 1.0f, 1.0f};
};
