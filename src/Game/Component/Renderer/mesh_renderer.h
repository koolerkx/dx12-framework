#pragma once
#include <array>
#include <cstring>

#include "Asset/asset_handle.h"
#include "Asset/asset_manager.h"
#include "Component/renderer_component.h"
#include "Component/transform_component.h"
#include "Framework/Math/Math.h"
#include "Framework/Render/render_handles.h"
#include "Framework/Render/render_settings.h"
#include "Framework/Serialize/serialize_node.h"
#include "Graphic/Frame/frame_packet.h"
#include "Graphic/Pipeline/shader_descriptors.h"
#include "Graphic/Pipeline/shader_registry.h"
#include "Graphic/Resource/Material/material_descriptor_pool.h"
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
    case DefaultMesh::Cylinder:
      return "Cylinder";
  }
  return "Cube";
}

inline DefaultMesh ParseDefaultMesh(const std::string& name) {
  if (name == "Quad") return DefaultMesh::Quad;
  if (name == "Plane") return DefaultMesh::Plane;
  if (name == "Rect") return DefaultMesh::Rect;
  if (name == "Sphere") return DefaultMesh::Sphere;
  if (name == "Cylinder") return DefaultMesh::Cylinder;
  return DefaultMesh::Cube;
}

struct TextureBinding {
  Texture* texture = nullptr;
  std::string path;
  AssetHandle<Texture> handle;

  void SetDirect(Texture* tex) {
    texture = tex;
    path.clear();
    handle = {};
  }

  void Clear() {
    texture = nullptr;
    path.clear();
    handle = {};
  }

  template <typename LoadFn>
  void LoadByPath(const std::string& p, LoadFn&& load_fn) {
    if (p.empty()) {
      Clear();
      return;
    }
    path = p;
    handle = load_fn(p);
    texture = handle.Get();
  }

  void Serialize(framework::SerializeNode& node, const char* key) const {
    if (!path.empty()) node.Write(key, path);
  }

  template <typename LoadFn>
  void Deserialize(const framework::SerializeNode& node, const char* key, LoadFn&& load_fn) {
    auto p = node.ReadString(key);
    if (p.empty()) return;
    LoadByPath(p, std::forward<LoadFn>(load_fn));
  }
};

enum class TextureSlot : uint8_t { Albedo, Normal, MetallicRoughness, Emissive };

class MeshRenderer : public RendererComponent<MeshRenderer> {
 public:
  struct Props {
    DefaultMesh mesh_type = DefaultMesh::Cube;
    const Mesh* mesh = nullptr;
    MeshHandle mesh_handle;
    std::string texture_path = "";
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
    float emissive_intensity = 1.0f;
  };

  using RendererComponent::RendererComponent;

