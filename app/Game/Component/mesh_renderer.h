#pragma once
#include <DirectXMath.h>

#include "Component/component.h"
#include "Graphic/Frame/frame_packet.h"
#include "Graphic/Resource/mesh.h"
#include "Graphic/Resource/Texture/texture.h"
#include "game_object.h"
#include "game_context.h"
#include "transform_component.h"

class MeshRenderer : public Component<MeshRenderer> {
 public:
  using Component::Component;

  void SetMesh(const Mesh* mesh) {
    mesh_ = mesh;
  }

  void SetTexture(Texture* texture) {
    texture_ = texture;
  }

  void SetMaterial(const std::string& name) {
    material_name_ = name;
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

    auto* context = GetOwner()->GetContext();
    auto& material_mgr = context->GetGraphic()->GetMaterialManager();

    SingleDrawCommand cmd;
    DirectX::XMStoreFloat4x4(&cmd.world_matrix, transform->GetWorldMatrix());
    cmd.color = color_;
    cmd.mesh = mesh_;

    // Get material
    cmd.material = material_mgr.GetMaterial(material_name_);
    if (!cmd.material) {
      cmd.material = material_mgr.GetDefaultOpaque();
    }

    // Setup material instance
    cmd.material_instance.material = cmd.material;
    if (texture_) {
      cmd.material_instance.albedo_texture_index = texture_->GetBindlessIndex();
    }

    // Calculate depth from camera for sorting
    // TODO: Use actual camera position from FramePacket
    DirectX::XMFLOAT3 position = transform->GetPosition();
    cmd.depth = position.z;  // Simple depth calculation

    packet.opaque_pass.emplace_back(cmd);
  }

 private:
  const Mesh* mesh_ = nullptr;
  Texture* texture_ = nullptr;
  std::string material_name_ = "Default_Opaque";
  DirectX::XMFLOAT4 color_ = {1.0f, 1.0f, 1.0f, 1.0f};
};
