#pragma once
#include <array>
#include <cstring>

#include "Framework/Math/Math.h"
#include "Framework/Render/render_handles.h"
#include "Framework/Render/render_settings.h"
#include "Framework/Render/render_types.h"
#include "Framework/Shader/shader_id.h"

struct CustomShaderData {
  std::array<float, 20> data{};
  bool active = false;
};

struct RenderRequest {
  MeshHandle mesh;
  MaterialHandle material;
  ShaderId shader_id = 0;
  Rendering::RenderSettings render_settings;

  Math::Matrix4 world_matrix;
  Math::Vector4 color{1, 1, 1, 1};
  Math::Vector2 uv_offset{0, 0};
  Math::Vector2 uv_scale{1, 1};

  RenderLayer layer = RenderLayer::Opaque;
  RenderTagMask tags = 0;
  float depth = 0.0f;

  CustomShaderData custom_data;

  template <typename ShaderType>
  RenderRequest& SetShader() {
    shader_id = ShaderType::ID;
    if constexpr (requires { ShaderType::RENDER_LAYER; }) layer = ShaderType::RENDER_LAYER;
    if constexpr (requires { ShaderType::RENDER_TAGS; }) tags = static_cast<RenderTagMask>(ShaderType::RENDER_TAGS);
    if constexpr (requires { ShaderType::DefaultRenderSettings(); }) render_settings = ShaderType::DefaultRenderSettings();
    return *this;
  }

  template <typename ShaderType>
    requires requires { typename ShaderType::Params; }
  RenderRequest& SetShader(const typename ShaderType::Params& params) {
    SetShader<ShaderType>();
    custom_data.active = true;
    static_assert(sizeof(params) <= sizeof(custom_data.data));
    std::memcpy(custom_data.data.data(), &params, sizeof(params));
    return *this;
  }

  RenderRequest& SetShader(ShaderId id) {
    shader_id = id;
    return *this;
  }
};

struct InstancedRenderRequest {
  MeshHandle mesh;
  MaterialHandle material;
  ShaderId shader_id = 0;
  Rendering::RenderSettings render_settings;

  Math::Vector4 color{1, 1, 1, 1};

  RenderLayer layer = RenderLayer::Opaque;
  RenderTagMask tags = 0;
  float depth = 0.0f;

  CustomShaderData custom_data;

  template <typename ShaderType>
  InstancedRenderRequest& SetShader() {
    shader_id = ShaderType::ID;
    if constexpr (requires { ShaderType::RENDER_LAYER; }) layer = ShaderType::RENDER_LAYER;
    if constexpr (requires { ShaderType::RENDER_TAGS; }) tags = static_cast<RenderTagMask>(ShaderType::RENDER_TAGS);
    if constexpr (requires { ShaderType::DefaultRenderSettings(); }) render_settings = ShaderType::DefaultRenderSettings();
    return *this;
  }

  InstancedRenderRequest& SetShader(ShaderId id) {
    shader_id = id;
    return *this;
  }
};
