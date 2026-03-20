/**
 * @file shader_types.h
 * @brief Core shader type definitions for the graphics pipeline.
 */
#pragma once
#include <d3d12.h>

#include "Framework/Render/shader_types.h"

namespace Graphics {

enum class RSPreset : uint8_t {
  Standard,

  Count
};

struct ShaderRenderHints {
  D3D12_PRIMITIVE_TOPOLOGY_TYPE topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
};

}  // namespace Graphics
