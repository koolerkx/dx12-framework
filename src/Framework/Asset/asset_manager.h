/**
 * @file asset_manager.h
 * @brief Asset loading facade with model-level caching.
 *
 * Caches complete ModelData (mesh pointers, textures, materials) keyed by
 * path + scale, so repeated LoadModel calls skip Assimp parsing entirely.
 */
#pragma once
#include <array>
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "Framework/Asset/font_service.h"
#include "Framework/Asset/mesh_service.h"
#include "Framework/Asset/model_data.h"
#include "Framework/Asset/text_mesh_handle.h"
#include "Framework/Asset/texture_service.h"
#include "Framework/Math/Math.h"
#include "Framework/Render/mesh_data.h"
#include "Framework/Render/render_handles.h"

namespace Font {
enum class FontFamily : uint16_t;
struct TextLayoutData;
}  // namespace Font

namespace Text {
struct TextLayoutProps;
}

// Quad for 3D, Rect for 2D
enum class DefaultMesh { Quad, Cube, Plane, Rect, Sphere, Cylinder, RoundedRect };

struct AssetServices {
  ITextureService& texture;
  IMeshService& mesh;
  IFontService& font;
};

class AssetManager {
 public:
  AssetManager();
  ~AssetManager();
  AssetManager(const AssetManager&) = delete;
  AssetManager& operator=(const AssetManager&) = delete;
  AssetManager(AssetManager&&) noexcept;
  AssetManager& operator=(AssetManager&&) noexcept;

  bool Initialize(const AssetServices& services);
  void Shutdown();

  // Texture management
  TextureHandle LoadTexture(const std::string& path);
  TextureHandle LoadTextureLinear(const std::string& path);
  TextureHandle LoadCubemap(const std::string& path);
  std::vector<TextureHandle> LoadTextures(const std::vector<std::string>& paths);

  struct ModelLoadOptions {
    float global_scale = 1.0f;
    bool flatten_node_transforms = false;
  };
  std::shared_ptr<ModelData> LoadModel(const std::string& path, float global_scale = 1.0f);
  std::shared_ptr<ModelData> LoadModel(const std::string& path, const ModelLoadOptions& options);

  void CreateTextureFromPixels(const std::string& cache_key, const uint8_t* pixels, uint32_t width, uint32_t height);

  using CubeCornerColors = std::array<Math::Vector4, 8>;
  MeshHandle CreateCubeMesh(const std::string& key, const CubeCornerColors& corner_colors);
  MeshHandle GetDefaultMeshHandle(DefaultMesh type) const;
  MeshHandle GetOrCreateMeshHandle(const std::string& key, const MeshData& data);

  bool LoadFont(Font::FontFamily family, const std::string& fnt_path, const std::string& texture_path);

  TextMeshHandle CreateTextMesh(
    const std::wstring& text, Font::FontFamily family, float pixel_size, const Text::TextLayoutProps& layout_props);

  TextureHandle GetDefaultWhiteTexture() const {
    return default_white_texture_;
  }

  // Cleanup per frame (called by engine, not by user)
  void ProcessDeferredCleanup(uint64_t completed_fence_value);
  void ClearUploadBuffers();

 private:
  void UploadDefaultMeshesToPool();
  void FreeMeshHandlesForModel(const ModelData& model_data);

  std::unordered_map<DefaultMesh, MeshHandle> default_mesh_handles_;
  std::unordered_map<std::string, std::shared_ptr<ModelData>> model_cache_;
  std::unordered_map<std::string, MeshHandle> mesh_handle_cache_;
  TextureHandle default_white_texture_;

  class Impl;
  std::unique_ptr<Impl> impl_;
};
