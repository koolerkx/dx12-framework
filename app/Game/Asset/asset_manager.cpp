#include "asset_manager.h"

#include <iostream>

#include "Graphic/Resource/Texture/texture_manager.h"
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
  CreateDefaultMeshes();
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

Graphic* AssetManager::GetGraphicForDebugUseOnly() const {
  return impl_.get()->graphic;
}

void AssetManager::CreateDefaultMeshes() {
  if (!impl_->graphic) return;

  // Get meshes from Graphic (it owns them)
  default_meshes_[DefaultMesh::Quad] = impl_->graphic->GetQuadMesh();
  default_meshes_[DefaultMesh::Cube] = impl_->graphic->GetCubeMesh();
}

const Mesh* AssetManager::GetDefaultMesh(DefaultMesh type) const {
  auto it = default_meshes_.find(type);
  if (it != default_meshes_.end()) {
    return it->second;
  }

  std::cerr << "[AssetManager] Default mesh not found: " << static_cast<int>(type) << std::endl;
  return nullptr;
}
