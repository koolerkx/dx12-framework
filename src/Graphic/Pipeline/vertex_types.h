/**
 * @file vertex_types.h
 * @brief Vertex struct definitions with embedded D3D12 input layouts.
 *
 * @note Each vertex struct contains its own input layout definition,
 *       ensuring Single Source of Truth for CPU struct and GPU layout.
 *
 * @warning When modifying vertex structs, always update the corresponding
 *          INPUT_LAYOUT offsets and add static_assert for size verification.
 *
 * @code
 * // Using vertex types directly
 * std::vector<Graphics::Vertex::LineVertex> vertices;
 * vertices.push_back({.position = {0, 0, 0}, .color = {1, 0, 0, 1}});
 *
 * // Getting input layout for PSO creation
 * auto layout = Graphics::Vertex::PositionColor::GetInputLayout();
 * @endcode
 */
#pragma once
#include <d3d12.h>

#include <array>
#include <span>
#include <vector>

#include "Framework/Math/Math.h"

using Math::Matrix4;
using Math::Vector2;
using Math::Vector3;
using Math::Vector4;

namespace Graphics::Vertex {

struct PositionColor {
  Vector3 position;
  Vector4 color;

  static constexpr std::array INPUT_LAYOUT = {
    D3D12_INPUT_ELEMENT_DESC{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    D3D12_INPUT_ELEMENT_DESC{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
  };

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return {INPUT_LAYOUT.data(), INPUT_LAYOUT.size()};
  }
};
static_assert(sizeof(PositionColor) == 28);

struct PositionTexCoordColor {
  Vector3 position;
  Vector2 tex_coord;
  Vector4 color;

  static constexpr std::array INPUT_LAYOUT = {
    D3D12_INPUT_ELEMENT_DESC{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    D3D12_INPUT_ELEMENT_DESC{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    D3D12_INPUT_ELEMENT_DESC{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
  };

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return {INPUT_LAYOUT.data(), INPUT_LAYOUT.size()};
  }
};
static_assert(sizeof(PositionTexCoordColor) == 36);

struct PositionNormalTexCoordColor {
  Vector3 position;
  Vector3 normal;
  Vector2 tex_coord;
  Vector4 color;

  static constexpr std::array INPUT_LAYOUT = {
    D3D12_INPUT_ELEMENT_DESC{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    D3D12_INPUT_ELEMENT_DESC{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    D3D12_INPUT_ELEMENT_DESC{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    D3D12_INPUT_ELEMENT_DESC{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
  };

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return {INPUT_LAYOUT.data(), INPUT_LAYOUT.size()};
  }
};
static_assert(sizeof(PositionNormalTexCoordColor) == 48);

struct Empty {
  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return {};
  }
};

struct SkyboxVertex {
  Vector3 position;

  static constexpr std::array INPUT_LAYOUT = {
    D3D12_INPUT_ELEMENT_DESC{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
  };

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return {INPUT_LAYOUT.data(), INPUT_LAYOUT.size()};
  }
};
static_assert(sizeof(SkyboxVertex) == 12);

struct ShadowVertex {
  Vector3 position;

  static constexpr std::array INPUT_LAYOUT = {
    D3D12_INPUT_ELEMENT_DESC{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
  };

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return {INPUT_LAYOUT.data(), INPUT_LAYOUT.size()};
  }
};
static_assert(sizeof(ShadowVertex) == 12);

struct ModelVertex {
  Vector3 position;
  Vector3 normal;
  Vector2 texcoord;
  Vector4 color;
  Vector4 tangent;

  static constexpr std::array INPUT_LAYOUT = {
    D3D12_INPUT_ELEMENT_DESC{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    D3D12_INPUT_ELEMENT_DESC{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    D3D12_INPUT_ELEMENT_DESC{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    D3D12_INPUT_ELEMENT_DESC{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    D3D12_INPUT_ELEMENT_DESC{"TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
  };

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return {INPUT_LAYOUT.data(), INPUT_LAYOUT.size()};
  }
};
static_assert(sizeof(ModelVertex) == 64);

struct DepthNormalVertex {
  Vector3 position;
  Vector3 normal;

  static constexpr std::array INPUT_LAYOUT = {
    D3D12_INPUT_ELEMENT_DESC{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    D3D12_INPUT_ELEMENT_DESC{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
  };

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return {INPUT_LAYOUT.data(), INPUT_LAYOUT.size()};
  }
};
static_assert(sizeof(DepthNormalVertex) == 24);

using LineVertex = PositionColor;
using SpriteVertex = PositionTexCoordColor;
using Basic3DVertex = PositionNormalTexCoordColor;

// IA slot 1 per-instance element for objectIndex (uint32_t, step rate 1)
constexpr D3D12_INPUT_ELEMENT_DESC OBJECT_INDEX_ELEMENT = {
  "OBJECT_INDEX", 0, DXGI_FORMAT_R32_UINT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1};

inline std::vector<D3D12_INPUT_ELEMENT_DESC> WithObjectIndex(std::span<const D3D12_INPUT_ELEMENT_DESC> base_layout) {
  std::vector<D3D12_INPUT_ELEMENT_DESC> result(base_layout.begin(), base_layout.end());
  result.push_back(OBJECT_INDEX_ELEMENT);
  return result;
}

}  // namespace Graphics::Vertex
