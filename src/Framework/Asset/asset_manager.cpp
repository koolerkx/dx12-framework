#include "asset_manager.h"

#include <cstdio>
#include <filesystem>
#include <limits>
#include <optional>

#include "Framework/Asset/font_service.h"
#include "Framework/Asset/mesh_service.h"
#include "Framework/Asset/texture_service.h"
#include "Framework/Font/text_layout_data.h"
#include "Framework/Logging/logger.h"
#include "Framework/Model/model_loader.h"
#include "Framework/Render/mesh_data.h"
#include "Framework/Render/mesh_data_factory.h"
#include "Framework/Render/render_handles.h"
#include "Framework/Render/vertex_data.h"

class AssetManager::Impl {
 public:
  ITextureService* texture_service = nullptr;
  IMeshService* mesh_service = nullptr;
  IFontService* font_service = nullptr;
};

// must defined in cpp
AssetManager::AssetManager() = default;
AssetManager::~AssetManager() = default;
AssetManager::AssetManager(AssetManager&&) noexcept = default;
AssetManager& AssetManager::operator=(AssetManager&&) noexcept = default;

bool AssetManager::Initialize(const AssetServices& services) {
  impl_ = std::make_unique<Impl>();
  impl_->texture_service = &services.texture;
  impl_->mesh_service = &services.mesh;
  impl_->font_service = &services.font;
  UploadDefaultMeshesToPool();
  auto white_tex = impl_->texture_service->LoadTextureSRGB("Content/textures/white.png");
  default_white_texture_ = white_tex;
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

TextureHandle AssetManager::LoadTexture(const std::string& path) {
  return impl_->texture_service->LoadTextureSRGB(path);
}

TextureHandle AssetManager::LoadTextureLinear(const std::string& path) {
  return impl_->texture_service->LoadTextureLinear(path);
}

TextureHandle AssetManager::LoadCubemap(const std::string& path) {
  return impl_->texture_service->LoadCubemap(path);
}

std::vector<TextureHandle> AssetManager::LoadTextures(const std::vector<std::string>& paths) {
  return impl_->texture_service->LoadTextures(paths);
}

void AssetManager::ProcessDeferredCleanup(uint64_t completed_fence_value) {
  impl_->texture_service->ProcessDeferredFrees(completed_fence_value);
}

void AssetManager::ClearUploadBuffers() {
  impl_->texture_service->CleanUploadBuffers();
}

void AssetManager::UploadDefaultMeshesToPool() {
  if (!impl_->mesh_service) return;
  auto* service = impl_->mesh_service;

  auto upload = [&](DefaultMesh type, const MeshData& data) {
    auto alloc = service->Allocate(data);
    if (alloc.success) default_mesh_handles_[type] = alloc.handle;
  };

  upload(DefaultMesh::Quad, MeshDataFactory::CreateQuadData());
  upload(DefaultMesh::Cube, MeshDataFactory::CreateCubeData());
  upload(DefaultMesh::Plane, MeshDataFactory::CreatePlaneData(10, 10));
  upload(DefaultMesh::Sphere, MeshDataFactory::CreateSphereData(32, 16));
  upload(DefaultMesh::Cylinder, MeshDataFactory::CreateCylinderData());
  upload(DefaultMesh::Rect, MeshDataFactory::CreateRectData());
  upload(DefaultMesh::RoundedRect, MeshDataFactory::CreateRoundedRectData());
}

MeshHandle AssetManager::GetDefaultMeshHandle(DefaultMesh type) const {
  auto it = default_mesh_handles_.find(type);
  if (it != default_mesh_handles_.end()) return it->second;
  return MeshHandle::Invalid();
}

MeshHandle AssetManager::GetOrCreateMeshHandle(const std::string& key, const MeshData& data) {
  auto [it, inserted] = mesh_handle_cache_.try_emplace(key, MeshHandle::Invalid());
  if (!inserted) return it->second;
  if (!impl_->mesh_service) return MeshHandle::Invalid();
  MeshAllocation alloc = impl_->mesh_service->Allocate(data);
  if (!alloc.success) return MeshHandle::Invalid();
  it->second = alloc.handle;
  return alloc.handle;
}

void AssetManager::CreateTextureFromPixels(const std::string& cache_key, const uint8_t* pixels, uint32_t width, uint32_t height) {
  impl_->texture_service->LoadTextureFromRawPixels(cache_key, pixels, width, height, false);
}

MeshHandle AssetManager::CreateCubeMesh(const std::string& key, const CubeCornerColors& corner_colors) {
  return GetOrCreateMeshHandle(key, MeshDataFactory::CreateCubeData(corner_colors));
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

  auto* texture_service = impl_->texture_service;

  using Loader = Model::ModelLoader<MeshHandle, TextureHandle, ModelSurfaceMaterial>;

  std::filesystem::path model_dir = std::filesystem::path(path).parent_path();

  auto load_texture_by_usage = [&](const std::string& tex_path, bool is_color) -> TextureHandle {
    return is_color ? texture_service->LoadTextureSRGB(tex_path) : texture_service->LoadTextureLinear(tex_path);
  };

  auto texture_cb = [&](const Model::TextureRef& tex_ref) -> TextureHandle {
    bool is_color = tex_ref.type == Model::TextureType::Color;

    if (tex_ref.embedded.has_value()) {
      auto& emb = *tex_ref.embedded;
      std::string emb_cache_key = path + "#emb_" + tex_ref.path;

      TextureHandle handle;
      if (emb.is_compressed) {
        handle = texture_service->LoadTextureFromMemory(emb_cache_key, emb.data.data(), emb.data.size(), is_color);
      } else {
        handle = texture_service->LoadTextureFromRawPixels(emb_cache_key, emb.data.data(), emb.width, emb.height, is_color);
      }

      if (!handle.IsValid()) {
        Logger::LogFormat(LogLevel::Error,
          LogCategory::Game,
          Logger::Here(),
          "[AssetManager] Failed to load embedded texture '{}' from model '{}'",
          tex_ref.path,
          path);
        return texture_service->LoadTextureSRGB("Content/textures/white.png");
      }
      return handle;
    }

    std::filesystem::path tex_path(tex_ref.path);

    auto handle = load_texture_by_usage(tex_ref.path, is_color);

    if (!handle.IsValid()) {
      std::string relative_str = (model_dir / tex_path).string();
      handle = load_texture_by_usage(relative_str, is_color);
    }

    if (!handle.IsValid() && tex_path.has_filename()) {
      std::string filename_str = (model_dir / tex_path.filename()).string();
      handle = load_texture_by_usage(filename_str, is_color);
    }

    if (!handle.IsValid()) {
      Logger::LogFormat(LogLevel::Error,
        LogCategory::Game,
        Logger::Here(),
        "[AssetManager] Failed to load texture '{}' for model '{}'",
        tex_ref.path,
        path);
      return texture_service->LoadTextureSRGB("Content/textures/white.png");
    }
    return handle;
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

  auto mesh_cb = [&](const Model::MeshData<VertexData::ModelVertex>& mesh_data) -> MeshHandle {
    std::string key = path + scale_suffix + "#" + mesh_data.name + "_" + std::to_string(mesh_counter++);
    mesh_material_indices.push_back(mesh_data.material_index);

    for (const auto& vertex : mesh_data.vertices) {
      if (vertex.position.y < global_min_y) global_min_y = vertex.position.y;
      Math::Vector3 pos(vertex.position.x, vertex.position.y, vertex.position.z);
      bounds_min = Math::Vector3::Min(bounds_min, pos);
      bounds_max = Math::Vector3::Max(bounds_max, pos);
    }

    if (auto found = mesh_handle_cache_.find(key); found != mesh_handle_cache_.end()) {
      return found->second;
    }

    MeshData md;
    md.vertices = mesh_data.vertices;
    md.indices = mesh_data.indices;

    auto alloc = impl_->mesh_service->Allocate(md);
    if (alloc.success) {
      mesh_handle_cache_[key] = alloc.handle;
    }
    return alloc.handle;
  };

  Model::LoadOptions load_options;
  load_options.global_scale = global_scale;
  auto result = Loader::Load<VertexData::ModelVertex, MeshHandle, ModelSurfaceMaterial, TextureHandle>(
    path, texture_cb, material_cb, mesh_cb, load_options);

  if (!result.success) {
    Logger::LogFormat(
      LogLevel::Error, LogCategory::Game, Logger::Here(), "[AssetManager] Failed to load model: {} - {}", path, result.error_message);
    return nullptr;
  }

  auto model_data = std::make_shared<ModelData>();
  model_data->path = path;
  model_data->surface_materials = std::move(result.surface_material_handles);
  model_data->root_node = std::move(result.root_node);
  model_data->min_y = (global_min_y < (std::numeric_limits<float>::max)()) ? global_min_y : 0.0f;
  if (bounds_min.x <= bounds_max.x) {
    model_data->bounds = Math::AABB(bounds_min, bounds_max);
  }

  model_data->texture_handles_ = std::move(result.texture_handles);

  auto resolve_texture = [&](const std::vector<std::optional<uint32_t>>& indices, uint32_t mat_idx) -> TextureHandle {
    if (mat_idx < indices.size()) {
      auto tex_idx = indices[mat_idx];
      if (tex_idx.has_value() && *tex_idx < model_data->texture_handles_.size()) {
        return model_data->texture_handles_[*tex_idx];
      }
    }
    return TextureHandle::Invalid();
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
      impl_->mesh_service->Free(entry.mesh_handle);
    }
  }
}

bool AssetManager::LoadFont(Font::FontFamily family, const std::string& fnt_path, const std::string& texture_path) {
  if (!impl_->font_service) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Game, Logger::Here(), "[AssetManager] Font service not initialized");
    return false;
  }

  return impl_->font_service->LoadFontVariant(family, fnt_path, texture_path);
}

TextMeshHandle AssetManager::CreateTextMesh(
  const std::wstring& text, Font::FontFamily family, float pixel_size, const Text::TextLayoutProps& layout_props) {
  if (!impl_->font_service) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Game, Logger::Here(), "[AssetManager] Font service not initialized");
    return TextMeshHandle();
  }

  Font::TextLayoutData layout_data;
  if (!impl_->font_service->CreateTextLayout(text, family, pixel_size, layout_props, layout_data)) {
    Logger::LogFormat(LogLevel::Error, LogCategory::Game, Logger::Here(), "[AssetManager] Failed to create text layout");
    return TextMeshHandle();
  }

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

  return TextMeshHandle(std::move(glyphs), layout_data.width, layout_data.height, layout_data.font_texture);
}
