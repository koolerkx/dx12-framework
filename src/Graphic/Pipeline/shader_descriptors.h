/**
 * @file shader_descriptors.h
 * @brief shader definitions
 *
 * @note To add a new shader:
 *       1. Define a new shader struct
 *       2. Add the struct to AllShaders tuple
 *
 * @warning Each shader must have a unique NAME. IDs are hash-derived and collisions will cause undefined behavior.
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

#include "Framework/Shader/default_shaders.h"
#include "Framework/Shader/shader_id.h"
#include "shader_types.h"
#include "vertex_types.h"

namespace Graphics {

// Post-Process Shaders
struct PostProcessToneMapShader {
  static constexpr std::string_view NAME = "ToneMap";
  static constexpr ShaderId ID = HashShaderName(NAME);
  using VertexType = Vertex::Empty;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/fullscreen.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/tonemap.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }
};

struct SkyboxShader {
  static constexpr std::string_view NAME = "Skybox";
  static constexpr ShaderId ID = HashShaderName(NAME);
  using VertexType = Vertex::SkyboxVertex;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/skybox.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/skybox.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }
};

struct PostProcessDepthViewShader {
  static constexpr std::string_view NAME = "DepthView";
  static constexpr ShaderId ID = HashShaderName(NAME);
  using VertexType = Vertex::Empty;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/fullscreen.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/depthview.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }
};

struct PostProcessBlitShader {
  static constexpr std::string_view NAME = "Blit";
  static constexpr ShaderId ID = HashShaderName(NAME);
  using VertexType = Vertex::Empty;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/fullscreen.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/blit.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }
};

struct ShadowDepthShader {
  static constexpr std::string_view NAME = "ShadowDepth";
  static constexpr ShaderId ID = HashShaderName(NAME);
  using VertexType = Vertex::ShadowVertex;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/shadow_depth.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/shadow_depth.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }
};

struct PostProcessBloomDownsampleShader {
  static constexpr std::string_view NAME = "BloomDownsample";
  static constexpr ShaderId ID = HashShaderName(NAME);
  using VertexType = Vertex::Empty;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/fullscreen.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/bloom_downsample.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }
};

struct PostProcessBloomUpsampleShader {
  static constexpr std::string_view NAME = "BloomUpsample";
  static constexpr ShaderId ID = HashShaderName(NAME);
  using VertexType = Vertex::Empty;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/fullscreen.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/bloom_upsample.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }
};

struct DepthNormalShader {
  static constexpr std::string_view NAME = "DepthNormal";
  static constexpr ShaderId ID = HashShaderName(NAME);
  using VertexType = Vertex::DepthNormalVertex;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/depth_normal.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/depth_normal.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }
};

struct PostProcessNormalViewShader {
  static constexpr std::string_view NAME = "NormalView";
  static constexpr ShaderId ID = HashShaderName(NAME);
  using VertexType = Vertex::Empty;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/fullscreen.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/normal_view.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }
};

struct PostProcessLinearDepthViewShader {
  static constexpr std::string_view NAME = "LinearDepthView";
  static constexpr ShaderId ID = HashShaderName(NAME);
  using VertexType = Vertex::Empty;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/fullscreen.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/linear_depth_view.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }
};

struct PostProcessSSAOShader {
  static constexpr std::string_view NAME = "SSAO";
  static constexpr ShaderId ID = HashShaderName(NAME);
  using VertexType = Vertex::Empty;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/fullscreen.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/ssao.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }
};

struct PostProcessSSAOBlurShader {
  static constexpr std::string_view NAME = "SSAOBlur";
  static constexpr ShaderId ID = HashShaderName(NAME);
  using VertexType = Vertex::Empty;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/fullscreen.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/ssao_blur.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }
};

struct PostProcessSMAAEdgeShader {
  static constexpr std::string_view NAME = "SMAAEdge";
  static constexpr ShaderId ID = HashShaderName(NAME);
  using VertexType = Vertex::Empty;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/fullscreen.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/smaa_edge.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }
};

struct PostProcessSMAABlendWeightShader {
  static constexpr std::string_view NAME = "SMAABlendWeight";
  static constexpr ShaderId ID = HashShaderName(NAME);
  using VertexType = Vertex::Empty;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/fullscreen.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/smaa_blend_weight.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }
};

struct PostProcessSMAANeighborhoodShader {
  static constexpr std::string_view NAME = "SMAANeighborhood";
  static constexpr ShaderId ID = HashShaderName(NAME);
  using VertexType = Vertex::Empty;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/fullscreen.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/smaa_neighborhood.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }
};

struct PostProcessOutlineShader {
  static constexpr std::string_view NAME = "Outline";
  static constexpr ShaderId ID = HashShaderName(NAME);
  using VertexType = Vertex::Empty;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/fullscreen.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/outline.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }
};

struct PostProcessVignetteShader {
  static constexpr std::string_view NAME = "Vignette";
  static constexpr ShaderId ID = HashShaderName(NAME);
  using VertexType = Vertex::Empty;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/fullscreen.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/vignette.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }
};

struct PostProcessFogShader {
  static constexpr std::string_view NAME = "Fog";
  static constexpr ShaderId ID = HashShaderName(NAME);
  using VertexType = Vertex::Empty;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/fullscreen.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/fog.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }
};

struct PostProcessChromaticAberrationShader {
  static constexpr std::string_view NAME = "ChromaticAberration";
  static constexpr ShaderId ID = HashShaderName(NAME);
  using VertexType = Vertex::Empty;

  static constexpr RSPreset RS_PRESET = RSPreset::Standard;
  static constexpr std::wstring_view VS_PATH = L"Content/shaders/fullscreen.vs.cso";
  static constexpr std::wstring_view PS_PATH = L"Content/shaders/chromatic_aberration.ps.cso";
  static constexpr ShaderRenderHints HINTS = {};

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return VertexType::GetInputLayout();
  }
};

// Shader Registration
using EngineShaders = std::tuple<PostProcessToneMapShader,
  SkyboxShader,
  PostProcessDepthViewShader,
  PostProcessBlitShader,
  ShadowDepthShader,
  PostProcessBloomDownsampleShader,
  PostProcessBloomUpsampleShader,
  DepthNormalShader,
  PostProcessNormalViewShader,
  PostProcessLinearDepthViewShader,
  PostProcessSSAOShader,
  PostProcessSSAOBlurShader,
  PostProcessSMAAEdgeShader,
  PostProcessSMAABlendWeightShader,
  PostProcessSMAANeighborhoodShader,
  PostProcessOutlineShader,
  PostProcessVignetteShader,
  PostProcessFogShader,
  PostProcessChromaticAberrationShader>;

using AllShaders = decltype(std::tuple_cat(std::declval<Shaders::DefaultShaders>(), std::declval<EngineShaders>()));

[[maybe_unused]] constexpr size_t SHADER_COUNT = std::tuple_size_v<AllShaders>;

}  // namespace Graphics
