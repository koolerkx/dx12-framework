/**
 * @file shader_descriptor.h
 * @brief Runtime shader descriptor for dynamic registration.
 */
#pragma once

#include <string_view>

#include "Framework/Render/render_settings.h"
#include "shader_id.h"
#include "vertex_format.h"

struct ShaderDescriptor {
  ShaderId id;
  std::string_view name;
  std::wstring_view vs_path;
  std::wstring_view ps_path;
  VertexFormat vertex_format;
  TopologyType topology = TopologyType::Triangle;
  bool structured_instancing = false;
  Rendering::RenderSettings default_settings = Rendering::RenderSettings::Opaque();
};

template <typename T>
constexpr ShaderDescriptor ToDescriptor() {
  using VS = typename T::VertexShader;
  using PS = typename T::PixelShader;
  ShaderDescriptor desc{};
  desc.id = T::ID;
  desc.name = T::NAME;
  desc.vs_path = VS::PATH;
  desc.ps_path = PS::PATH;
  desc.vertex_format = VS::VERTEX_FORMAT;
  if constexpr (requires { VS::TOPOLOGY; }) desc.topology = VS::TOPOLOGY;
  if constexpr (requires { VS::STRUCTURED_INSTANCING; }) desc.structured_instancing = VS::STRUCTURED_INSTANCING;
  if constexpr (requires { T::DefaultRenderSettings(); }) desc.default_settings = T::DefaultRenderSettings();
  return desc;
}
