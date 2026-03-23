/**
 * @file shader_types.h
 * @brief Core shader type definitions for the graphics pipeline.
 *
 * @note ShaderId is defined in Framework/Shader/shader_id.h (global type).
 *       This file adds Graphic-specific types: RSPreset, ShaderRenderHints.
 */
#pragma once
#include <d3d12.h>

#include <cstdint>

#include "Framework/Shader/shader_id.h"

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
