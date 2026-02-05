/**
 * @file shader_registry.h
 * @brief Shader metadata table and lookup API
 *
 * @code
 * // Get metadata by shader type
 * const auto& meta = ShaderRegistry::GetMetadata<Graphics::DebugLineShader>();
 *
 * // Get metadata by runtime ID
 * const auto& meta = ShaderRegistry::GetMetadata(shader_id);
 *
 * // Access metadata fields
 * std::string_view name = meta.name;
 * auto layout = meta.input_layout;
 * auto topology = meta.render_hints.topology;
 *
 * // Iterate all shaders
 * for (const auto& m : ShaderRegistry::GetAllMetadata()) {
 *   Logger::Log("Shader: {}", m.name);
 * }
 * @endcode
 */

#pragma once
#include <d3d12.h>

#include <array>
#include <span>
#include <string_view>

#include "shader_descriptors.h"
#include "shader_types.h"

namespace ShaderRegistry {

using Graphics::RSPreset;
using Graphics::ShaderId;
using Graphics::ShaderRenderHints;

// ShaderMetadata - Runtime representation of shader properties
struct ShaderMetadata {
  ShaderId id;
  RSPreset rs_preset;
  std::string_view name;
  std::wstring_view vs_path;
  std::wstring_view ps_path;
  std::span<const D3D12_INPUT_ELEMENT_DESC> input_layout;
  ShaderRenderHints render_hints;
  bool supports_instancing;
};

namespace detail {

inline bool HasInstanceElements(std::span<const D3D12_INPUT_ELEMENT_DESC> layout) {
  for (const auto& elem : layout) {
    if (elem.InputSlotClass == D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA) {
      return true;
    }
  }
  return false;
}

template <typename ShaderType>
ShaderMetadata MakeMetadata() {
  auto layout = ShaderType::GetInputLayout();
  return {
    ShaderType::ID,
    ShaderType::RS_PRESET,
    ShaderType::NAME,
    ShaderType::VS_PATH,
    ShaderType::PS_PATH,
    layout,
    ShaderType::HINTS,
    HasInstanceElements(layout),
  };
}

template <typename Tuple, size_t... Is>
auto MakeMetadataArray(std::index_sequence<Is...>) {
  return std::array<ShaderMetadata, sizeof...(Is)>{MakeMetadata<std::tuple_element_t<Is, Tuple>>()...};
}

inline const auto METADATA_TABLE = MakeMetadataArray<Graphics::AllShaders>(std::make_index_sequence<Graphics::SHADER_COUNT>{});

}  // namespace detail

// Public API
inline const ShaderMetadata& GetMetadata(ShaderId id) {
  for (const auto& meta : detail::METADATA_TABLE) {
    if (meta.id == id) {
      return meta;
    }
  }
  return detail::METADATA_TABLE[0];
}

template <typename ShaderType>
inline const ShaderMetadata& GetMetadata() {
  return GetMetadata(ShaderType::ID);
}

inline std::string_view GetName(ShaderId id) {
  return GetMetadata(id).name;
}

inline RSPreset GetRSPreset(ShaderId id) {
  return GetMetadata(id).rs_preset;
}

inline constexpr size_t GetShaderCount() {
  return Graphics::SHADER_COUNT;
}

inline const auto& GetAllMetadata() {
  return detail::METADATA_TABLE;
}

}  // namespace ShaderRegistry
