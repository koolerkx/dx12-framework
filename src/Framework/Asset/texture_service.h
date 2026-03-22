/**
 * @file texture_service.h
 * @brief Interface for texture loading operations, decoupled from GPU implementation.
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "Framework/Render/texture_handle.h"

class ITextureService {
 public:
  virtual ~ITextureService() = default;
  virtual TextureHandle LoadTextureSRGB(const std::string& path) = 0;
  virtual TextureHandle LoadTextureLinear(const std::string& path) = 0;
  virtual TextureHandle LoadCubemap(const std::string& path) = 0;
  virtual std::vector<TextureHandle> LoadTextures(const std::vector<std::string>& paths) = 0;
  virtual TextureHandle LoadTextureFromMemory(const std::string& cache_key, const uint8_t* data, size_t size, bool is_color) = 0;
  virtual TextureHandle LoadTextureFromRawPixels(
    const std::string& cache_key, const uint8_t* pixels, uint32_t w, uint32_t h, bool is_color) = 0;
  virtual void ProcessDeferredFrees(uint64_t completed_fence_value) = 0;
  virtual void CleanUploadBuffers() = 0;
};
