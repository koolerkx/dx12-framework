#pragma once
#include <DirectXMath.h>

#include <vector>

using float2 = DirectX::XMFLOAT2;
using float4 = DirectX::XMFLOAT4;
using TextureHandle = struct Texture*;

struct UiSpriteItem {
  float2 pos;
  float2 size;
  TextureHandle tex;
  float4 color;
};

struct RenderWorld {
  std::vector<UiSpriteItem> ui_sprites;

  // std::vector<MeshInstance> opaque_meshes;
  // CameraData world_camera;
};
