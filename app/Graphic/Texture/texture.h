#pragma once

#include <d3d12.h>

#include <cstdint>

#include "Core/types.h"

struct Texture {
  ComPtr<ID3D12Resource> resource;
  uint32_t srv_index;

  uint32_t GetBindlessIndex() const {
    return srv_index;
  }
};
