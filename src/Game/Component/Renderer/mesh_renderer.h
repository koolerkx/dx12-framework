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
  struct Props {
    const Mesh* mesh = nullptr;
    Texture* texture = nullptr;
    Vector4 color = {1, 1, 1, 1};
    Graphics::ShaderId shader_id = Graphics::Basic3DShader::ID;
    RenderLayer render_layer = RenderLayer::Opaque;
    float specular_intensity = 0.5f;
    float specular_power = 32.0f;
    float rim_intensity = 0.0f;
    float rim_power = 4.0f;
    Vector3 rim_color = {1, 1, 1};
    bool rim_shadow_affected = false;
  };

  using Component::Component;

  MeshRenderer(GameObject* owner, const Props& props) : Component(owner) {
    if (props.mesh) SetMesh(props.mesh);
    if (props.texture) SetTexture(props.texture);
    SetColor(props.color);
    SetShaderId(props.shader_id);
    SetRenderLayer(props.render_layer);
    SetSpecularIntensity(props.specular_intensity);
    SetSpecularPower(props.specular_power);
    SetRimIntensity(props.rim_intensity);
    SetRimPower(props.rim_power);
    SetRimColor(props.rim_color);
    SetRimShadowAffected(props.rim_shadow_affected);
  }

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
  void SetSpecularIntensity(float intensity) {
    specular_intensity_ = intensity;
  }
  void SetSpecularPower(float power) {
    specular_power_ = power;
  }
  void SetRimIntensity(float intensity) {
    rim_intensity_ = intensity;
  }
  void SetRimPower(float power) {
    rim_power_ = power;
  }
  void SetRimColor(const Vector3& color) {
    rim_color_ = color;
  }
  void SetRimShadowAffected(bool affected) {
    rim_shadow_affected_ = affected;
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
    float specular_intensity;
    float specular_power;
    float rim_intensity;
    float rim_power;
    Vector3 rim_color;
    bool rim_shadow_affected;
  };

  EditorData GetEditorData() const {
    return {color_, render_settings_, render_layer_, render_tags_, specular_intensity_, specular_power_,
      rim_intensity_, rim_power_, rim_color_, rim_shadow_affected_};
  }

  void ApplyEditorData(const EditorData& data) {
    color_ = data.color;
    render_settings_ = data.render_settings;
    render_layer_ = data.render_layer;
    render_tags_ = data.render_tags;
    specular_intensity_ = data.specular_intensity;
    specular_power_ = data.specular_power;
    rim_intensity_ = data.rim_intensity;
    rim_power_ = data.rim_power;
    rim_color_ = data.rim_color;
    rim_shadow_affected_ = data.rim_shadow_affected;
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
    cmd.material_instance.specular_intensity = specular_intensity_;
    cmd.material_instance.specular_power = specular_power_;
    cmd.material_instance.rim_intensity = rim_intensity_;
    cmd.material_instance.rim_power = rim_power_;
    cmd.material_instance.rim_color[0] = rim_color_.x;
    cmd.material_instance.rim_color[1] = rim_color_.y;
    cmd.material_instance.rim_color[2] = rim_color_.z;
    cmd.material_instance.rim_shadow_affected = rim_shadow_affected_;

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

  float specular_intensity_ = 0.5f;
  float specular_power_ = 32.0f;
  float rim_intensity_ = 0.0f;
  float rim_power_ = 4.0f;
  Vector3 rim_color_ = {1.0f, 1.0f, 1.0f};
  bool rim_shadow_affected_ = false;

  RenderLayer render_layer_ = RenderLayer::Opaque;
  RenderTagMask render_tags_ = static_cast<uint32_t>(RenderTag::CastShadow | RenderTag::ReceiveShadow | RenderTag::Lit);
};
