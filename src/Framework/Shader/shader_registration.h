/**
 * @file shader_registration.h
 * @brief Interface for runtime shader registration.
 */
#pragma once

#include "shader_descriptor.h"

class IShaderRegistration {
 public:
  virtual ~IShaderRegistration() = default;
  virtual void RegisterShader(const ShaderDescriptor& descriptor) = 0;
};
