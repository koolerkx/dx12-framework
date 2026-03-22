/**
 * @file texture_service.h
 * @brief TextureService implementation delegating to TextureManager.
 */
#pragma once

#include "Framework/Asset/texture_service.h"

class TextureManager;

class TextureService : public ITextureService {
 public:
  explicit TextureService(TextureManager& texture_manager);

  TextureHandle LoadTextureSRGB(const std::string& path) override;
  TextureHandle LoadTextureLinear(const std::string& path) override;
  TextureHandle LoadCubemap(const std::string& path) override;
  std::vector<TextureHandle> LoadTextures(const std::vector<std::string>& paths) override;
  TextureHandle LoadTextureFromMemory(const std::string& cache_key, const uint8_t* data, size_t size, bool is_color) override;
  TextureHandle LoadTextureFromRawPixels(
    const std::string& cache_key, const uint8_t* pixels, uint32_t w, uint32_t h, bool is_color) override;
  void ProcessDeferredFrees(uint64_t completed_fence_value) override;
  void CleanUploadBuffers() override;

 private:
  TextureManager& texture_manager_;
};
