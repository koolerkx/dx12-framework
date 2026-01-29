#pragma once

#include <d3d12.h>

#include <cstdint>
#include <string>

#include "Core/types.h"

struct Texture {
  ComPtr<ID3D12Resource> resource;
  uint32_t srv_index = UINT32_MAX;
  std::wstring source_path;

  uint32_t GetBindlessIndex() const {
    return srv_index;
  }

  Texture() = default;
  Texture(const Texture&) = delete;
  Texture& operator=(const Texture&) = delete;
  Texture(Texture&&) = default;
  Texture& operator=(Texture&&) = default;

  ~Texture() = default;
};
