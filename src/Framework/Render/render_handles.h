/**
 * @file render_handles.h
 * @brief Opaque handle types for render resources (mesh, material, texture).
 */
#pragma once

#include <cstdint>

#include "Framework/Handle/handle.h"

struct MeshHandleTag {};
using MeshHandle = Framework::Handle<MeshHandleTag>;

struct MaterialHandleTag {};
using MaterialHandle = Framework::Handle<MaterialHandleTag>;

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
