#pragma once
#include <memory>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

#include "Resource/mesh.h"
#include "asset_handle.h"
#include "model_data.h"
#include "text_mesh_handle.h"

class Graphic;
struct Texture;
struct AudioClip;

namespace Font {
enum class FontFamily : uint16_t;
struct TextLayoutData;
}  // namespace Font

namespace Text {
struct TextLayoutProps;
}

// Quad for 3D, Rect for 2D
enum class DefaultMesh { Quad, Cube, Plane, Rect, Sphere };

class AssetManager {
 public:
  AssetManager();
  ~AssetManager();
  AssetManager(const AssetManager&) = delete;
  AssetManager& operator=(const AssetManager&) = delete;
  AssetManager(AssetManager&&) noexcept;
  AssetManager& operator=(AssetManager&&) noexcept;

  bool Initialize(Graphic* graphic);
  void Shutdown();

  // Texture management
  AssetHandle<Texture> LoadTexture(const std::string& path);
  AssetHandle<Texture> LoadTextureLinear(const std::string& path);
  AssetHandle<Texture> LoadCubemap(const std::string& path);
  std::vector<AssetHandle<Texture>> LoadTextures(const std::vector<std::string>& paths);

  std::shared_ptr<ModelData> LoadModel(const std::string& path);

  // Todo: Audio management
  // AssetHandle<AudioClip> LoadAudio(const std::string& path);

  const Mesh* GetDefaultMesh(DefaultMesh type) const;
  std::optional<DefaultMesh> FindDefaultMeshType(const Mesh* mesh) const;

  bool LoadFont(Font::FontFamily family, const std::string& fnt_path, const std::string& texture_path);

  TextMeshHandle CreateTextMesh(
    const std::wstring& text, Font::FontFamily family, float pixel_size, const Text::TextLayoutProps& layout_props);

  Texture* GetDefaultWhiteTexture() const {
    return default_white_texture_.get();
  }

  Graphic* GetGraphicForDebugUseOnly() const;

  // Cleanup per frame (called by engine, not by user)
  void ProcessDeferredCleanup(uint64_t completed_fence_value);
  void ClearUploadBuffers();

 private:
  void CreateDefaultMeshes();
  std::unordered_map<DefaultMesh, const Mesh*> default_meshes_;
  std::unordered_map<std::string, std::shared_ptr<ModelData>> model_cache_;
  std::shared_ptr<Texture> default_white_texture_;

  class Impl;
  std::unique_ptr<Impl> impl_;
};
