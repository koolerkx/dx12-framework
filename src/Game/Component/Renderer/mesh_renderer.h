#pragma once
#include "Asset/asset_handle.h"
#include "Asset/asset_manager.h"
#include "Component/component.h"
#include "Component/render_settings.h"
#include "Component/transform_component.h"
#include "Framework/Math/Math.h"
#include "Framework/Serialize/serialize_node.h"
#include "Graphic/Frame/frame_packet.h"
#include "Graphic/Pipeline/shader_descriptors.h"
#include "Graphic/Pipeline/shader_registry.h"
#include "Graphic/Resource/Texture/texture.h"
#include "Graphic/Resource/mesh.h"
#include "game_context.h"
#include "game_object.h"
#include "scene.h"


using Math::Vector3;
using Math::Vector4;

inline std::string DefaultMeshToString(DefaultMesh type) {
  switch (type) {
    case DefaultMesh::Quad:
      return "Quad";
    case DefaultMesh::Cube:
      return "Cube";
    case DefaultMesh::Plane:
      return "Plane";
    case DefaultMesh::Rect:
      return "Rect";
    case DefaultMesh::Sphere:
      return "Sphere";
  }
  return "Cube";
}

inline DefaultMesh ParseDefaultMesh(const std::string& name) {
  if (name == "Quad") return DefaultMesh::Quad;
  if (name == "Plane") return DefaultMesh::Plane;
  if (name == "Rect") return DefaultMesh::Rect;
  if (name == "Sphere") return DefaultMesh::Sphere;
  return DefaultMesh::Cube;
}

class MeshRenderer : public Component<MeshRenderer> {
 public:
  struct Props {
    DefaultMesh mesh_type = DefaultMesh::Cube;
    const Mesh* mesh = nullptr;
    std::string texture_path;
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
    Texture* normal_texture = nullptr;
    Texture* metallic_roughness_texture = nullptr;
    Texture* emissive_texture = nullptr;
    float metallic = 0.0f;
    float roughness = 0.5f;
    Vector3 emissive_color = {0, 0, 0};
  };

  using Component::Component;

  MeshRenderer(GameObject* owner, const Props& props) : Component(owner) {
    if (props.mesh) {
      SetMesh(props.mesh);
    } else {
      SetDefaultMesh(props.mesh_type);
    }
    if (props.texture) {
      SetTexture(props.texture);
    } else if (!props.texture_path.empty()) {
      SetTexturePath(props.texture_path);
    }
    SetColor(props.color);
    SetShaderId(props.shader_id);
    SetRenderLayer(props.render_layer);
    SetSpecularIntensity(props.specular_intensity);
    SetSpecularPower(props.specular_power);
    SetRimIntensity(props.rim_intensity);
    SetRimPower(props.rim_power);
    SetRimColor(props.rim_color);
    SetRimShadowAffected(props.rim_shadow_affected);
    if (props.normal_texture) SetNormalTexture(props.normal_texture);
    if (props.metallic_roughness_texture) SetMetallicRoughnessTexture(props.metallic_roughness_texture);
    if (props.emissive_texture) SetEmissiveTexture(props.emissive_texture);
    SetMetallic(props.metallic);
    SetRoughness(props.roughness);
    SetEmissiveColor(props.emissive_color);
  }

  void SetMesh(const Mesh* mesh) {
    mesh_ = mesh;
    mesh_type_name_.clear();
  }

  void SetDefaultMesh(DefaultMesh type) {
    auto* context = GetOwner()->GetContext();
    if (!context) return;
    mesh_ = context->GetAssetManager().GetDefaultMesh(type);
    mesh_type_name_ = DefaultMeshToString(type);
  }

  void SetTexture(Texture* texture) {
    texture_ = texture;
    texture_path_.clear();
    texture_handle_ = {};
  }

