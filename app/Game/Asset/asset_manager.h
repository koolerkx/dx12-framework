#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "Resource/mesh.h"
#include "asset_handle.h"

class Graphic;
struct Texture;
struct Model;
struct AudioClip;

enum class DefaultMesh { Quad, Cube, Plane };

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
  std::vector<AssetHandle<Texture>> LoadTextures(const std::vector<std::string>& paths);

  // Todo: Model management
  // AssetHandle<Model> LoadModel(const std::string& path);

  // Todo: Audio management
  // AssetHandle<AudioClip> LoadAudio(const std::string& path);

  const Mesh* GetDefaultMesh(DefaultMesh type) const;

  Graphic* GetGraphicForDebugUseOnly() const;

  // Cleanup per frame (called by engine, not by user)
  void ProcessDeferredCleanup(uint64_t completed_fence_value);
  void ClearUploadBuffers();

 private:
  void CreateDefaultMeshes();
  std::unordered_map<DefaultMesh, std::unique_ptr<Mesh>> owned_default_meshes_;
  std::unordered_map<DefaultMesh, const Mesh*> default_meshes_;

  class Impl;
  std::unique_ptr<Impl> impl_;
};
