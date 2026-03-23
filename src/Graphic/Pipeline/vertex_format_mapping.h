/**
 * @file vertex_format_mapping.h
 * @brief Maps Framework VertexFormat/TopologyType to DX12 types.
 */
#pragma once

#include <d3d12.h>

#include <span>

#include "Framework/Shader/vertex_format.h"
#include "vertex_types.h"

namespace Graphics {

inline std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayoutForFormat(VertexFormat format) {
  switch (format) {
    case VertexFormat::Sprite:
      return Vertex::SpriteVertex::GetInputLayout();
    case VertexFormat::Basic3D:
      return Vertex::Basic3DVertex::GetInputLayout();
    case VertexFormat::Model:
      return Vertex::ModelVertex::GetInputLayout();
    case VertexFormat::Line:
      return Vertex::LineVertex::GetInputLayout();
    default:
      return {};
  }
}

inline D3D12_PRIMITIVE_TOPOLOGY_TYPE ToD3D12Topology(TopologyType type) {
  switch (type) {
    case TopologyType::Triangle:
      return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    case TopologyType::Line:
      return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
    case TopologyType::Point:
      return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
    case TopologyType::Patch:
      return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
    default:
      return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
  }
}

}  // namespace Graphics
