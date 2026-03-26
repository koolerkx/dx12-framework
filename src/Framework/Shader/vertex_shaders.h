/**
 * @file vertex_shaders.h
 * @brief Reusable vertex shader types for shader composition.
 */
#pragma once

#include <string_view>

#include "vertex_format.h"

namespace VS {

struct Sprite {
  static constexpr std::wstring_view PATH = L"Content/shaders/sprite.vs.cso";
  static constexpr VertexFormat VERTEX_FORMAT = VertexFormat::Sprite;
};

struct Basic3D {
  static constexpr std::wstring_view PATH = L"Content/shaders/basic.vs.cso";
  static constexpr VertexFormat VERTEX_FORMAT = VertexFormat::Basic3D;
};

struct Model {
  static constexpr std::wstring_view PATH = L"Content/shaders/pbr.vs.cso";
  static constexpr VertexFormat VERTEX_FORMAT = VertexFormat::Model;
  static constexpr bool STRUCTURED_INSTANCING = true;
};

struct DebugLine {
  static constexpr std::wstring_view PATH = L"Content/shaders/debug_line_instanced.vs.cso";
  static constexpr VertexFormat VERTEX_FORMAT = VertexFormat::Line;
  static constexpr TopologyType TOPOLOGY = TopologyType::Line;
};

}  // namespace VS
