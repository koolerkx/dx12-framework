#pragma once
#include <DirectXMath.h>

#include "Component/component.h"
#include "Component/render_settings.h"
#include "Graphic/Frame/frame_packet.h"
#include "Graphic/Pipeline/shader_descriptors.h"
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

  void SetShaderId(Graphics::ShaderId shader_id) {
    shader_id_ = shader_id;
  }

  template <typename ShaderType>
  void SetShader() {
    shader_id_ = ShaderType::ID;
  }

  // Layer/Tag API
  void SetRenderLayer(RenderLayer layer) {
    render_layer_ = layer;
  }
  void SetRenderTags(RenderTagMask tags) {
    render_tags_ = tags;
  }
  void AddRenderTag(RenderTag tag) {
    render_tags_ |= static_cast<uint32_t>(tag);
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

    DrawCommand cmd;
    DirectX::XMStoreFloat4x4(&cmd.world_matrix, transform->GetWorldMatrix());
    cmd.color = color_;
    cmd.mesh = mesh_;

    cmd.material = material_mgr.GetOrCreateMaterial(shader_id_, render_settings_);
    cmd.material_instance.material = cmd.material;
    cmd.material_instance.albedo_texture_index = texture_ ? texture_->GetBindlessIndex() : 0;
    cmd.material_instance.sampler_index = static_cast<uint32_t>(render_settings_.sampler_type);

    DirectX::XMFLOAT3 worldPos = transform->GetWorldPosition();
    DirectX::XMFLOAT3 camPos = packet.main_camera.position;
    DirectX::XMVECTOR worldVec = XMLoadFloat3(&worldPos);
    DirectX::XMVECTOR camVec = XMLoadFloat3(&camPos);
    cmd.depth = DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(DirectX::XMVectorSubtract(worldVec, camVec)));

    cmd.layer = render_layer_;
    cmd.tags = render_tags_;
    packet.AddCommand(std::move(cmd));

    // DEPRECATED: Also push to old system for backward compatibility
    SingleDrawCommand old_cmd;
    old_cmd.mesh = cmd.mesh;
    old_cmd.material = cmd.material;
    old_cmd.material_instance = cmd.material_instance;
    old_cmd.world_matrix = cmd.world_matrix;
    old_cmd.color = cmd.color;
    old_cmd.depth = cmd.depth;
    packet.opaque_pass.emplace_back(old_cmd);
  }

 private:
  const Mesh* mesh_ = nullptr;
  Texture* texture_ = nullptr;
  Graphics::ShaderId shader_id_ = Graphics::Basic3DShader::ID;
  Rendering::RenderSettings render_settings_ = Rendering::RenderSettings::Opaque();
  DirectX::XMFLOAT4 color_ = {1.0f, 1.0f, 1.0f, 1.0f};

  // New layer/tag system
  RenderLayer render_layer_ = RenderLayer::Opaque;
  RenderTagMask render_tags_ = 0;
};
