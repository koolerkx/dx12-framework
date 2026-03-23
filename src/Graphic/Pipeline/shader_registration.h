/**
 * @file shader_registration.h
 * @brief Implementation of IShaderRegistration for runtime shader registration.
 */
#pragma once

#include "Framework/Shader/shader_registration.h"
#include "shader_registry.h"
#include "vertex_format_mapping.h"

class ShaderRegistration final : public IShaderRegistration {
 public:
  void RegisterShader(const ShaderDescriptor& descriptor) override {
    auto layout = Graphics::GetInputLayoutForFormat(descriptor.vertex_format);

    ShaderRegistry::ShaderMetadata metadata{
      .id = descriptor.id,
      .rs_preset = Graphics::RSPreset::Standard,
      .name = descriptor.name,
      .vs_path = descriptor.vs_path,
      .ps_path = descriptor.ps_path,
      .input_layout = layout,
      .render_hints = {.topology = Graphics::ToD3D12Topology(descriptor.topology)},
      .supports_instancing = false,
      .supports_structured_instancing = descriptor.structured_instancing,
      .default_settings = descriptor.default_settings,
    };

    for (const auto& elem : layout) {
      if (elem.InputSlotClass == D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA) {
        metadata.supports_instancing = true;
        break;
      }
    }

    ShaderRegistry::RegisterDynamic(metadata);
  }
};
