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
#include <optional>
#include <span>
#include <string_view>
#include <unordered_map>

#include "Framework/Render/render_settings.h"
#include "shader_descriptors.h"
#include "shader_types.h"
#include "vertex_format_mapping.h"

namespace ShaderRegistry {

using Graphics::RSPreset;
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
  bool supports_structured_instancing;
  Rendering::RenderSettings default_settings;
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

template <typename T>
constexpr bool GetStructuredInstancing() {
  if constexpr (requires { T::STRUCTURED_INSTANCING; }) {
    return T::STRUCTURED_INSTANCING;
  } else {
    return false;
  }
}

template <typename ShaderType>
Rendering::RenderSettings GetDefaultSettings() {
  if constexpr (requires { ShaderType::DefaultRenderSettings(); }) {
    return ShaderType::DefaultRenderSettings();
  } else {
    return Rendering::RenderSettings::Opaque();
  }
}

template <typename T>
constexpr std::wstring_view GetVSPath() {
  if constexpr (requires { typename T::VertexShader; })
    return T::VertexShader::PATH;
  else
    return T::VS_PATH;
}

template <typename T>
constexpr std::wstring_view GetPSPath() {
  if constexpr (requires { typename T::PixelShader; })
    return T::PixelShader::PATH;
  else
    return T::PS_PATH;
}

template <typename ShaderType>
ShaderMetadata MakeMetadata() {
  std::span<const D3D12_INPUT_ELEMENT_DESC> layout;
  ShaderRenderHints hints = {};

  if constexpr (requires { typename ShaderType::VertexShader; }) {
    layout = Graphics::GetInputLayoutForFormat(ShaderType::VertexShader::VERTEX_FORMAT);
    if constexpr (requires { ShaderType::VertexShader::TOPOLOGY; })
      hints.topology = Graphics::ToD3D12Topology(ShaderType::VertexShader::TOPOLOGY);
  } else {
    layout = ShaderType::GetInputLayout();
    hints = ShaderType::HINTS;
  }

  RSPreset rs = RSPreset::Standard;
  if constexpr (requires { ShaderType::RS_PRESET; }) rs = ShaderType::RS_PRESET;

  return {
    ShaderType::ID,
    rs,
    ShaderType::NAME,
    GetVSPath<ShaderType>(),
    GetPSPath<ShaderType>(),
    layout,
    hints,
    HasInstanceElements(layout),
    GetStructuredInstancing<ShaderType>(),
    GetDefaultSettings<ShaderType>(),
  };
}

template <typename Tuple, size_t... Is>
auto MakeMetadataArray(std::index_sequence<Is...>) {
  return std::array<ShaderMetadata, sizeof...(Is)>{MakeMetadata<std::tuple_element_t<Is, Tuple>>()...};
}

inline const auto METADATA_TABLE = MakeMetadataArray<Graphics::AllShaders>(std::make_index_sequence<Graphics::SHADER_COUNT>{});

inline std::unordered_map<ShaderId, ShaderMetadata>& GetDynamicCache() {
  static std::unordered_map<ShaderId, ShaderMetadata> cache;
  return cache;
}

}  // namespace detail

inline void RegisterDynamic(const ShaderMetadata& metadata) {
  detail::GetDynamicCache().try_emplace(metadata.id, metadata);
}

inline const ShaderMetadata& GetMetadata(ShaderId id) {
  for (const auto& meta : detail::METADATA_TABLE) {
    if (meta.id == id) {
      return meta;
    }
  }
  auto& cache = detail::GetDynamicCache();
  auto it = cache.find(id);
  if (it != cache.end()) {
    return it->second;
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

inline std::optional<ShaderId> FindIdByName(std::string_view name) {
  for (const auto& meta : detail::METADATA_TABLE) {
    if (meta.name == name) return meta.id;
  }
  for (const auto& [_, meta] : detail::GetDynamicCache()) {
    if (meta.name == name) return meta.id;
  }
  return std::nullopt;
}

inline size_t GetShaderCount() {
  return Graphics::SHADER_COUNT + detail::GetDynamicCache().size();
}

inline const auto& GetEngineMetadata() {
  return detail::METADATA_TABLE;
}

inline const auto& GetDynamicMetadata() {
  return detail::GetDynamicCache();
}

}  // namespace ShaderRegistry
