#pragma once
#include <DirectXMath.h>

#include <vector>

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
  DirectX::XMFLOAT4X4 world_matrix;
  DirectX::XMFLOAT4 color;
  Texture* texture;
  Mesh* mesh;
  float depth;  // For sorting
};

// Data specifically for UI rendering
struct UiDrawCommand {
  DirectX::XMFLOAT4X4 world_matrix;
  DirectX::XMFLOAT2 size;
  DirectX::XMFLOAT4 color;
  Texture* texture = nullptr;
  int layer_id = 0;
};

// The packet sent from Scene to Renderer
struct FramePacket {
  CameraData main_camera;

  std::vector<OpaqueDrawCommand> opaque_pass;
  std::vector<UiDrawCommand> ui_pass;
  // TODO: std::vector<MeshDrawCommand> transparent_pass;

  void Clear() {
    opaque_pass.clear();
    ui_pass.clear();
  }
};
