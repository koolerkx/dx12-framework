/**
 * @file shader_id.h
 * @brief Hash-based shader identifier type.
 */
#pragma once

#include <cstdint>
#include <string_view>

#include "Framework/Core/utils.h"

using ShaderId = uint32_t;

constexpr ShaderId HashShaderName(std::string_view name) {
  return utils::HashString(name);
}
