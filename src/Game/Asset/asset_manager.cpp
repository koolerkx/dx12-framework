#include "asset_manager.h"

#include <cstdio>
#include <filesystem>
#include <limits>
#include <optional>

#include "Framework/Logging/logger.h"
#include "Framework/Model/model_loader.h"
#include "Graphic/Pipeline/vertex_types.h"
#include "Graphic/Resource/Font/sprite_font_manager.h"
#include "Graphic/Resource/Texture/texture_manager.h"
#include "Graphic/Resource/Mesh/mesh_buffer_pool.h"
#include "Graphic/Resource/mesh_factory.h"
#include "Graphic/Resource/mesh_registry.h"
#include "Graphic/graphic.h"
#include "text_mesh_handle.h"

class AssetManager::Impl {
 public:
  Graphic* graphic = nullptr;
  TextureManager* texture_manager = nullptr;
  Font::SpriteFontManager* font_manager = nullptr;
  MeshBufferPool* mesh_buffer_pool = nullptr;
};

// must defined in cpp
AssetManager::AssetManager() = default;
AssetManager::~AssetManager() = default;
AssetManager::AssetManager(AssetManager&&) noexcept = default;
AssetManager& AssetManager::operator=(AssetManager&&) noexcept = default;

bool AssetManager::Initialize(Graphic* graphic) {
  impl_ = std::make_unique<Impl>();
  impl_->graphic = graphic;
  impl_->texture_manager = &graphic->GetTextureManager();
  impl_->font_manager = &graphic->GetSpriteFontManager();
  impl_->mesh_buffer_pool = &graphic->GetMeshBufferPool();
  CreateDefaultMeshes();
  default_white_texture_ = impl_->texture_manager->LoadTextureSRGB("Content/textures/white.png");
  return true;
}

void AssetManager::Shutdown() {
  for (auto& [key, model_data] : model_cache_) {
    if (model_data) FreeMeshHandlesForModel(*model_data);
  }
  model_cache_.clear();
  mesh_handle_cache_.clear();
  impl_.reset();
}

AssetHandle<Texture> AssetManager::LoadTexture(const std::string& path) {
  auto texture = impl_->texture_manager->LoadTexture(path);
  return AssetHandle<Texture>(texture, path);
}

AssetHandle<Texture> AssetManager::LoadTextureLinear(const std::string& path) {
  auto texture = impl_->texture_manager->LoadTextureLinear(path);
  return AssetHandle<Texture>(texture, path);
}

AssetHandle<Texture> AssetManager::LoadCubemap(const std::string& path) {
  auto texture = impl_->texture_manager->LoadCubemapFromCrossHDR(path);
  return AssetHandle<Texture>(texture, path);
}

std::vector<AssetHandle<Texture>> AssetManager::LoadTextures(const std::vector<std::string>& paths) {
  auto textures = impl_->texture_manager->LoadTextures(paths);

  std::vector<AssetHandle<Texture>> handles;
  handles.reserve(textures.size());
  for (size_t i = 0; i < textures.size(); ++i) {
    handles.emplace_back(textures[i], paths[i]);
  }
  return handles;
}

void AssetManager::ProcessDeferredCleanup(uint64_t completed_fence_value) {
  impl_->texture_manager->ProcessDeferredFrees(completed_fence_value);
}

void AssetManager::ClearUploadBuffers() {
  impl_->texture_manager->CleanUploadBuffers();
}

Graphic* AssetManager::GetGraphicForDebugUseOnly() const {
  return impl_.get()->graphic;
}

void AssetManager::CreateDefaultMeshes() {
  if (!impl_->graphic) return;

  ID3D12Device* device = impl_->graphic->GetDevice();
  auto& registry = impl_->graphic->GetMeshRegistry();

  auto register_default = [&](DefaultMesh type, const std::string& key, auto create_fn) {
    auto mesh = std::make_unique<Mesh>();
    if (create_fn(device, *mesh)) {
      default_meshes_[type] = registry.Register(key, std::move(mesh));
    }
  };

  register_default(DefaultMesh::Quad, "default:quad", [](auto* d, auto& m) { return MeshFactory::CreateQuad(d, m); });
  register_default(DefaultMesh::Cube, "default:cube", [](auto* d, auto& m) { return MeshFactory::CreateCube(d, m); });
  register_default(DefaultMesh::Rect, "default:rect", [](auto* d, auto& m) { return MeshFactory::CreateRect(d, m); });
  register_default(DefaultMesh::Plane, "default:plane", [](auto* d, auto& m) { return MeshFactory::CreatePlane(d, m, 10, 10); });
  register_default(DefaultMesh::Sphere, "default:sphere", [](auto* d, auto& m) { return MeshFactory::CreateSphere(d, m, 32, 16); });
  register_default(DefaultMesh::Cylinder, "default:cylinder", [](auto* d, auto& m) { return MeshFactory::CreateCylinder(d, m); });
  register_default(DefaultMesh::RoundedRect, "default:rounded_rect", [](auto* d, auto& m) { return MeshFactory::CreateRoundedRect(d, m); });
}

