#pragma once
#include "Component/component.h"
#include "Component/render_settings.h"
#include "Component/transform_component.h"
#include "Framework/Math/Math.h"
#include "Graphic/Frame/frame_packet.h"
#include "Graphic/Pipeline/shader_descriptors.h"
#include "Graphic/Resource/Texture/texture.h"
#include "Graphic/Resource/mesh.h"
#include "game_context.h"
#include "game_object.h"

using Math::Vector3;
using Math::Vector4;

class MeshRenderer : public Component<MeshRenderer> {
 public:
  using Component::Component;

  void SetMesh(const Mesh* mesh) {
    mesh_ = mesh;
  }

  void SetTexture(Texture* texture) {
    texture_ = texture;
  }

  void SetColor(const Vector4& color) {
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

  Texture* GetTexture() const {
    return texture_;
  }

  Graphics::ShaderId GetShaderId() const {
    return shader_id_;
  }

  struct EditorData {
    Vector4 color;
    Rendering::RenderSettings render_settings;
    RenderLayer render_layer;
    RenderTagMask render_tags;
  };

  EditorData GetEditorData() const {
    return {color_, render_settings_, render_layer_, render_tags_};
  }

  void ApplyEditorData(const EditorData& data) {
    color_ = data.color;
    render_settings_ = data.render_settings;
    render_layer_ = data.render_layer;
    render_tags_ = data.render_tags;
  }

  void OnRender(FramePacket& packet) override {
    if (!mesh_) return;

    auto* transform = GetOwner()->GetTransform();
    if (!transform) return;

    auto* context = GetOwner()->GetContext();
    auto& material_mgr = context->GetGraphic()->GetMaterialManager();

    DrawCommand cmd;
    cmd.world_matrix = transform->GetWorldMatrix();
    cmd.color = color_;
    cmd.mesh = mesh_;

    cmd.material = material_mgr.GetOrCreateMaterial(shader_id_, render_settings_);
    cmd.material_instance.material = cmd.material;
    cmd.material_instance.albedo_texture_index = texture_ ? texture_->GetBindlessIndex() : 0;
    cmd.material_instance.sampler_index = static_cast<uint32_t>(render_settings_.sampler_type);

    Vector3 worldPos = transform->GetWorldPosition();
    Vector3 camPos = packet.main_camera.position;
    cmd.depth = Vector3::DistanceSquared(worldPos, camPos);

    cmd.layer = render_layer_;
    cmd.tags = render_tags_;
    packet.AddCommand(std::move(cmd));
  }

 private:
  const Mesh* mesh_ = nullptr;
  Texture* texture_ = nullptr;
  Graphics::ShaderId shader_id_ = Graphics::Basic3DShader::ID;
  Rendering::RenderSettings render_settings_ = Rendering::RenderSettings::Opaque();
  Vector4 color_ = {1.0f, 1.0f, 1.0f, 1.0f};

  RenderLayer render_layer_ = RenderLayer::Opaque;
  RenderTagMask render_tags_ = 0;
};
