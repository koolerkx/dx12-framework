#pragma once
#include <d3d12.h>

#include <span>
#include <string_view>

#include "shader_types.h"

namespace ShaderRegistry {

using Graphics::RSPreset;
using Graphics::ShaderFamily;
using Graphics::ShaderID;

// Shader metadata
struct ShaderMetadata {
  ShaderID id;
  ShaderFamily family;
  std::string_view name;

  // Shader paths (.cso)
  std::wstring_view vs_path;
  std::wstring_view ps_path;

  // Input layout
  std::span<const D3D12_INPUT_ELEMENT_DESC> input_layout;
};

// Helper functions
const ShaderMetadata& GetMetadata(ShaderID id);
ShaderFamily GetFamily(ShaderID id);
std::string_view GetName(ShaderID id);
RSPreset GetRSPreset(ShaderFamily family);

// Input Layout definitions
namespace InputLayouts {
// Sprite vertex: POSITION + TEXCOORD + COLOR
extern const D3D12_INPUT_ELEMENT_DESC SPRITE[];
extern const size_t SPRITE_COUNT;

// Sprite instanced: Per-vertex + Per-instance data
extern const D3D12_INPUT_ELEMENT_DESC SPRITE_INSTANCED[];
extern const size_t SPRITE_INSTANCED_COUNT;

// Basic 3D: POSITION + TEXCOORD + COLOR
extern const D3D12_INPUT_ELEMENT_DESC BASIC3D[];
extern const size_t BASIC3D_COUNT;

// Debug line: POSITION + COLOR
extern const D3D12_INPUT_ELEMENT_DESC DEBUG_LINE[];
extern const size_t DEBUG_LINE_COUNT;
}  // namespace InputLayouts

}  // namespace ShaderRegistry
