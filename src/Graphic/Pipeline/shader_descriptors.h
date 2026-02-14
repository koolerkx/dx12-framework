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

struct SpriteInstancedShader {
  static constexpr ShaderId ID = 1;
  using VertexType = Vertex::SpriteInstanced;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::string_view NAME = "SpriteInstanced";
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/sprite_instanced.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/sprite_instanced.ps.cso";
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
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/fullscreen.vs.cso";
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
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/fullscreen.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/depthview.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }
};

struct PostProcessBlitShader {
  static constexpr ShaderId ID = 9;
  using VertexType = Vertex::Empty;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::string_view NAME = "Blit";
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/fullscreen.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/blit.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }
};

struct ShadowDepthShader {
  static constexpr ShaderId ID = 10;
  using VertexType = Vertex::ShadowVertex;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::string_view NAME = "ShadowDepth";
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/shadow_depth.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/shadow_depth.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }
};

struct PBRShader {
  static constexpr ShaderId ID = 11;
  using VertexType = Vertex::ModelVertex;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::string_view NAME = "PBR";
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/pbr.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/pbr.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }
};

struct PostProcessBloomDownsampleShader {
  static constexpr ShaderId ID = 12;
  using VertexType = Vertex::Empty;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::string_view NAME = "BloomDownsample";
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/fullscreen.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/bloom_downsample.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }
};

struct PostProcessBloomUpsampleShader {
  static constexpr ShaderId ID = 13;
  using VertexType = Vertex::Empty;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::string_view NAME = "BloomUpsample";
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/fullscreen.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/bloom_upsample.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }
};

struct DepthNormalShader {
  static constexpr ShaderId ID = 14;
  using VertexType = Vertex::DepthNormalVertex;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::string_view NAME = "DepthNormal";
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/depth_normal.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/depth_normal.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }
};

struct PostProcessNormalViewShader {
  static constexpr ShaderId ID = 15;
  using VertexType = Vertex::Empty;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::string_view NAME = "NormalView";
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/fullscreen.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/normal_view.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }
};

struct PostProcessLinearDepthViewShader {
  static constexpr ShaderId ID = 16;
  using VertexType = Vertex::Empty;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::string_view NAME = "LinearDepthView";
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/fullscreen.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/linear_depth_view.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }
};

// Shader Registration
using AllShaders = std::tuple<SpriteShader,
  SpriteInstancedShader,
  Basic3DShader,
  DebugLineShader,
  PostProcessToneMapShader,
  SkyboxShader,
  PostProcessDepthViewShader,
  PostProcessBlitShader,
  ShadowDepthShader,
  PBRShader,
  PostProcessBloomDownsampleShader,
  PostProcessBloomUpsampleShader,
  DepthNormalShader,
  PostProcessNormalViewShader,
  PostProcessLinearDepthViewShader>;

[[maybe_unused]] constexpr size_t SHADER_COUNT = std::tuple_size_v<AllShaders>;

}  // namespace Graphics
