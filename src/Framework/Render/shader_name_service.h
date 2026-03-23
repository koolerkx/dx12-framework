/**
 * @file shader_name_service.h
 * @brief Interface for shader name ↔ ID lookup, used by Game layer for serialization.
 */
#pragma once

#include <optional>
#include <string_view>

#include "Framework/Shader/shader_id.h"

class IShaderNameService {
 public:
  virtual ~IShaderNameService() = default;
  virtual std::string_view GetName(ShaderId shader_id) const = 0;
  virtual std::optional<ShaderId> FindIdByName(std::string_view name) const = 0;
};