void AssetManager::CreateTextureFromPixels(const std::string& cache_key, const uint8_t* pixels, uint32_t width, uint32_t height) {
  impl_->texture_manager->LoadTextureFromRawPixels(cache_key, pixels, width, height, false);
}

const Mesh* AssetManager::CreateCube(const std::string& key, const CubeCornerColors& corner_colors) {
  auto& registry = impl_->graphic->GetMeshRegistry();
  if (auto* existing = registry.Find(key)) {
    return existing;
  }

  auto mesh = std::make_unique<Mesh>();
  if (!MeshFactory::CreateCube(impl_->graphic->GetDevice(), *mesh, corner_colors)) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Game, Logger::Here(), "[AssetManager] Failed to create cube: {}", key);
    return nullptr;
  }
  return registry.Register(key, std::move(mesh));
}

const Mesh* AssetManager::CreateRoundedRect(const std::string& key, float aspect_ratio, float corner_radius) {
  auto& registry = impl_->graphic->GetMeshRegistry();
  if (auto* existing = registry.Find(key)) {
    return existing;
  }

  auto mesh = std::make_unique<Mesh>();
  bool ok = MeshFactory::CreateRoundedRect(impl_->graphic->GetDevice(), *mesh, corner_radius, 8, aspect_ratio);
  if (!ok) {
    return nullptr;
  }
  return registry.Register(key, std::move(mesh));
}

const Mesh* AssetManager::GetDefaultMesh(DefaultMesh type) const {
  auto it = default_meshes_.find(type);
  if (it != default_meshes_.end()) {
    return it->second;
  }

  Logger::LogFormat(
    LogLevel::Error, LogCategory::Game, Logger::Here(), "[AssetManager] Default mesh not found: {}", static_cast<int>(type));
  return nullptr;
}

std::optional<DefaultMesh> AssetManager::FindDefaultMeshType(const Mesh* mesh) const {
  for (const auto& [type, ptr] : default_meshes_) {
    if (ptr == mesh) return type;
  }
  return std::nullopt;
}

