#include "shader_metadata.h"

namespace ShaderRegistry {

// ============================================================================
// Input Layout Definitions
// ============================================================================

namespace InputLayouts {

// Sprite vertex: POSITION + TEXCOORD + COLOR
const D3D12_INPUT_ELEMENT_DESC SPRITE[] = {
  {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
  {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
  {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
};
const size_t SPRITE_COUNT = std::size(SPRITE);

// Sprite instanced: Per-vertex + Per-instance data
const D3D12_INPUT_ELEMENT_DESC SPRITE_INSTANCED[] = {
  // Slot 0: Mesh Data (Per-Vertex)
  {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
  {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
  {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},

  // Slot 1: Instance Data (Per-Instance)
  // World Matrix: 4x float4 rows (64 bytes total)
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
const size_t SPRITE_INSTANCED_COUNT = std::size(SPRITE_INSTANCED);

// Basic 3D: POSITION + TEXCOORD + COLOR
const D3D12_INPUT_ELEMENT_DESC BASIC3D[] = {
  {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
  {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
  {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
};
const size_t BASIC3D_COUNT = std::size(BASIC3D);

// Debug line: POSITION + COLOR
const D3D12_INPUT_ELEMENT_DESC DEBUG_LINE[] = {
  {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
  {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
};
const size_t DEBUG_LINE_COUNT = std::size(DEBUG_LINE);

// Empty: Full-screen triangle (no input layout)
// Use nullptr for empty input layout with count 0
const D3D12_INPUT_ELEMENT_DESC* EMPTY = nullptr;
const size_t EMPTY_COUNT = 0;

}  // namespace InputLayouts

// ============================================================================
// Shader Registry Table
// ============================================================================

namespace {
// Centralized shader registry table
const ShaderMetadata SHADER_TABLE[] = {
  // === Sprite Shaders ===

  // Non-instanced sprite
  {ShaderID::Sprite,
    ShaderFamily::Sprite,
    "Sprite",
    L"Content/shaders/sprite.vs.cso",
    L"Content/shaders/sprite.ps.cso",
    {InputLayouts::SPRITE, InputLayouts::SPRITE_COUNT}},

  // Instanced UI sprite
  {ShaderID::SpriteInstancedUI,
    ShaderFamily::Sprite,
    "SpriteInstancedUI",
    L"Content/shaders/sprite_instanced_ui.vs.cso",
    L"Content/shaders/sprite_instanced_ui.ps.cso",
    {InputLayouts::SPRITE_INSTANCED, InputLayouts::SPRITE_INSTANCED_COUNT}},

  // Instanced World sprite (opaque)
  {ShaderID::SpriteInstancedWorld,
    ShaderFamily::Sprite,
    "SpriteInstancedWorld",
    L"Content/shaders/sprite_instanced_world.vs.cso",
    L"Content/shaders/sprite_instanced_world.ps.cso",
    {InputLayouts::SPRITE_INSTANCED, InputLayouts::SPRITE_INSTANCED_COUNT}},

  // Instanced World sprite (transparent)
  {ShaderID::SpriteInstancedWorldTransparent,
    ShaderFamily::Sprite,
    "SpriteInstancedWorldTransparent",
    L"Content/shaders/sprite_instanced_world_transparent.vs.cso",
    L"Content/shaders/sprite_instanced_world_transparent.ps.cso",
    {InputLayouts::SPRITE_INSTANCED, InputLayouts::SPRITE_INSTANCED_COUNT}},

  // === Basic 3D Shaders ===

  // Basic 3D rendering
  {ShaderID::Basic3D,
    ShaderFamily::Mesh,
    "Basic3D",
    L"Content/shaders/basic.vs.cso",
    L"Content/shaders/basic.ps.cso",
    {InputLayouts::BASIC3D, InputLayouts::BASIC3D_COUNT}},

  // Debug line rendering
  {ShaderID::DebugLine,
    ShaderFamily::Mesh,
    "DebugLine",
    L"Content/shaders/debug_line.vs.cso",
    L"Content/shaders/debug_line.ps.cso",
    {InputLayouts::DEBUG_LINE, InputLayouts::DEBUG_LINE_COUNT}},

  // === Post-Process Shaders ===

  // Tone mapping pass (HDR to SDR)
  {ShaderID::PostProcessToneMap,
    ShaderFamily::PostProcess,
    "ToneMap",
    L"Content/shaders/tonemap.vs.cso",
    L"Content/shaders/tonemap.ps.cso",
    {InputLayouts::EMPTY, InputLayouts::EMPTY_COUNT}},
};

static_assert(sizeof(SHADER_TABLE) / sizeof(ShaderMetadata) == 7,
  "Shader table must have exactly 7 implemented shader entries (Sprite, SpriteInstancedUI, "
  "SpriteInstancedWorld, SpriteInstancedWorldTransparent, Basic3D, DebugLine, PostProcessToneMap)");
}  // namespace

// ============================================================================
// Helper Functions
// ============================================================================

const ShaderMetadata& GetMetadata(ShaderID id) {
  // Switch-based lookup to handle sparse ShaderID enum values
  switch (id) {
    case ShaderID::Sprite:
      return SHADER_TABLE[0];
    case ShaderID::SpriteInstancedUI:
      return SHADER_TABLE[1];
    case ShaderID::SpriteInstancedWorld:
      return SHADER_TABLE[2];
    case ShaderID::SpriteInstancedWorldTransparent:
      return SHADER_TABLE[3];
    case ShaderID::Basic3D:
      return SHADER_TABLE[4];
    case ShaderID::DebugLine:
      return SHADER_TABLE[5];
    case ShaderID::PostProcessToneMap:
      return SHADER_TABLE[6];
    default:
      // Return first shader as fallback for unimplemented shaders
      return SHADER_TABLE[0];
  }
}

ShaderFamily GetFamily(ShaderID id) {
  return GetMetadata(id).family;
}

std::string_view GetName(ShaderID id) {
  return GetMetadata(id).name;
}

RSPreset GetRSPreset(ShaderFamily family) {
  switch (family) {
    case ShaderFamily::Sprite:
    case ShaderFamily::Mesh:
    case ShaderFamily::Text:
    case ShaderFamily::PostProcess:
      return RSPreset::Standard;

    case ShaderFamily::Deferred:
      return RSPreset::Deferred;

    case ShaderFamily::Compute:
      return RSPreset::Compute;

    default:
      return RSPreset::Standard;
  }
}

}  // namespace ShaderRegistry
