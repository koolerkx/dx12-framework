#pragma once
#include <cstdint>

struct TextureHandle {
  uint32_t srv_index = UINT32_MAX;

  bool IsValid() const {
    return srv_index != UINT32_MAX;
  }

  uint32_t GetBindlessIndex() const {
    return srv_index;
  }

  static TextureHandle Invalid() {
    return {};
  }
};