std::shared_ptr<ModelData> AssetManager::LoadModel(const std::string& path, float global_scale) {
  auto format_scale = [](float s) -> std::string {
    char buf[32];
    std::snprintf(buf, sizeof(buf), "%.6g", s);
    return buf;
  };

  std::string scale_suffix = (global_scale == 1.0f) ? "" : "@" + format_scale(global_scale);
  std::string cache_key = path + scale_suffix;
  auto cache_it = model_cache_.find(cache_key);
  if (cache_it != model_cache_.end()) {
    return cache_it->second;
  }

  auto* texture_manager = impl_->texture_manager;

  using Loader = Model::ModelLoader<MeshHandle, std::shared_ptr<Texture>, ModelSurfaceMaterial>;

  std::filesystem::path model_dir = std::filesystem::path(path).parent_path();

  auto load_texture_by_usage = [&](const std::string& tex_path, bool is_color) -> std::shared_ptr<Texture> {
    return is_color ? texture_manager->LoadTextureSRGB(tex_path) : texture_manager->LoadTextureLinear(tex_path);
  };

  auto texture_cb = [&](const Model::TextureRef& tex_ref) -> std::shared_ptr<Texture> {
    bool is_color = tex_ref.type == Model::TextureType::Color;

    if (tex_ref.embedded.has_value()) {
      auto& emb = *tex_ref.embedded;
      std::string cache_key = path + "#emb_" + tex_ref.path;

      std::shared_ptr<Texture> texture;
      if (emb.is_compressed) {
        texture = texture_manager->LoadTextureFromMemory(cache_key, emb.data.data(), emb.data.size(), is_color);
      } else {
        texture = texture_manager->LoadTextureFromRawPixels(cache_key, emb.data.data(), emb.width, emb.height, is_color);
      }

      if (!texture) {
        Logger::LogFormat(LogLevel::Error,
          LogCategory::Game,
          Logger::Here(),
          "[AssetManager] Failed to load embedded texture '{}' from model '{}'",
          tex_ref.path,
          path);
      }
      return texture ? texture : default_white_texture_;
    }

    std::filesystem::path tex_path(tex_ref.path);

    // attempt using the path exactly as specified by the model (may be absolute or relative)
    auto texture = load_texture_by_usage(tex_ref.path, is_color);

    if (!texture) {
      // attempt by resolving the model-relative path (model directory + path)
      std::string relative_str = (model_dir / tex_path).string();
      texture = load_texture_by_usage(relative_str, is_color);
    }

    if (!texture && tex_path.has_filename()) {
      // attempt using only the filename inside the model's directory
      std::string filename_str = (model_dir / tex_path.filename()).string();
      texture = load_texture_by_usage(filename_str, is_color);
    }

    if (!texture) {
      Logger::LogFormat(LogLevel::Error,
        LogCategory::Game,
        Logger::Here(),
        "[AssetManager] Failed to load texture '{}' for model '{}'",
        tex_ref.path,
        path);
      return default_white_texture_;
    }
    return texture;
  };

  std::vector<std::optional<uint32_t>> material_albedo_texture_indices;
  std::vector<std::optional<uint32_t>> material_normal_texture_indices;
  std::vector<std::optional<uint32_t>> material_metallic_roughness_indices;
  std::vector<std::optional<uint32_t>> material_emissive_texture_indices;

  auto material_cb = [&](const Model::SurfaceMaterialData& mat) -> ModelSurfaceMaterial {
    material_albedo_texture_indices.push_back(mat.base_color_texture_index);
    material_normal_texture_indices.push_back(mat.normal_texture_index);
    material_metallic_roughness_indices.push_back(mat.metallic_roughness_texture_index);
    material_emissive_texture_indices.push_back(mat.emissive_texture_index);
    return ModelSurfaceMaterial{
      .base_color_factor = mat.base_color_factor,
      .emissive_factor = mat.emissive_factor,
      .metallic_factor = mat.metallic_factor,
      .roughness_factor = mat.roughness_factor,
      .is_double_sided = mat.is_double_sided,
    };
  };

  std::vector<uint32_t> mesh_material_indices;
  uint32_t mesh_counter = 0;
  float global_min_y = (std::numeric_limits<float>::max)();
  Math::Vector3 bounds_min((std::numeric_limits<float>::max)(), (std::numeric_limits<float>::max)(), (std::numeric_limits<float>::max)());
  Math::Vector3 bounds_max(
    -(std::numeric_limits<float>::max)(), -(std::numeric_limits<float>::max)(), -(std::numeric_limits<float>::max)());

  auto mesh_cb = [&](const Model::MeshData<Graphics::Vertex::ModelVertex>& mesh_data) -> MeshHandle {
    std::string key = path + scale_suffix + "#" + mesh_data.name + "_" + std::to_string(mesh_counter++);
    mesh_material_indices.push_back(mesh_data.material_index);

    for (const auto& vertex : mesh_data.vertices) {
      if (vertex.position.y < global_min_y) global_min_y = vertex.position.y;
      Math::Vector3 pos(vertex.position.x, vertex.position.y, vertex.position.z);
      bounds_min = Math::Vector3::Min(bounds_min, pos);
      bounds_max = Math::Vector3::Max(bounds_max, pos);
    }

    if (auto cache_it = mesh_handle_cache_.find(key); cache_it != mesh_handle_cache_.end()) {
      return cache_it->second;
    }

    auto alloc = impl_->mesh_buffer_pool->Allocate(
      std::span{mesh_data.vertices.data(), mesh_data.vertices.size()},
      std::span{mesh_data.indices.data(), mesh_data.indices.size()});
    if (alloc.success) {
      mesh_handle_cache_[key] = alloc.handle;
    }
    return alloc.handle;
  };

  Model::LoadOptions load_options;
  load_options.global_scale = global_scale;
  auto result = Loader::Load<Graphics::Vertex::ModelVertex, MeshHandle, ModelSurfaceMaterial, std::shared_ptr<Texture>>(
    path, texture_cb, material_cb, mesh_cb, load_options);

  if (!result.success) {
    Logger::LogFormat(
      LogLevel::Error, LogCategory::Game, Logger::Here(), "[AssetManager] Failed to load model: {} - {}", path, result.error_message);
    return nullptr;
  }

  auto model_data = std::make_shared<ModelData>();
  model_data->path = path;
  model_data->textures = std::move(result.texture_handles);
  model_data->surface_materials = std::move(result.surface_material_handles);
  model_data->root_node = std::move(result.root_node);
  model_data->min_y = (global_min_y < (std::numeric_limits<float>::max)()) ? global_min_y : 0.0f;
  if (bounds_min.x <= bounds_max.x) {
    model_data->bounds = Math::AABB(bounds_min, bounds_max);
  }

  auto resolve_texture = [&](const std::vector<std::optional<uint32_t>>& indices, uint32_t mat_idx) -> std::shared_ptr<Texture> {
    if (mat_idx < indices.size()) {
      auto tex_idx = indices[mat_idx];
      if (tex_idx.has_value() && *tex_idx < model_data->textures.size()) {
        return model_data->textures[*tex_idx];
      }
    }
    return nullptr;
  };

  model_data->sub_meshes.reserve(result.mesh_handles.size());
  for (size_t i = 0; i < result.mesh_handles.size(); ++i) {
    if (!result.mesh_handles[i].IsValid()) continue;

    uint32_t mat_idx = mesh_material_indices[i];

    ModelSubMeshEntry entry;
    entry.mesh_handle = result.mesh_handles[i];
    entry.surface_material_index = mat_idx;
    entry.albedo_texture = resolve_texture(material_albedo_texture_indices, mat_idx);
    entry.normal_texture = resolve_texture(material_normal_texture_indices, mat_idx);
    entry.metallic_roughness_texture = resolve_texture(material_metallic_roughness_indices, mat_idx);
    entry.emissive_texture = resolve_texture(material_emissive_texture_indices, mat_idx);

    model_data->sub_meshes.push_back(entry);
  }

  model_cache_[cache_key] = model_data;
  return model_data;
}