  MeshRenderer(GameObject* owner, const Props& props) : RendererComponent(owner) {
    mesh_handle_ = props.mesh_handle;
    if (props.mesh) {
      SetMesh(props.mesh);
    } else if (!props.mesh_handle.IsValid()) {
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
    SetEmissiveIntensity(props.emissive_intensity);
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

  // --- Texture: immediate setters (game code) ---

  void SetTexture(Texture* texture) {
    albedo_.SetDirect(texture);
    material_dirty_ = true;
  }
  void SetNormalTexture(Texture* texture) {
    normal_.SetDirect(texture);
    material_dirty_ = true;
  }
  void SetMetallicRoughnessTexture(Texture* texture) {
    metallic_roughness_.SetDirect(texture);
    material_dirty_ = true;
  }
  void SetEmissiveTexture(Texture* texture) {
    emissive_.SetDirect(texture);
    material_dirty_ = true;
  }

  void SetTexturePath(const std::string& path) {
    if (path.empty()) {
      albedo_.Clear();
      material_dirty_ = true;
      return;
    }
    auto* context = GetOwner()->GetContext();
    if (!context) return;
    albedo_.LoadByPath(path, [&](const std::string& p) { return context->GetAssetManager().LoadTexture(p); });
    material_dirty_ = true;
  }
  void SetNormalTexturePath(const std::string& path) {
    if (path.empty()) {
      normal_.Clear();
      material_dirty_ = true;
      return;
    }
    auto* context = GetOwner()->GetContext();
    if (!context) return;
    normal_.LoadByPath(path, [&](const std::string& p) { return context->GetAssetManager().LoadTextureLinear(p); });
    material_dirty_ = true;
  }
  void SetMetallicRoughnessPath(const std::string& path) {
    if (path.empty()) {
      metallic_roughness_.Clear();
      material_dirty_ = true;
      return;
    }
    auto* context = GetOwner()->GetContext();
    if (!context) return;
    metallic_roughness_.LoadByPath(path, [&](const std::string& p) { return context->GetAssetManager().LoadTextureLinear(p); });
    material_dirty_ = true;
  }
  void SetEmissivePath(const std::string& path) {
    if (path.empty()) {
      emissive_.Clear();
      material_dirty_ = true;
      return;
    }
    auto* context = GetOwner()->GetContext();
    if (!context) return;
    emissive_.LoadByPath(path, [&](const std::string& p) { return context->GetAssetManager().LoadTexture(p); });
    material_dirty_ = true;
  }

  // --- Texture: deferred loading (editor / mid-frame safe) ---

  void RequestTextureChange(TextureSlot slot, const std::string& path) {
    pending_texture_requests_.push_back({slot, path});
  }

  bool ResolvePendingTextures() {
    if (pending_texture_requests_.empty()) return false;
    auto* context = GetOwner()->GetContext();
    auto resolve_binding = [&](TextureBinding& binding, const std::string& path, bool linear) {
      if (path.empty()) {
        binding.Clear();
        return;
      }
      if (!context) return;
      auto& assets = context->GetAssetManager();
      binding.LoadByPath(path, [&](const std::string& p) { return linear ? assets.LoadTextureLinear(p) : assets.LoadTexture(p); });
    };
    for (auto& req : pending_texture_requests_) {
      switch (req.slot) {
        case TextureSlot::Albedo:
          resolve_binding(albedo_, req.path, false);
          break;
        case TextureSlot::Normal:
          resolve_binding(normal_, req.path, true);
          break;
        case TextureSlot::MetallicRoughness:
          resolve_binding(metallic_roughness_, req.path, true);
          break;
        case TextureSlot::Emissive:
          resolve_binding(emissive_, req.path, false);
          break;
      }
    }
    pending_texture_requests_.clear();
    material_dirty_ = true;
    return true;
  }

  bool HasPendingTextureRequests() const {
    return !pending_texture_requests_.empty();
  }

  // --- Texture: getters ---

  Texture* GetTexture() const {
    return albedo_.texture;
  }
  Texture* GetNormalTexture() const {
    return normal_.texture;
  }
  Texture* GetMetallicRoughnessTexture() const {
    return metallic_roughness_.texture;
  }
  Texture* GetEmissiveTexture() const {
    return emissive_.texture;
  }

  const std::string& GetTexturePath() const {
    return albedo_.path;
  }
  const std::string& GetNormalTexturePath() const {
    return normal_.path;
  }
  const std::string& GetMetallicRoughnessPath() const {
    return metallic_roughness_.path;
  }
  const std::string& GetEmissivePath() const {
    return emissive_.path;
  }

  // --- Other properties ---

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

  template <typename ShaderType>
    requires requires { typename ShaderType::Params; }
  void SetShaderWithParams(const typename ShaderType::Params& params) {
    shader_id_ = ShaderType::ID;
    SetCustomData(params);
    if constexpr (requires { ShaderType::RENDER_LAYER; }) render_layer_ = ShaderType::RENDER_LAYER;
    if constexpr (requires { ShaderType::RENDER_TAGS; }) render_tags_ = ShaderType::RENDER_TAGS;
    if constexpr (requires { ShaderType::DefaultRenderSettings(); }) render_settings_ = ShaderType::DefaultRenderSettings();
  }

  void SetRenderLayer(RenderLayer layer) {
    render_layer_ = layer;
  }
  void SetSpecularIntensity(float intensity) {
    specular_intensity_ = intensity;
    material_dirty_ = true;
  }
  void SetSpecularPower(float power) {
    specular_power_ = power;
    material_dirty_ = true;
  }
  void SetRimIntensity(float intensity) {
    rim_intensity_ = intensity;
    material_dirty_ = true;
  }
  void SetRimPower(float power) {
    rim_power_ = power;
    material_dirty_ = true;
  }
  void SetRimColor(const Vector3& color) {
    rim_color_ = color;
    material_dirty_ = true;
  }
  void SetRimShadowAffected(bool affected) {
    rim_shadow_affected_ = affected;
    material_dirty_ = true;
  }
  void SetMetallic(float metallic) {
    metallic_ = metallic;
    material_dirty_ = true;
  }
  void SetRoughness(float roughness) {
    roughness_ = roughness;
    material_dirty_ = true;
  }
  void SetEmissiveColor(const Vector3& color) {
    emissive_color_ = color;
    material_dirty_ = true;
  }
  void SetEmissiveIntensity(float intensity) {
    emissive_intensity_ = intensity;
    material_dirty_ = true;
  }
  float GetEmissiveIntensity() const {
    return emissive_intensity_;
  }
  void SetRenderTags(RenderTagMask tags) {
    render_tags_ = tags;
  }
  void AddRenderTag(RenderTag tag) {
    render_tags_ |= static_cast<uint32_t>(tag);
  }
  template <typename T>
  void SetCustomData(const T& data) {
    static_assert(std::is_trivially_copyable_v<T>);
    static_assert(sizeof(T) <= sizeof(custom_data_));
    custom_data_ = {};
    memcpy(custom_data_.data(), &data, sizeof(T));
    has_custom_data_ = true;
  }
  const Mesh* GetMesh() const {
    return mesh_;
  }
  Graphics::ShaderId GetShaderId() const {
    return shader_id_;
  }

  // --- Serialization ---

  void OnSerialize(framework::SerializeNode& node) const override {
    if (!mesh_type_name_.empty()) {
      node.Write("MeshType", mesh_type_name_);
    }
    albedo_.Serialize(node, "Texture");
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
    node.Write("EmissiveIntensity", emissive_intensity_);
    normal_.Serialize(node, "NormalTexture");
    metallic_roughness_.Serialize(node, "MetallicRoughnessTexture");
    emissive_.Serialize(node, "EmissiveTexture");
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
    emissive_intensity_ = node.ReadFloat("EmissiveIntensity", emissive_intensity_);

    auto& assets = context->GetAssetManager();

    auto load_srgb = [&](const std::string& p) { return assets.LoadTexture(p); };
    auto load_linear = [&](const std::string& p) { return assets.LoadTextureLinear(p); };
    normal_.Deserialize(node, "NormalTexture", load_linear);
    metallic_roughness_.Deserialize(node, "MetallicRoughnessTexture", load_linear);
    emissive_.Deserialize(node, "EmissiveTexture", load_srgb);
  }

  // --- Editor data ---

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
    float emissive_intensity;
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
      emissive_color_,
      emissive_intensity_};
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
    emissive_intensity_ = data.emissive_intensity;
    material_dirty_ = true;
  }

