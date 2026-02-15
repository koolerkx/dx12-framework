/**
 * @file shader_types.h
 * @brief Core shader type definitions for the graphics pipeline.
 *
 * @note This file contains fundamental types used throughout the shader system.
 *       For shader definitions, see shader_descriptors.h.
 *
 * @code
 * // Using ShaderId
 * Graphics::ShaderId id = Graphics::DebugLineShader::ID;
 *
 * // Using RSPreset
 * ID3D12RootSignature* rs = shader_mgr.GetRootSignature(Graphics::RSPreset::Standard);
 * @endcode
 */
#pragma once
#include <cstdint>

namespace Graphics {

/// Shader identifier type - each shader struct defines its own unique ID
using ShaderId = uint32_t;

/// Root Signature Preset - determines which root signature to use
enum class RSPreset : uint8_t {
  Standard,  ///< FrameCB + ObjectCB + MaterialData + SRVs + Samplers

  Count
};

}  // namespace Graphics