void AssetManager::FreeMeshHandlesForModel(const ModelData& model_data) {
  for (const auto& entry : model_data.sub_meshes) {
    if (entry.mesh_handle.IsValid()) {
      impl_->mesh_buffer_pool->Free(entry.mesh_handle);
    }
  }
}

bool AssetManager::LoadFont(Font::FontFamily family, const std::string& fnt_path, const std::string& texture_path) {
  if (!impl_->font_manager) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Game, Logger::Here(), "[AssetManager] Font manager not initialized");
    return false;
  }

  return impl_->font_manager->LoadFontVariant(family, fnt_path, texture_path);
}

TextMeshHandle AssetManager::CreateTextMesh(
  const std::wstring& text, Font::FontFamily family, float pixel_size, const Text::TextLayoutProps& layout_props) {
  if (!impl_->font_manager) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Game, Logger::Here(), "[AssetManager] Font manager not initialized");
    return TextMeshHandle();
  }

  // Create text layout - pure CPU data
  Font::TextLayoutData layout_data;
  if (!impl_->font_manager->CreateTextLayout(text, family, pixel_size, layout_props, layout_data)) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Game, Logger::Here(), "[AssetManager] Failed to create text layout");
    return TextMeshHandle();
  }

  // Convert to GlyphLayoutData with UV transform data
  std::vector<GlyphLayoutData> glyphs;
  glyphs.reserve(layout_data.glyphs.size());

  for (const auto& glyph : layout_data.glyphs) {
    GlyphLayoutData glyph_data;
    glyph_data.x = glyph.x;
    glyph_data.y = glyph.y;
    glyph_data.width = glyph.width;
    glyph_data.height = glyph.height;

    glyph_data.uv_offset = {glyph.u0, glyph.v1};
    glyph_data.uv_scale = {glyph.u1 - glyph.u0, glyph.v0 - glyph.v1};
    glyphs.push_back(glyph_data);
  }

  // Get texture from layout (non-owning pointer)
  Texture* texture = layout_data.variant ? layout_data.variant->texture.get() : nullptr;

  // Return handle with pure CPU data - no mesh ownership
  // TextRenderer will use shared Quad mesh from AssetManager
  return TextMeshHandle(std::move(glyphs), layout_data.width, layout_data.height, texture);
}