  // --- Render ---

  void OnRender(FramePacket& packet) override {
    if (!mesh_ && !mesh_handle_.IsValid()) return;

    auto* transform = GetOwner()->GetTransform();
    if (!transform) return;

    auto* context = GetOwner()->GetContext();
    auto& material_mgr = context->GetGraphic()->GetMaterialManager();
    auto& pool = context->GetGraphic()->GetMaterialDescriptorPool();

    if (!material_handle_.IsValid() || material_dirty_) {
      MaterialDescriptor desc = BuildMaterialDescriptor(context);
      if (!material_handle_.IsValid()) {
        material_handle_ = pool.Allocate(desc);
      } else {
        pool.Update(material_handle_, desc);
      }
      material_dirty_ = false;
    }

    Vector3 worldPos = transform->GetWorldPosition();
    Vector3 camPos = packet.main_camera.position;
    float depth = Vector3::DistanceSquared(worldPos, camPos);

    if (mesh_handle_.IsValid()) {
      RenderRequest request;
      request.mesh = mesh_handle_;
      request.shader_id = shader_id_;
      request.render_settings = render_settings_;
      request.material = material_handle_;
      request.color = color_;
      request.world_matrix = transform->GetWorldMatrix();
      request.depth = depth;
      request.layer = render_layer_;
      request.tags = render_tags_;
      if (has_custom_data_) {
        request.custom_data.data = custom_data_;
        request.custom_data.active = true;
      }
      packet.Draw(std::move(request));
    } else {
      DrawCommand cmd;
      cmd.world_matrix = transform->GetWorldMatrix();
      cmd.color = color_;
      cmd.mesh = mesh_;
      cmd.material_handle = material_handle_;
      cmd.material = material_mgr.GetOrCreateMaterial(shader_id_, render_settings_);
      cmd.depth = depth;
      if (has_custom_data_) {
        cmd.custom_data = custom_data_;
        cmd.has_custom_data = true;
      }
      cmd.layer = render_layer_;
      cmd.tags = render_tags_;
      cmd.depth_test = render_settings_.depth_test;
      cmd.depth_write = render_settings_.depth_write;
      packet.AddCommand(std::move(cmd));
    }
  }

