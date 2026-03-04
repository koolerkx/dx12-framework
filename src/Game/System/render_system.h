#pragma once

#include <algorithm>
#include <vector>

#include "system.h"

struct FramePacket;
class IRenderable;

class RenderSystem : public ISystem {
 public:
  void Update(float /*dt*/) override {}
  void Render(FramePacket& packet);

  void Register(IRenderable* renderable);
  void Unregister(IRenderable* renderable);
  void Clear();

 private:
  std::vector<IRenderable*> renderables_;
};
