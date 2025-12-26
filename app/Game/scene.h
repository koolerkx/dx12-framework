#pragma once
#include <DirectXMath.h>

#include <vector>

#include "render_world.h"

using float2 = DirectX::XMFLOAT2;
using float4 = DirectX::XMFLOAT4;

using TextureHandle = struct Texture*;

struct SpriteObject {
  uint64_t entity_id;
  float2 pos;
  float2 size;
  TextureHandle tex;
  float4 color;
};

class Scene {
 public:
  void AddSprite(const SpriteObject& sprite);
  const std::vector<SpriteObject>& GetSprites() const {
    return sprites_;
  }
  RenderWorld BuildRenderWorld() const;

 private:
  std::vector<SpriteObject> sprites_;
  uint64_t next_entity_id_ = 1;
};
