/**
 * @file shader_descriptors.h
 * @brief shader definitions
 *
 * @note To add a new shader:
 *       1. Define a new shader struct
 *       2. Add the struct to AllShaders tuple
 *
 * @warning Each shader must have a unique ID. Duplicate IDs will cause undefined behavior.
 *
 * @code
 * // Template API (recommended)
 * Material* mat = material_mgr.GetOrCreateMaterial<Graphics::DebugLineShader>(settings);
 *
 * // Runtime ID API
 * Graphics::ShaderId id = Graphics::DebugLineShader::ID;
 * Material* mat = material_mgr.GetOrCreateMaterial(id, settings);
 *
 * // Access shader properties
 * constexpr auto name = Graphics::DebugLineShader::NAME;  // "DebugLine"
 * auto layout = Graphics::DebugLineShader::GetInputLayout();
 * @endcode
 */
#pragma once
#include <d3d12.h>

#include <span>
#include <string_view>
#include <tuple>

#include "shader_types.h"
#include "vertex_types.h"

namespace Graphics {

// ShaderRenderHints - Data-driven PSO configuration
struct ShaderRenderHints {
  D3D12_PRIMITIVE_TOPOLOGY_TYPE topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
};

// Sprite Shaders
struct SpriteShader {
  static constexpr ShaderId ID = 0;
  using VertexType = Vertex::SpriteVertex;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::string_view NAME = "Sprite";
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/sprite.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/sprite.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }
};

struct SpriteInstancedUIShader {
  static constexpr ShaderId ID = 1;
  using VertexType = Vertex::SpriteInstanced;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::string_view NAME = "SpriteInstancedUI";
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/sprite_instanced_ui.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/sprite_instanced_ui.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }
};

struct SpriteInstancedWorldShader {
  static constexpr ShaderId ID = 2;
  using VertexType = Vertex::SpriteInstanced;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::string_view NAME = "SpriteInstancedWorld";
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/sprite_instanced_world.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/sprite_instanced_world.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }
};

struct SpriteInstancedWorldTransparentShader {
  static constexpr ShaderId ID = 3;
  using VertexType = Vertex::SpriteInstanced;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::string_view NAME = "SpriteInstancedWorldTransparent";
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/sprite_instanced_world_transparent.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/sprite_instanced_world_transparent.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }
};

// Mesh Shaders
struct Basic3DShader {
  static constexpr ShaderId ID = 4;
  using VertexType = Vertex::Basic3DVertex;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::string_view NAME = "Basic3D";
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/basic.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/basic.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }
};

struct DebugLineShader {
  static constexpr ShaderId ID = 5;
  using VertexType = Vertex::LineVertex;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::string_view NAME = "DebugLine";
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/debug_line.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/debug_line.ps.cso";
  static constexpr ShaderRenderHints HINTS = {.topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }
};

// Post-Process Shaders
struct PostProcessToneMapShader {
  static constexpr ShaderId ID = 6;
  using VertexType = Vertex::Empty;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::string_view NAME = "ToneMap";
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/tonemap.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/tonemap.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }
};

struct SkyboxShader {
  static constexpr ShaderId ID = 7;
  using VertexType = Vertex::SkyboxVertex;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::string_view NAME = "Skybox";
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/skybox.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/skybox.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }
};

struct PostProcessDepthViewShader {
  static constexpr ShaderId ID = 8;
  using VertexType = Vertex::Empty;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::string_view NAME = "DepthView";
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/depthview.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/depthview.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }
};

// Shader Registration
using AllShaders = std::tuple<SpriteShader,
  SpriteInstancedUIShader,
  SpriteInstancedWorldShader,
  SpriteInstancedWorldTransparentShader,
  Basic3DShader,
  DebugLineShader,
  PostProcessToneMapShader,
  SkyboxShader,
  PostProcessDepthViewShader>;

[[maybe_unused]] constexpr size_t SHADER_COUNT = std::tuple_size_v<AllShaders>;

}  // namespace Graphics
