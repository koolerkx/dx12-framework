#pragma once

#include <d3d12.h>

#include <cstdint>
#include <string>

#include "Core/types.h"

struct Texture {
  std::wstring source_path;

  uint32_t GetBindlessIndex() const {
    return srv_index_;
  }

  ID3D12Resource* GetResource() const {
    return resource_.Get();
  }

  Texture() = default;
  Texture(const Texture&) = delete;
  Texture& operator=(const Texture&) = delete;
  Texture(Texture&&) = default;
  Texture& operator=(Texture&&) = default;

  ~Texture() = default;

 private:
  friend class TextureManager;

  ComPtr<ID3D12Resource> resource_;
  uint32_t srv_index_ = UINT32_MAX;
};
