/**
 * @file shader_types.h
 * @brief Shader identifier type shared across Game and Graphic layers.
 */
#pragma once

#include <cstdint>

namespace Graphics {

/// Shader identifier type - each shader struct defines its own unique ID
using ShaderId = uint32_t;

}  // namespace Graphics
