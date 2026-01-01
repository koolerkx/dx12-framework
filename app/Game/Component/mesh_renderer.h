#pragma once
#include <DirectXMath.h>

#include "Component/component.h"
#include "Component/render_settings.h"
#include "Graphic/Frame/frame_packet.h"
#include "Graphic/Pipeline/shader_types.h"
#include "Graphic/Resource/Texture/texture.h"
#include "Graphic/Resource/mesh.h"
#include "game_context.h"
#include "game_object.h"
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

  void SetColor(const DirectX::XMFLOAT4& color) {
    color_ = color;
  }

  void SetRenderSettings(const Rendering::RenderSettings& settings) {
    render_settings_ = settings;
  }

  void SetShaderID(Graphics::ShaderID shader_id) {
    shader_id_ = shader_id;
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

    cmd.material = material_mgr.GetOrCreateMaterial(shader_id_, render_settings_);
    cmd.material_instance.material = cmd.material;
    cmd.material_instance.albedo_texture_index = texture_ ? texture_->GetBindlessIndex() : 0;
    cmd.material_instance.sampler_index = static_cast<uint32_t>(render_settings_.sampler_type);

    // Calculate depth from camera for sorting
    // TODO: Use actual camera position from FramePacket
    DirectX::XMFLOAT3 position = transform->GetPosition();
    cmd.depth = position.z;  // Simple depth calculation

    packet.opaque_pass.emplace_back(cmd);
  }

 private:
  const Mesh* mesh_ = nullptr;
  Texture* texture_ = nullptr;
  Graphics::ShaderID shader_id_ = Graphics::ShaderID::Basic3D;
  Rendering::RenderSettings render_settings_ = Rendering::RenderSettings::Opaque();
  DirectX::XMFLOAT4 color_ = {1.0f, 1.0f, 1.0f, 1.0f};
};
