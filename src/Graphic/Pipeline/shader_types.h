/**
 * @file shader_types.h
 * @brief Core shader type definitions for the graphics pipeline.
 *
 * @note ShaderId is defined in Framework/Render/shader_types.h (shared across layers).
 *       This file adds Graphic-specific types: RSPreset, ShaderRenderHints.
 *       For shader definitions, see shader_descriptors.h.
 */
#pragma once
#include <d3d12.h>

#include "Framework/Render/shader_types.h"

namespace Graphics {

/// Root Signature Preset - determines which root signature to use
enum class RSPreset : uint8_t {
  Standard,  ///< FrameCB + ObjectCB + MaterialData + SRVs + Samplers

  Count
};

struct ShaderRenderHints {
  D3D12_PRIMITIVE_TOPOLOGY_TYPE topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
};

}  // namespace Graphics