  void SetTexturePath(const std::string& path) {
    texture_path_ = path;
    if (path.empty()) {
      texture_ = nullptr;
      texture_handle_ = {};
      return;
    }
    auto* context = GetOwner()->GetContext();
    if (!context) return;
    texture_handle_ = context->GetAssetManager().LoadTexture(path);
    texture_ = texture_handle_.Get();
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
  void SetNormalTexture(Texture* texture) {
    normal_texture_ = texture;
  }
  void SetMetallicRoughnessTexture(Texture* texture) {
    metallic_roughness_texture_ = texture;
  }
  void SetEmissiveTexture(Texture* texture) {
    emissive_texture_ = texture;
  }
  void SetMetallic(float metallic) {
    metallic_ = metallic;
  }
  void SetRoughness(float roughness) {
    roughness_ = roughness;
  }
  void SetEmissiveColor(const Vector3& color) {
    emissive_color_ = color;
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

  Texture* GetMetallicRoughnessTexture() const {
    return metallic_roughness_texture_;
  }

  Texture* GetEmissiveTexture() const {
    return emissive_texture_;
  }

  Texture* GetNormalTexture() const {
    return normal_texture_;
  }

  void OnSerialize(framework::SerializeNode& node) const override {
    if (!mesh_type_name_.empty()) {
      node.Write("MeshType", mesh_type_name_);
    }
    if (!texture_path_.empty()) {
      node.Write("Texture", texture_path_);
    }
    node.WriteVec4("Color", color_.x, color_.y, color_.z, color_.w);
    auto shader_name = ShaderRegistry::GetName(shader_id_);
    node.Write("Shader", std::string(shader_name));
    node.Write("RenderLayer", render_layer_ == RenderLayer::Opaque ? "Opaque" : "Transparent");
    node.Write("BlendMode", static_cast<int>(render_settings_.blend_mode));
    node.Write("SamplerType", static_cast<int>(render_settings_.sampler_type));
    node.Write("DepthTest", render_settings_.depth_test);
    node.Write("DepthWrite", render_settings_.depth_write);
    node.Write("DoubleSided", render_settings_.double_sided);
    node.Write("RenderTags", static_cast<int>(render_tags_));
    node.Write("SpecularIntensity", specular_intensity_);
    node.Write("SpecularPower", specular_power_);
    node.Write("RimIntensity", rim_intensity_);
    node.Write("RimPower", rim_power_);
    node.WriteVec3("RimColor", rim_color_.x, rim_color_.y, rim_color_.z);
    node.Write("RimShadowAffected", rim_shadow_affected_);
    node.Write("Metallic", metallic_);
    node.Write("Roughness", roughness_);
    node.WriteVec3("EmissiveColor", emissive_color_.x, emissive_color_.y, emissive_color_.z);
  }

  void OnDeserialize(const framework::SerializeNode& node) override {
    auto mesh_type_str = node.ReadString("MeshType", "Cube");
    auto* context = GetOwner()->GetContext();
    if (context) {
      mesh_ = context->GetAssetManager().GetDefaultMesh(ParseDefaultMesh(mesh_type_str));
      mesh_type_name_ = mesh_type_str;
    }
    auto tex_path = node.ReadString("Texture");
    if (!tex_path.empty()) {
      SetTexturePath(tex_path);
    }
    node.ReadVec4("Color", color_.x, color_.y, color_.z, color_.w);
    auto shader_name = node.ReadString("Shader");
    if (!shader_name.empty()) {
      auto id = ShaderRegistry::FindIdByName(shader_name);
      if (id) shader_id_ = *id;
    }
    auto layer_str = node.ReadString("RenderLayer", "Opaque");
    render_layer_ = (layer_str == "Opaque") ? RenderLayer::Opaque : RenderLayer::Transparent;
    render_settings_.blend_mode =
      static_cast<Rendering::BlendMode>(node.ReadInt("BlendMode", static_cast<int>(render_settings_.blend_mode)));
    render_settings_.sampler_type =
      static_cast<Rendering::SamplerType>(node.ReadInt("SamplerType", static_cast<int>(render_settings_.sampler_type)));
    render_settings_.depth_test = node.ReadBool("DepthTest", render_settings_.depth_test);
    render_settings_.depth_write = node.ReadBool("DepthWrite", render_settings_.depth_write);
    render_settings_.double_sided = node.ReadBool("DoubleSided", render_settings_.double_sided);
    render_tags_ = static_cast<RenderTagMask>(node.ReadInt("RenderTags", static_cast<int>(render_tags_)));
    specular_intensity_ = node.ReadFloat("SpecularIntensity", specular_intensity_);
    specular_power_ = node.ReadFloat("SpecularPower", specular_power_);
    rim_intensity_ = node.ReadFloat("RimIntensity", rim_intensity_);
    rim_power_ = node.ReadFloat("RimPower", rim_power_);
    node.ReadVec3("RimColor", rim_color_.x, rim_color_.y, rim_color_.z);
    rim_shadow_affected_ = node.ReadBool("RimShadowAffected", rim_shadow_affected_);
    metallic_ = node.ReadFloat("Metallic", metallic_);
    roughness_ = node.ReadFloat("Roughness", roughness_);
    node.ReadVec3("EmissiveColor", emissive_color_.x, emissive_color_.y, emissive_color_.z);
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
    float metallic;
    float roughness;
    Vector3 emissive_color;
  };

  EditorData GetEditorData() const {
    return {color_,
      render_settings_,
      render_layer_,
      render_tags_,
      specular_intensity_,
      specular_power_,
      rim_intensity_,
      rim_power_,
      rim_color_,
      rim_shadow_affected_,
      metallic_,
      roughness_,
      emissive_color_};
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
    metallic_ = data.metallic;
    roughness_ = data.roughness;
    emissive_color_ = data.emissive_color;
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
    Texture* effective_texture = texture_ ? texture_ : context->GetAssetManager().GetDefaultWhiteTexture();
    cmd.material_instance.albedo_texture_index = effective_texture ? effective_texture->GetBindlessIndex() : 0;
    cmd.material_instance.sampler_index = static_cast<uint32_t>(render_settings_.sampler_type);
    cmd.material_instance.specular_intensity = specular_intensity_;
    cmd.material_instance.specular_power = specular_power_;
    cmd.material_instance.rim_intensity = rim_intensity_;
    cmd.material_instance.rim_power = rim_power_;
    cmd.material_instance.rim_color[0] = rim_color_.x;
    cmd.material_instance.rim_color[1] = rim_color_.y;
    cmd.material_instance.rim_color[2] = rim_color_.z;
    cmd.material_instance.rim_shadow_affected = rim_shadow_affected_;
    cmd.material_instance.normal_texture_index = normal_texture_ ? normal_texture_->GetBindlessIndex() : 0;
    cmd.material_instance.metallic_roughness_index = metallic_roughness_texture_ ? metallic_roughness_texture_->GetBindlessIndex() : 0;
    cmd.material_instance.emissive_texture_index = emissive_texture_ ? emissive_texture_->GetBindlessIndex() : 0;
    cmd.material_instance.has_normal_map = (normal_texture_ != nullptr);
    cmd.material_instance.has_metallic_roughness_map = (metallic_roughness_texture_ != nullptr);
    cmd.material_instance.has_emissive_map = (emissive_texture_ != nullptr);
    cmd.material_instance.metallic_factor = metallic_;
    cmd.material_instance.roughness_factor = roughness_;
    cmd.material_instance.emissive_factor[0] = emissive_color_.x;
    cmd.material_instance.emissive_factor[1] = emissive_color_.y;
    cmd.material_instance.emissive_factor[2] = emissive_color_.z;

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
  std::string texture_path_;
  AssetHandle<Texture> texture_handle_;
  std::string mesh_type_name_;
  Graphics::ShaderId shader_id_ = Graphics::Basic3DShader::ID;
  Rendering::RenderSettings render_settings_ = Rendering::RenderSettings::Opaque();
  Vector4 color_ = {1.0f, 1.0f, 1.0f, 1.0f};

  float specular_intensity_ = 0.5f;
  float specular_power_ = 32.0f;
  float rim_intensity_ = 0.0f;
  float rim_power_ = 4.0f;
  Vector3 rim_color_ = {1.0f, 1.0f, 1.0f};
  bool rim_shadow_affected_ = false;

  Texture* normal_texture_ = nullptr;
  Texture* metallic_roughness_texture_ = nullptr;
  Texture* emissive_texture_ = nullptr;
  float metallic_ = 0.0f;
  float roughness_ = 0.5f;
  Vector3 emissive_color_ = {0.0f, 0.0f, 0.0f};

  RenderLayer render_layer_ = RenderLayer::Opaque;
  RenderTagMask render_tags_ = static_cast<uint32_t>(RenderTag::CastShadow | RenderTag::ReceiveShadow | RenderTag::Lit);
};
