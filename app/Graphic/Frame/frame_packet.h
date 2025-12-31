#pragma once
#include <DirectXMath.h>

#include <vector>

#include "Pipeline/material.h"
#include "camera_data.h"

struct Texture;
class Mesh;

enum class RenderPassTag {
  Ui,                // Used for HUD, Menus (Overlay)
  WorldOpaque,       // Used for solid sprites in the world
  WorldTransparent,  // Used for alpha-blended sprites with depth
  Debug              // Used for engine tools/gizmos
};

struct SpriteInstanceData {
  DirectX::XMFLOAT4X4 world_matrix;  // 64 bytes - per-glyph/sprite world transform
  DirectX::XMFLOAT4 color;           // 16 bytes - per-instance tint color
  DirectX::XMFLOAT2 uv_offset;       //  8 bytes - UV offset for atlas lookup
  DirectX::XMFLOAT2 uv_scale;        //  8 bytes - UV scale for atlas lookup
};

struct OpaqueDrawCommand {
  const Material* material = nullptr;
  const Mesh* mesh = nullptr;
  DirectX::XMFLOAT4X4 world_matrix;
  DirectX::XMFLOAT4 color;
  MaterialInstance material_instance;
  float depth = 0.0f;
  DirectX::XMFLOAT2 uv_offset = {0.0f, 0.0f};  // UV offset for atlas
  DirectX::XMFLOAT2 uv_scale = {1.0f, 1.0f};   // UV scale (1,1 = full texture)

  // Instanced rendering support (if non-empty, use DrawIndexedInstanced)
  std::vector<SpriteInstanceData> instances;
};

struct TransparentDrawCommand {
  const Material* material = nullptr;
  const Mesh* mesh = nullptr;
  DirectX::XMFLOAT4X4 world_matrix;
  DirectX::XMFLOAT4 color;
  MaterialInstance material_instance;
  float depth = 0.0f;
  DirectX::XMFLOAT2 uv_offset = {0.0f, 0.0f};  // UV offset for atlas
  DirectX::XMFLOAT2 uv_scale = {1.0f, 1.0f};   // UV scale (1,1 = full texture)

  // Instanced rendering support (if non-empty, use DrawIndexedInstanced)
  std::vector<SpriteInstanceData> instances;
};

// Data specifically for UI rendering
struct UiDrawCommand {
  const Material* material = nullptr;
  const Mesh* mesh = nullptr;
  DirectX::XMFLOAT4X4 world_matrix;
  DirectX::XMFLOAT2 size;
  DirectX::XMFLOAT4 color;
  MaterialInstance material_instance;
  float depth = 0.0f;
  int layer_id = 0;
  DirectX::XMFLOAT2 uv_offset = {0.0f, 0.0f};  // UV offset for atlas/text glyphs
  DirectX::XMFLOAT2 uv_scale = {1.0f, 1.0f};   // UV scale (1,1 = full texture)

  // When populated, the above single-object fields (world_matrix, color, uv_offset, uv_scale) are ignored
  std::vector<SpriteInstanceData> instances;
};

// The packet sent from Scene to Renderer
struct FramePacket {
  CameraData main_camera;

  std::vector<OpaqueDrawCommand> opaque_pass;
  std::vector<TransparentDrawCommand> transparent_pass;
  std::vector<UiDrawCommand> ui_pass;

  void Clear() {
    opaque_pass.clear();
    transparent_pass.clear();
    ui_pass.clear();
  }
};