  void OnDestroy() override {
    if (material_handle_.IsValid()) {
      auto* context = GetOwner()->GetContext();
      if (context && context->GetGraphic()) {
        context->GetGraphic()->GetMaterialDescriptorPool().Free(material_handle_);
      }
      material_handle_ = MaterialHandle::Invalid();
    }
    RendererComponent::OnDestroy();
  }

 private:
  const Mesh* mesh_ = nullptr;
  MeshHandle mesh_handle_;
  MaterialHandle material_handle_;
  bool material_dirty_ = true;
  TextureBinding albedo_;
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

  TextureBinding normal_;
  TextureBinding metallic_roughness_;
  TextureBinding emissive_;
  float metallic_ = 0.0f;
  float roughness_ = 0.5f;
  Vector3 emissive_color_ = {0.0f, 0.0f, 0.0f};
  float emissive_intensity_ = 1.0f;

  std::array<float, 20> custom_data_{};
  bool has_custom_data_ = false;

  RenderLayer render_layer_ = RenderLayer::Opaque;
  RenderTagMask render_tags_ = static_cast<uint32_t>(RenderTag::CastShadow | RenderTag::ReceiveShadow | RenderTag::Lit);

  struct PendingTextureRequest {
    TextureSlot slot;
    std::string path;
  };
  std::vector<PendingTextureRequest> pending_texture_requests_;

  MaterialDescriptor BuildMaterialDescriptor(GameContext* context) const {
    Texture* effective_texture = albedo_.texture ? albedo_.texture : context->GetAssetManager().GetDefaultWhiteTexture();
    MaterialDescriptor desc{};
    desc.albedo_texture_index = effective_texture ? effective_texture->GetBindlessIndex() : 0;
    desc.normal_texture_index = normal_.texture ? normal_.texture->GetBindlessIndex() : 0;
    desc.metallic_roughness_index = metallic_roughness_.texture ? metallic_roughness_.texture->GetBindlessIndex() : 0;
    desc.emissive_texture_index = emissive_.texture ? emissive_.texture->GetBindlessIndex() : 0;
    desc.flags = flags::If(false, MaterialFlags::AlphaTest) | flags::If(render_settings_.double_sided, MaterialFlags::DoubleSided) |
                 flags::If(rim_shadow_affected_, MaterialFlags::RimShadowAffected) |
                 flags::If(normal_.texture != nullptr, MaterialFlags::HasNormalMap) |
                 flags::If(metallic_roughness_.texture != nullptr, MaterialFlags::HasMetallicRoughnessMap) |
                 flags::If(emissive_.texture != nullptr, MaterialFlags::HasEmissiveMap);
    desc.specular_intensity = specular_intensity_;
    desc.specular_power = specular_power_;
    desc.rim_intensity = rim_intensity_;
    desc.rim_power = rim_power_;
    desc.rim_color = rim_color_;
    desc.metallic_factor = metallic_;
    desc.roughness_factor = roughness_;
    desc.sampler_index = static_cast<uint32_t>(render_settings_.sampler_type);
    desc.emissive_factor =
      Vector3(emissive_color_.x * emissive_intensity_, emissive_color_.y * emissive_intensity_, emissive_color_.z * emissive_intensity_);
    return desc;
  }
};
