#pragma once
#include <memory>
#include <string>
#include <vector>

#include "asset_handle.h"

class Graphic;
struct Texture;
struct Model;
struct AudioClip;

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

  // Cleanup per frame (called by engine, not by user)
  void ProcessDeferredCleanup(uint64_t completed_fence_value);
  void ClearUploadBuffers();

 private:
  class Impl;
  std::unique_ptr<Impl> impl_;
};
