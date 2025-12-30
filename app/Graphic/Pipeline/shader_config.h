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
