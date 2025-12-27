#include "asset_manager.h"

#include "Graphic/Texture/texture_manager.h"
#include "Graphic/graphic.h"

class AssetManager::Impl {
 public:
  Graphic* graphic = nullptr;
  TextureManager* texture_manager = nullptr;
  // Todo: ModelManager* model_manager = nullptr;
  // Todo: AudioManager* audio_manager = nullptr;
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
  return true;
}

void AssetManager::Shutdown() {
  impl_.reset();
}

AssetHandle<Texture> AssetManager::LoadTexture(const std::string& path) {
  auto texture = impl_->texture_manager->LoadTexture(path);
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
