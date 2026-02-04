/**
 * @file vertex_types.h
 * @brief Vertex struct definitions with embedded D3D12 input layouts.
 *
 * @note Each vertex struct contains its own input layout definition,
 *       ensuring Single Source of Truth for CPU struct and GPU layout.
 *
 * @warning When modifying vertex structs, always update the corresponding
 *          kInputLayout offsets and add static_assert for size verification.
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
#include <DirectXMath.h>
#include <d3d12.h>

#include <array>
#include <span>

namespace Graphics::Vertex {

// PositionColor - Used for debug lines and simple colored geometry
struct PositionColor {
  DirectX::XMFLOAT3 position;
  DirectX::XMFLOAT4 color;

  static constexpr std::array kInputLayout = {
    D3D12_INPUT_ELEMENT_DESC{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    D3D12_INPUT_ELEMENT_DESC{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
  };

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return {kInputLayout.data(), kInputLayout.size()};
  }
};
static_assert(sizeof(PositionColor) == 28);

// PositionTexCoordColor - Used for sprites and basic 3D geometry
struct PositionTexCoordColor {
  DirectX::XMFLOAT3 position;
  DirectX::XMFLOAT2 tex_coord;
  DirectX::XMFLOAT4 color;

  static constexpr std::array kInputLayout = {
    D3D12_INPUT_ELEMENT_DESC{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    D3D12_INPUT_ELEMENT_DESC{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    D3D12_INPUT_ELEMENT_DESC{"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
  };

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return {kInputLayout.data(), kInputLayout.size()};
  }
};
static_assert(sizeof(PositionTexCoordColor) == 36);

// SpriteInstance - Per-instance data for instanced sprite rendering
struct SpriteInstance {
  DirectX::XMFLOAT4X4 world_matrix;  // 64 bytes
  DirectX::XMFLOAT4 color;           // 16 bytes
  DirectX::XMFLOAT2 uv_offset;       //  8 bytes
  DirectX::XMFLOAT2 uv_scale;        //  8 bytes

  static constexpr std::array kInstanceLayout = {
    D3D12_INPUT_ELEMENT_DESC{"INSTANCE_WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
    D3D12_INPUT_ELEMENT_DESC{"INSTANCE_WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
    D3D12_INPUT_ELEMENT_DESC{"INSTANCE_WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
    D3D12_INPUT_ELEMENT_DESC{"INSTANCE_WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
    D3D12_INPUT_ELEMENT_DESC{"INSTANCE_COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
    D3D12_INPUT_ELEMENT_DESC{"INSTANCE_UV_OFFSET", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 80, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
    D3D12_INPUT_ELEMENT_DESC{"INSTANCE_UV_SCALE", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 88, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
  };

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInstanceLayout() {
    return {kInstanceLayout.data(), kInstanceLayout.size()};
  }
};
static_assert(sizeof(SpriteInstance) == 96);

// SpriteInstanced - Combined layout for instanced rendering (Slot 0: Vertex + Slot 1: Instance)
namespace detail {
inline constexpr auto MakeSpriteInstancedLayout() {
  std::array<D3D12_INPUT_ELEMENT_DESC, PositionTexCoordColor::kInputLayout.size() + SpriteInstance::kInstanceLayout.size()> result{};
  size_t idx = 0;
  for (const auto& elem : PositionTexCoordColor::kInputLayout) {
    result[idx++] = elem;
  }
  for (const auto& elem : SpriteInstance::kInstanceLayout) {
    result[idx++] = elem;
  }
  return result;
}
}  // namespace detail

struct SpriteInstanced {
  static constexpr auto kInputLayout = detail::MakeSpriteInstancedLayout();

  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return {kInputLayout.data(), kInputLayout.size()};
  }
};

// Empty - Used for full-screen triangle (no vertex input)
struct Empty {
  static std::span<const D3D12_INPUT_ELEMENT_DESC> GetInputLayout() {
    return {};
  }
};

// Type aliases for semantic clarity
using LineVertex = PositionColor;
using SpriteVertex = PositionTexCoordColor;
using Basic3DVertex = PositionTexCoordColor;

}  // namespace Graphics::Vertex
