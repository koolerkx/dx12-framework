#include "texture_service.h"

#include "Texture/texture.h"
#include "Texture/texture_manager.h"

TextureService::TextureService(TextureManager& texture_manager) : texture_manager_(texture_manager) {
}

TextureHandle TextureService::LoadTextureSRGB(const std::string& path) {
  auto texture = texture_manager_.LoadTextureSRGB(path);
  return texture ? TextureHandle{texture->GetBindlessIndex()} : TextureHandle::Invalid();
}

TextureHandle TextureService::LoadTextureLinear(const std::string& path) {
  auto texture = texture_manager_.LoadTextureLinear(path);
  return texture ? TextureHandle{texture->GetBindlessIndex()} : TextureHandle::Invalid();
}

TextureHandle TextureService::LoadCubemap(const std::string& path) {
  auto texture = texture_manager_.LoadCubemapFromCrossHDR(path);
  return texture ? TextureHandle{texture->GetBindlessIndex()} : TextureHandle::Invalid();
}

std::vector<TextureHandle> TextureService::LoadTextures(const std::vector<std::string>& paths) {
  auto textures = texture_manager_.LoadTextures(paths);
  std::vector<TextureHandle> handles;
  handles.reserve(textures.size());
  for (const auto& tex : textures) {
    handles.emplace_back(tex ? TextureHandle{tex->GetBindlessIndex()} : TextureHandle::Invalid());
  }
  return handles;
}

TextureHandle TextureService::LoadTextureFromMemory(const std::string& cache_key, const uint8_t* data, size_t size, bool is_color) {
  auto texture = texture_manager_.LoadTextureFromMemory(cache_key, data, size, is_color);
  return texture ? TextureHandle{texture->GetBindlessIndex()} : TextureHandle::Invalid();
}

TextureHandle TextureService::LoadTextureFromRawPixels(
  const std::string& cache_key, const uint8_t* pixels, uint32_t w, uint32_t h, bool is_color) {
  auto texture = texture_manager_.LoadTextureFromRawPixels(cache_key, pixels, w, h, is_color);
  return texture ? TextureHandle{texture->GetBindlessIndex()} : TextureHandle::Invalid();
}

void TextureService::ProcessDeferredFrees(uint64_t completed_fence_value) {
  texture_manager_.ProcessDeferredFrees(completed_fence_value);
}

void TextureService::CleanUploadBuffers() {
  texture_manager_.CleanUploadBuffers();
}
