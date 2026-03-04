#pragma once

#include "system.h"

struct FramePacket;

class RenderSystem : public ISystem {
 public:
  void Update(float /*dt*/) override {}
  void Render(FramePacket& packet);
};
