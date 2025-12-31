#pragma once
#include <d3d12.h>

#include <string>
#include <vector>

// Shader configuration preset
// Defines which shaders to use and their input layout
struct ShaderConfig {
  std::wstring vs_path;  // Vertex shader path
  std::wstring ps_path;  // Pixel shader path
  std::vector<D3D12_INPUT_ELEMENT_DESC> input_layout;

  ShaderConfig() = default;

  ShaderConfig(const std::wstring& vs, const std::wstring& ps, const std::vector<D3D12_INPUT_ELEMENT_DESC>& layout)
      : vs_path(vs), ps_path(ps), input_layout(layout) {
  }
};

// Predefined shader configurations
namespace ShaderPresets {

// Basic 3D rendering (position + UV)
inline ShaderConfig CreateBasic3D() {
  std::vector<D3D12_INPUT_ELEMENT_DESC> layout = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};

  return ShaderConfig(L"Content/shaders/basic.vs.cso", L"Content/shaders/basic.ps.cso", layout);
}

// Sprite rendering (position + UV)
inline ShaderConfig CreateSprite() {
  std::vector<D3D12_INPUT_ELEMENT_DESC> layout = {
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};

  return ShaderConfig(L"Content/shaders/sprite.vs.cso", L"Content/shaders/sprite.ps.cso", layout);
}

// Common input layout for sprite instanced rendering
// Slot 0: position + UV (per-vertex), Slot 1: per-instance data (96 bytes)
inline std::vector<D3D12_INPUT_ELEMENT_DESC> GetSpriteInstancedLayout() {
  return {
    // Slot 0: Mesh Data (Per-Vertex)
    {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
    {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},

    // Slot 1: Instance Data (Per-Instance)
    // World Matrix: 4x float4 rows (64 bytes total, occupies 4 semantic indices)
    {"INSTANCE_WORLD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
    {"INSTANCE_WORLD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
    {"INSTANCE_WORLD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
    {"INSTANCE_WORLD", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
    
    // Color (16 bytes)
    {"INSTANCE_COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 64, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
    
    // UV Offset and Scale (16 bytes)
    {"INSTANCE_UV_OFFSET", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 80, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
    {"INSTANCE_UV_SCALE", 0, DXGI_FORMAT_R32G32_FLOAT, 1, 88, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
  };
}

// Sprite instanced rendering for UI pass (orthographic projection, Y-down)
inline ShaderConfig CreateSpriteInstancedUI() {
  return ShaderConfig(
    L"Content/shaders/sprite_instanced_ui.vs.cso",
    L"Content/shaders/sprite_instanced_ui.ps.cso",
    GetSpriteInstancedLayout());
}

// Sprite instanced rendering for World pass (perspective projection, Y-up)
inline ShaderConfig CreateSpriteInstancedWorld() {
  return ShaderConfig(
    L"Content/shaders/sprite_instanced_world.vs.cso",
    L"Content/shaders/sprite_instanced_world.ps.cso",
    GetSpriteInstancedLayout());
}

// Skeletal mesh rendering (position + UV + normal + bone weights/indices)
// inline ShaderConfig CreateSkeletalMesh() {
//   std::vector<D3D12_INPUT_ELEMENT_DESC> layout = {
//     {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
//     {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
//     {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
//     {"BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
//     {"BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};

//   return ShaderConfig(L"Content/shaders/skeletal.vs.cso", L"Content/shaders/skeletal.ps.cso", layout);
// }

// PBR material rendering (position + UV + normal + tangent)
// inline ShaderConfig CreatePBR() {
//   std::vector<D3D12_INPUT_ELEMENT_DESC> layout = {
//     {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
//     {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
//     {"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
//     {"TANGENT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}};

//   return ShaderConfig(L"Content/shaders/pbr.vs.cso", L"Content/shaders/pbr.ps.cso", layout);
// }

}  // namespace ShaderPresets
