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

struct OpaqueDrawCommand {
  const Material* material = nullptr;
  const Mesh* mesh = nullptr;
  DirectX::XMFLOAT4X4 world_matrix;
  DirectX::XMFLOAT4 color;
  MaterialInstance material_instance;
  float depth = 0.0f;
};

struct TransparentDrawCommand {
  const Material* material = nullptr;
  const Mesh* mesh = nullptr;
  DirectX::XMFLOAT4X4 world_matrix;
  DirectX::XMFLOAT4 color;
  MaterialInstance material_instance;
  float depth = 0.0f;
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
